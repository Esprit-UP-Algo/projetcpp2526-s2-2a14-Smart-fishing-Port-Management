#include "addfrigodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>
#include <QDateEdit>
#include <QDate>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QAbstractItemView>
#include <QLineEdit>

// ==================== HELPERS POUR DIALOGUE STYLÉ ====================
class ComboClickFilter : public QObject {
public:
    ComboClickFilter(QObject* parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            QLineEdit* le = qobject_cast<QLineEdit*>(watched);
            if (le && le->parentWidget()) {
                QComboBox* cb = qobject_cast<QComboBox*>(le->parentWidget());
                if (cb && event->type() == QEvent::MouseButtonPress) {
                    cb->showPopup();
                    return true;
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

class DialogMoveFilter : public QObject {
public:
    DialogMoveFilter(QDialog* dialog, QObject* parent = nullptr) : QObject(parent), m_dialog(dialog), m_dragging(false) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (!m_dialog) return QObject::eventFilter(watched, event);
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) {
                    m_dragging = true;
                    m_dragOffset = me->globalPosition().toPoint() - m_dialog->frameGeometry().topLeft();
                    return true;
                }
                break;
            }
            case QEvent::MouseMove: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                    m_dialog->move(me->globalPosition().toPoint() - m_dragOffset);
                    return true;
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) { m_dragging = false; return true; }
                break;
            }
            default: break;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    QDialog* m_dialog;
    bool m_dragging;
    QPoint m_dragOffset;
};

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle) {
    if (!dialog || !dragHandle) return;
    dragHandle->setCursor(Qt::OpenHandCursor);
    dragHandle->installEventFilter(new DialogMoveFilter(dialog, dragHandle));
}

AddFrigoDialog::AddFrigoDialog(QWidget *parent, FrigoModel* frigoData)
    : QDialog(parent), frigoData(frigoData), isEdit(frigoData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

AddFrigoDialog::~AddFrigoDialog() {}

void AddFrigoDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Frigo" : "Nouveau Frigo");
    setFixedSize(650, 750);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 100));

    QWidget* container = new QWidget(this);
    container->setGeometry(15, 15, 620, 720);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 28px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header avec Dégradé Premium (Cyan/Indigo pour Frigo)
    QFrame* header = new QFrame();
    header->setFixedHeight(130);
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563EB, stop:1 #5D9CEC);
            border-top-left-radius: 28px;
            border-top-right-radius: 28px;
        }
    )");
    
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(35, 0, 25, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    titleCol->setAlignment(Qt::AlignVCenter);

    QLabel* title = new QLabel(isEdit ? "Modifier le Frigo" : "Ajouter un Frigo");
    title->setFont(QFont("Segoe UI", 22, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    titleCol->addWidget(title);

    QLabel* sub = new QLabel(isEdit ? "✏️  Mise à jour des paramètres de conservation" : "🧊  Installation d'une nouvelle unité de stockage à froid");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color: rgba(255, 255, 255, 0.85); background: transparent;");
    titleCol->addWidget(sub);
    headerLay->addLayout(titleCol, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(38, 38);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.15); color: white; border: none;
                      border-radius: 19px; font-size: 15px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.3); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLay->addWidget(closeBtn);

    mainLayout->addWidget(header);
    makeDialogMovable(this, header);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    QWidget* formContent = new QWidget();
    formContent->setStyleSheet("background: transparent;");
    QVBoxLayout* formLayout = new QVBoxLayout(formContent);
    formLayout->setSpacing(18);
    formLayout->setContentsMargins(45, 30, 45, 30);

    auto addLabel = [&](const QString& text) {
        QLabel* lbl = new QLabel(text);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
        lbl->setStyleSheet("color: #4B5563; margin-bottom: 2px; letter-spacing: 0.5px;");
        formLayout->addWidget(lbl);
        return lbl;
    };

    addLabel("RÉFÉRENCE UNITÉ (FRG-XXXX)");
    refEdit = new QLineEdit();
    refEdit->setPlaceholderText("FRG-001");
    refEdit->setStyleSheet(getInputStyle());
    QRegularExpression frgRegex("^FRG(-\\d{0,6})?$");
    refEdit->setValidator(new QRegularExpressionValidator(frgRegex, this));
    formLayout->addWidget(refEdit);
    refError = new QLabel("");
    refError->setFont(QFont("Segoe UI", 8, QFont::Bold));
    refError->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    refError->hide();
    formLayout->addWidget(refError);
    connect(refEdit, &QLineEdit::textChanged, this, [=](const QString& t){
        QRegularExpression frgFull("^FRG-\\d+$");
        bool ok = frgFull.match(t.trimmed()).hasMatch();
        updateFieldStyle(refEdit, ok);
        if(!ok) { refError->setText("Format invalide — ex: FRG-001"); refError->show(); }
        else refError->hide();
    });

    addLabel("CAPACITÉ MAX (Kg)");
    capEdit = new QLineEdit();
    capEdit->setPlaceholderText("0.00");
    capEdit->setStyleSheet(getInputStyle());
    QDoubleValidator* capVal = new QDoubleValidator(0.01, 999999.99, 2, this);
    capVal->setNotation(QDoubleValidator::StandardNotation);
    capEdit->setValidator(capVal);
    formLayout->addWidget(capEdit);
    capError = new QLabel("");
    capError->setFont(QFont("Segoe UI", 8, QFont::Bold));
    capError->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    capError->hide();
    formLayout->addWidget(capError);
    connect(capEdit, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; double val = t.trimmed().replace(",",".").toDouble(&ok);
        bool valid = ok && val > 0;
        updateFieldStyle(capEdit, valid);
        if(!valid) { capError->setText("Nombre positif requis"); capError->show(); }
        else capError->hide();
    });

    addLabel("TEMPÉRATURE (°C)");
    tempEdit = new QLineEdit();
    tempEdit->setPlaceholderText("-20.0");
    tempEdit->setStyleSheet(getInputStyle());
    QDoubleValidator* tVal = new QDoubleValidator(-100.0, 100.0, 2, this);
    tVal->setNotation(QDoubleValidator::StandardNotation);
    tempEdit->setValidator(tVal);
    formLayout->addWidget(tempEdit);
    tempError = new QLabel("");
    tempError->setFont(QFont("Segoe UI", 8, QFont::Bold));
    tempError->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    tempError->hide();
    formLayout->addWidget(tempError);
    connect(tempEdit, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; t.trimmed().replace(",",".").toDouble(&ok);
        bool valid = ok && !t.trimmed().isEmpty();
        updateFieldStyle(tempEdit, valid);
        if(!valid) { tempError->setText("Nombre requis (ex: -20.0)"); tempError->show(); }
        else tempError->hide();
    });

    addLabel("STATUT");
    statusBox = new QComboBox();
    statusBox->addItems({"Disponible", "Occupé", "Maintenance"});
    statusBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(statusBox);

    addLabel("TYPE DE POISSON");
    fishBox = new QComboBox();
    fishBox->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon", "Sans"});
    fishBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(fishBox);

    addLabel("DATE DE RÉSERVATION / MISE EN SERVICE");
    dateResEdit = new QDateEdit();
    dateResEdit->setCalendarPopup(true);
    dateResEdit->setDate(QDate::currentDate());
    dateResEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(dateResEdit);

    addLabel("OCCUPATION ACTUELLE (Kg)");
    occEdit = new QLineEdit();
    occEdit->setPlaceholderText("0.00");
    occEdit->setStyleSheet(getInputStyle());
    QDoubleValidator* oVal = new QDoubleValidator(0.0, 999999.99, 2, this);
    oVal->setNotation(QDoubleValidator::StandardNotation);
    occEdit->setValidator(oVal);
    formLayout->addWidget(occEdit);
    occError = new QLabel("");
    occError->setFont(QFont("Segoe UI", 8, QFont::Bold));
    occError->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    occError->hide();
    formLayout->addWidget(occError);
    connect(occEdit, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; double val = t.trimmed().replace(",",".").toDouble(&ok);
        bool capOk; double cap = capEdit->text().replace(",",".").toDouble(&capOk);
        bool valid = ok && val >= 0 && (!capOk || val <= cap);
        updateFieldStyle(occEdit, valid);
        if(!valid) { occError->setText(cap > 0 ? QString("0 à %1 Kg requis").arg(cap) : "Nombre ≥ 0 requis"); occError->show(); }
        else occError->hide();
    });

    addLabel("TÉLÉPHONE RESPONSABLE");
    phoneBox = new QComboBox();
    phoneBox->setEditable(true);
    phoneBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(phoneBox);

    scrollArea->setWidget(formContent);
    mainLayout->addWidget(scrollArea, 1);

    // Footer avec boutons stylés
    QFrame* footer = new QFrame();
    footer->setFixedHeight(90);
    QHBoxLayout* btnLay = new QHBoxLayout(footer);
    btnLay->setContentsMargins(45, 0, 45, 15);
    btnLay->setSpacing(20);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #4B5563; border-radius: 15px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLay->addWidget(cancelBtn, 1);

    QPushButton* saveBtn = new QPushButton(isEdit ? "💾  Mettre à jour" : "➕  Activer Frigo");
    saveBtn->setFixedHeight(50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    saveBtn->setStyleSheet(R"(
        QPushButton { 
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563EB, stop:1 #5D9CEC);
            color: white; border: none; border-radius: 15px; padding: 0 20px;
        }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &AddFrigoDialog::onSave);
    btnLay->addWidget(saveBtn, 2);

    mainLayout->addWidget(footer);

    // Populer les téléphones
    QSqlQuery qEmp("SELECT DISTINCT TELEPHONE FROM EMPLOYEES WHERE TELEPHONE IS NOT NULL");
    while (qEmp.next()) phoneBox->addItem(qEmp.value(0).toString());

    // Fix black popup background on combo dropdowns
    ComboClickFilter* comboFilter = new ComboClickFilter(this);
    for (QComboBox* combo : findChildren<QComboBox*>()) {
        if (!combo->isEditable()) {
            combo->setEditable(true);
            combo->lineEdit()->setReadOnly(true);
            combo->lineEdit()->setCursor(Qt::PointingHandCursor);
            combo->lineEdit()->installEventFilter(comboFilter);
        }
        if (combo->view() && combo->view()->window()) {
            combo->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
            combo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
        }
    }
}

void AddFrigoDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

QString AddFrigoDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 10px 15px;
            color: #1F2937;
            font-size: 11pt;
            outline: none;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #2563EB;
            background-color: white;
            outline: none;
        }
        *[state="error"] { border: 2px solid #EF4444; background-color: #FEF2F2; }
        *[state="success"] { border: 2px solid #10B981; }
        QComboBox::drop-down { border: none; width: 30px; outline: none; }
        QComboBox::down-arrow { image: none; border-left: 6px solid transparent; border-right: 6px solid transparent; border-top: 6px solid #6B7280; margin-right: 15px; }
        QComboBox QAbstractItemView {
            background: white;
            color: #1e293b;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            selection-background-color: #EBF5FF;
            selection-color: #1e293b;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 32px;
            padding: 4px 10px;
            border-radius: 4px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #F1F5F9;
        }
    )";
}

void AddFrigoDialog::populateFields()
{
    if (!frigoData) return;
    refEdit->setText(frigoData->getRef());
    capEdit->setText(QString::number(frigoData->getCap()));
    tempEdit->setText(QString::number(frigoData->getTemp()));
    occEdit->setText(QString::number(frigoData->getOcc()));
    statusBox->setCurrentText(frigoData->getStat());
    fishBox->setCurrentText(frigoData->getType());
    phoneBox->setCurrentText(frigoData->getTelephone());
    QDate dt = QDate::fromString(frigoData->getDateRes(), "dd/MM/yyyy");
    if (dt.isValid()) dateResEdit->setDate(dt);
}

void AddFrigoDialog::onSave() { if (validateInputs()) accept(); }

bool AddFrigoDialog::validateInputs()
{
    bool isValid = true;
    
    // Validation Référence
    QString ref = refEdit->text().trimmed();
    QRegularExpression frgFull("^FRG-\\d+$");
    if (ref.isEmpty()) {
        refError->setText("⚠️ La référence est obligatoire.");
        refError->show();
        updateFieldStyle(refEdit, false);
        isValid = false;
    } else if (!frgFull.match(ref).hasMatch()) {
        refError->setText("⚠️ Format invalide ! (ex: FRG-001)");
        refError->show();
        updateFieldStyle(refEdit, false);
        isValid = false;
    } else {
        refError->hide();
        updateFieldStyle(refEdit, true);
    }

    // Validation Capacité
    bool ok;
    double capVal = capEdit->text().replace(",", ".").toDouble(&ok);
    if (capEdit->text().isEmpty()) {
        capError->setText("⚠️ Capacité obligatoire.");
        capError->show();
        updateFieldStyle(capEdit, false);
        isValid = false;
    } else if (!ok || capVal <= 0) {
        capError->setText("⚠️ Nombre positif requis.");
        capError->show();
        updateFieldStyle(capEdit, false);
        isValid = false;
    } else {
        capError->hide();
        updateFieldStyle(capEdit, true);
    }

    // Validation Température
    tempEdit->text().replace(",", ".").toDouble(&ok);
    if (tempEdit->text().isEmpty()) {
        tempError->setText("⚠️ Température obligatoire.");
        tempError->show();
        updateFieldStyle(tempEdit, false);
        isValid = false;
    } else if (!ok) {
        tempError->setText("⚠️ Nombre requis.");
        tempError->show();
        updateFieldStyle(tempEdit, false);
        isValid = false;
    } else {
        tempError->hide();
        updateFieldStyle(tempEdit, true);
    }

    // Validation Occupation
    double occVal = occEdit->text().trimmed().isEmpty() ? 0.0 : occEdit->text().replace(",", ".").toDouble(&ok);
    if (!ok || occVal < 0 || occVal > capVal) {
        occError->setText(QString("⚠️ 0 à %1 Kg requis.").arg(capVal));
        occError->show();
        updateFieldStyle(occEdit, false);
        isValid = false;
    } else {
        occError->hide();
        updateFieldStyle(occEdit, true);
    }

    return isValid;
}

FrigoModel AddFrigoDialog::getData() const
{
    return FrigoModel("", refEdit->text(), capEdit->text().toDouble(),
                      fishBox->currentText(), statusBox->currentText(),
                      dateResEdit->date().toString("dd/MM/yyyy"),
                      tempEdit->text().toDouble(), occEdit->text().toDouble(),
                      phoneBox->currentText());
}

void AddFrigoDialog::setData(QString, QString, QString, QString, QString, QString) {}
QString AddFrigoDialog::getId() { return ""; }
QString AddFrigoDialog::getCap() { return capEdit->text(); }
QString AddFrigoDialog::getHum() { return "0"; }
QString AddFrigoDialog::getTemp() { return tempEdit->text(); }
QString AddFrigoDialog::getStatus() { return statusBox->currentText(); }
QString AddFrigoDialog::getFish() { return fishBox->currentText(); }
