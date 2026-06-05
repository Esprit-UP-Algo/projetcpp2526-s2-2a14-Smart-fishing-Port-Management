#include "bateaudialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
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

BateauDialog::BateauDialog(QWidget *parent, Bateau* bateauData)
    : QDialog(parent), bateauData(bateauData), isEdit(bateauData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

BateauDialog::~BateauDialog() {}

void BateauDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Bateau" : "Ajouter Bateau");
    setFixedSize(650, 720);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 100));

    QWidget* container = new QWidget(this);
    container->setGeometry(15, 15, 620, 690);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 28px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header avec Dégradé Premium (Bleu Portuaire)
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

    QLabel* title = new QLabel(isEdit ? "Modifier le Bateau" : "Nouveau Bateau");
    title->setFont(QFont("Segoe UI", 22, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    titleCol->addWidget(title);

    QLabel* sub = new QLabel(isEdit ? "✏️  Mise à jour des spécifications techniques" : "🚢  Enregistrement d'un nouveau navire au registre");
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

    // Zone de formulaire scrollable
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

    auto addErrorLabel = [&]() {
        QLabel* lbl = new QLabel("");
        lbl->setFont(QFont("Segoe UI", 8, QFont::Bold));
        lbl->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
        lbl->hide();
        formLayout->addWidget(lbl);
        return lbl;
    };

    addLabel("NOM DU BATEAU");
    nomBateauInput = new QLineEdit();
    nomBateauInput->setPlaceholderText("Nom du navire");
    nomBateauInput->setStyleSheet(getInputStyle());
    nomBateauInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-ZÀ-ÿ\\s-]*$"), this));
    formLayout->addWidget(nomBateauInput);
    nomErrorLabel = addErrorLabel();
    connect(nomBateauInput, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok = t.trimmed().length() >= 2;
        updateFieldStyle(nomBateauInput, ok);
        if(!ok) { nomErrorLabel->setText("Mimumum 2 lettres requis"); nomErrorLabel->show(); }
        else nomErrorLabel->hide();
    });

    QLabel* l1 = new QLabel("IMMATRICULATION");
    l1->setFont(QFont("Segoe UI", 9, QFont::Bold));
    l1->setStyleSheet("color: #4B5563; margin-bottom: 2px;");
    formLayout->addWidget(l1);
    immatriculationInput = new QLineEdit();
    immatriculationInput->setPlaceholderText("Ex: TN-2025-001");
    immatriculationInput->setStyleSheet(getInputStyle());
    QRegularExpression immatRegex("^TN(-\\d{0,4}(-\\d{0,6})?)?$");
    immatriculationInput->setValidator(new QRegularExpressionValidator(immatRegex, this));
    formLayout->addWidget(immatriculationInput);
    immatErrorLabel = new QLabel("");
    immatErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    immatErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    immatErrorLabel->hide();
    formLayout->addWidget(immatErrorLabel);
    
    immatUnicityLabel = new QLabel("");
    immatUnicityLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    immatUnicityLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    immatUnicityLabel->hide();
    formLayout->addWidget(immatUnicityLabel);
    connect(immatriculationInput, &QLineEdit::textChanged, this, [=](const QString& t){
        QRegularExpression fullImmat("^TN-\\d{4}-\\d+$");
        bool ok = fullImmat.match(t.trimmed()).hasMatch();
        updateFieldStyle(immatriculationInput, ok);
        if(!ok) { immatErrorLabel->setText("Format TN-YYYY-ID requis"); immatErrorLabel->show(); }
        else immatErrorLabel->hide();
    });

    QLabel* l2 = new QLabel("CODE SECRET (4 CHIFFRES)");
    l2->setFont(QFont("Segoe UI", 9, QFont::Bold));
    l2->setStyleSheet("color: #4B5563; margin-bottom: 2px;");
    formLayout->addWidget(l2);
    codeSecInput = new QLineEdit();
    codeSecInput->setPlaceholderText("Ex: 1234");
    codeSecInput->setMaxLength(4);
    codeSecInput->setEchoMode(QLineEdit::Normal);
    codeSecInput->setStyleSheet(getInputStyle());
    codeSecInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,4}$"), this));
    formLayout->addWidget(codeSecInput);
    codeErrorLabel = new QLabel("");
    codeErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    codeErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    codeErrorLabel->hide();
    formLayout->addWidget(codeErrorLabel);
    connect(codeSecInput, &QLineEdit::textChanged, this, [=](const QString& t){
        QRegularExpression codeExact("^\\d{4}$");
        bool ok = codeExact.match(t.trimmed()).hasMatch();
        updateFieldStyle(codeSecInput, ok);
        if(!ok) { codeErrorLabel->setText("Exactement 4 chiffres"); codeErrorLabel->show(); }
        else codeErrorLabel->hide();
    });

    QLabel* l3 = new QLabel("CAPACITÉ (T)");
    l3->setFont(QFont("Segoe UI", 9, QFont::Bold));
    l3->setStyleSheet("color: #4B5563; margin-bottom: 2px;");
    formLayout->addWidget(l3);
    capaciteInput = new QLineEdit();
    capaciteInput->setPlaceholderText("Tonnes (T)");
    capaciteInput->setStyleSheet(getInputStyle());
    QDoubleValidator* capVal = new QDoubleValidator(0.01, 999999.99, 2, this);
    capVal->setNotation(QDoubleValidator::StandardNotation);
    capaciteInput->setValidator(capVal);
    formLayout->addWidget(capaciteInput);
    capErrorLabel = new QLabel("");
    capErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    capErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    capErrorLabel->hide();
    formLayout->addWidget(capErrorLabel);
    connect(capaciteInput, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; double val = t.trimmed().replace(",", ".").toDouble(&ok);
        bool valid = ok && val > 0;
        updateFieldStyle(capaciteInput, valid);
        if(!valid) { capErrorLabel->setText("Nombre positif requis"); capErrorLabel->show(); }
        else capErrorLabel->hide();
    });

    QLabel* l4 = new QLabel("LONGUEUR (m)");
    l4->setFont(QFont("Segoe UI", 9, QFont::Bold));
    l4->setStyleSheet("color: #4B5563; margin-bottom: 2px;");
    formLayout->addWidget(l4);
    longueurInput = new QLineEdit();
    longueurInput->setPlaceholderText("Ex: 12.5");
    longueurInput->setStyleSheet(getInputStyle());
    QDoubleValidator* longVal = new QDoubleValidator(0.01, 999.99, 2, this);
    longVal->setNotation(QDoubleValidator::StandardNotation);
    longueurInput->setValidator(longVal);
    formLayout->addWidget(longueurInput);
    
    longErrorLabel = new QLabel("");
    longErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    longErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    longErrorLabel->hide();
    formLayout->addWidget(longErrorLabel);

    connect(longueurInput, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; double val = t.trimmed().replace(",", ".").toDouble(&ok);
        bool valid = ok && val > 0;
        updateFieldStyle(longueurInput, valid);
        if(!valid) { longErrorLabel->setText("Nombre positif requis"); longErrorLabel->show(); }
        else longErrorLabel->hide();
    });

    QLabel* l5 = new QLabel("ÂGE DU BATEAU (ans)");
    l5->setFont(QFont("Segoe UI", 9, QFont::Bold));
    l5->setStyleSheet("color: #4B5563; margin-bottom: 2px;");
    formLayout->addWidget(l5);
    ageInput = new QLineEdit();
    ageInput->setPlaceholderText("Ex: 5");
    ageInput->setStyleSheet(getInputStyle());
    ageInput->setValidator(new QIntValidator(1, 1000, this));
    formLayout->addWidget(ageInput);
    
    ageErrorLabel = new QLabel("");
    ageErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    ageErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    ageErrorLabel->hide();
    formLayout->addWidget(ageErrorLabel);

    connect(ageInput, &QLineEdit::textChanged, this, [=](const QString& t){
        bool ok; int val = t.trimmed().toInt(&ok);
        bool valid = ok && val > 0;
        updateFieldStyle(ageInput, valid);
        if(!valid && !t.isEmpty()) { ageErrorLabel->setText("Un entier positif est requis"); ageErrorLabel->show(); }
        else ageErrorLabel->hide();
    });

    addLabel("PROPRIÉTAIRE / EMPLOYÉ");
    employeeCombo = new QComboBox();
    employeeCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(employeeCombo);

    addLabel("ÉTAT ACTUEL");
    etatCombo = new QComboBox();
    etatCombo->addItems({"Au port", "En mer", "En maintenance"});
    etatCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(etatCombo);

    addLabel("QUAI AFFECTÉ");
    quaiCombo = new QComboBox();
    quaiCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(quaiCombo);
    quaiErrorLabel = new QLabel("");
    quaiErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    quaiErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    quaiErrorLabel->hide();
    formLayout->addWidget(quaiErrorLabel);

    addLabel("DATE DE DERNIÈRE MAINTENANCE");
    dateMaintenanceInput = new QDateEdit();
    dateMaintenanceInput->setCalendarPopup(true);
    dateMaintenanceInput->setDate(QDate::currentDate());
    dateMaintenanceInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(dateMaintenanceInput);
    dateErrorLabel = new QLabel("");
    dateErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    dateErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    dateErrorLabel->hide();
    formLayout->addWidget(dateErrorLabel);


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

    QPushButton* saveBtn = new QPushButton(isEdit ? "💾  Mettre à jour" : "➕  Enregistrer le Navire");
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
    connect(saveBtn, &QPushButton::clicked, this, &BateauDialog::onSave);
    btnLay->addWidget(saveBtn, 2);

    mainLayout->addWidget(footer);

    // Logique d'activation du quai
    auto updateQuaiStatus = [=]() {
        if (etatCombo->currentIndex() == 0) { // "Au port"
            quaiCombo->setEnabled(true);
        } else {
            quaiCombo->setCurrentIndex(0); // "Aucun quai"
            quaiCombo->setEnabled(false);
        }
    };
    connect(etatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int){ updateQuaiStatus(); });

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

void BateauDialog::onSave() { if(validateInputs()) accept(); }

void BateauDialog::setEmployeeList(const QList<QPair<QString, QString>>& employees) {
    employeeCombo->clear();
    for (const auto& pair : employees) employeeCombo->addItem(pair.second, pair.first);
}

void BateauDialog::setQuaiList(const QList<QPair<QString, QString>>& quais) {
    quaiCombo->clear();
    quaiCombo->addItem("Aucun quai", "");
    for (const auto& pair : quais) quaiCombo->addItem(pair.second, pair.first);
    quaiCombo->setCurrentIndex(0);
}

bool BateauDialog::validateInputs() {
    // Force validation of all fields
    emit nomBateauInput->textChanged(nomBateauInput->text());
    emit immatriculationInput->textChanged(immatriculationInput->text());
    emit codeSecInput->textChanged(codeSecInput->text());
    emit capaciteInput->textChanged(capaciteInput->text());
    emit longueurInput->textChanged(longueurInput->text());

    bool allValid = true;

    // 1. Nom
    QString nom = nomBateauInput->text().trimmed();
    if(nom.isEmpty() || nom.length() < 2){ 
        updateFieldStyle(nomBateauInput, false);
        nomErrorLabel->setText("Le nom est obligatoire (min 2 lettres)");
        nomErrorLabel->show();
        allValid = false;
    } else { nomErrorLabel->hide(); }

    // 2. Immatriculation - format TN-YYYY-NUMERO obligatoire
    QString immat = immatriculationInput->text().trimmed();
    QRegularExpression fullImmat("^TN-\\d{4}-\\d+$");
    if (immat.isEmpty() || !fullImmat.match(immat).hasMatch()) {
        updateFieldStyle(immatriculationInput, false);
        immatErrorLabel->setText("Format TN-YYYY-ID requis");
        immatErrorLabel->show();
        immatUnicityLabel->hide();
        allValid = false;
    } else {
        immatErrorLabel->hide();
        // Contrôle unicité
        int currentIdB = (isEdit && bateauData) ? bateauData->getIdBateau().toInt() : -1;
        if (Bateau::immatriculationExiste(immat, currentIdB)) {
            updateFieldStyle(immatriculationInput, false);
            immatUnicityLabel->setText("⚠ Immatriculation déjà utilisée");
            immatUnicityLabel->show();
            allValid = false;
        } else {
            immatUnicityLabel->hide();
            updateFieldStyle(immatriculationInput, true);
        }
    }

    // 3. Capacité
    bool ok;
    QString capStr = capaciteInput->text().replace(",", ".");
    double cap = capStr.toDouble(&ok);
    if(!ok || cap <= 0){ 
        updateFieldStyle(capaciteInput, false);
        capErrorLabel->setText("Nombre positif requis");
        capErrorLabel->show();
        allValid = false;
    } else { capErrorLabel->hide(); }

    // 4. Longueur
    QString lonStr = longueurInput->text().replace(",", ".");
    double lon = lonStr.toDouble(&ok);
    if(!ok || lon <= 0){ 
        updateFieldStyle(longueurInput, false);
        longErrorLabel->setText("Nombre positif requis");
        longErrorLabel->show();
        allValid = false;
    } else { longErrorLabel->hide(); }

    // 4.1. Âge
    QString ageStr = ageInput->text().trimmed();
    int ageVal = ageStr.toInt(&ok);
    if (!ageStr.isEmpty() && (!ok || ageVal <= 0)) {
        updateFieldStyle(ageInput, false);
        ageErrorLabel->setText("Un entier positif est requis");
        ageErrorLabel->show();
        allValid = false;
    } else { ageErrorLabel->hide(); }

    // 5. Code Secret
    QString codeStr = codeSecInput->text().trimmed();
    QRegularExpression codeExact("^\\d{4}$");
    if (!codeExact.match(codeStr).hasMatch()) {
        updateFieldStyle(codeSecInput, false);
        codeErrorLabel->setText("Exactement 4 chiffres requis");
        codeErrorLabel->show();
        allValid = false;
    } else { codeErrorLabel->hide(); }

    // 6. Date de maintenance
    if(dateMaintenanceInput->date() > QDate::currentDate()) {
        updateFieldStyle(dateMaintenanceInput, false);
        dateErrorLabel->setText("La date ne peut pas être dans le futur");
        dateErrorLabel->show();
        allValid = false;
    } else { dateErrorLabel->hide(); }

    // 7. Quai
    if (etatCombo->currentText() == "Au port" && quaiCombo->currentData().toString().isEmpty()) {
        updateFieldStyle(quaiCombo, false);
        quaiErrorLabel->setText("Sélectionnez un quai pour un bateau au port");
        quaiErrorLabel->show();
        allValid = false;
    } else { quaiErrorLabel->hide(); }

    return allValid;
}

void BateauDialog::showError(const QString& msg) { QMessageBox::warning(this, "Validation", msg); }

void BateauDialog::populateFields() {
    if(!bateauData) return;
    nomBateauInput->setText(bateauData->getNomBateau());
    immatriculationInput->setText(bateauData->getImmatriculation());
    capaciteInput->setText(bateauData->getCapacite());
    longueurInput->setText(bateauData->getLongueur());
    ageInput->setText(bateauData->getAgeBateau());
    dateMaintenanceInput->setDate(QDate::fromString(bateauData->getDateMaintenance(), "dd/MM/yyyy"));
    int eIdx = employeeCombo->findData(bateauData->getIdEmploye());
    if(eIdx != -1) employeeCombo->setCurrentIndex(eIdx);
    int qIdx = quaiCombo->findData(bateauData->getIdQuai());
    if(qIdx != -1) quaiCombo->setCurrentIndex(qIdx);
    
    QString etatStr = bateauData->getEtat();
    if (etatStr == "En mer") etatCombo->setCurrentIndex(1);
    else if (etatStr == "En maintenance") etatCombo->setCurrentIndex(2);
    else etatCombo->setCurrentIndex(0);
    
    if (etatCombo->currentIndex() != 0) quaiCombo->setEnabled(false);
    else quaiCombo->setEnabled(true);
    
    codeSecInput->setText(QString::number(bateauData->getCodeSecret()));
}

Bateau BateauDialog::getData() const {
    Bateau b;
    if(isEdit && bateauData) b.setIdBateau(bateauData->getIdBateau());
    b.setNomBateau(nomBateauInput->text().trimmed());
    b.setImmatriculation(immatriculationInput->text().trimmed());
    b.setCapacite(capaciteInput->text().trimmed());
    b.setLongueur(longueurInput->text().trimmed());
    b.setAgeBateau(ageInput->text().trimmed());
    b.setDateMaintenance(dateMaintenanceInput->date().toString("dd/MM/yyyy"));
    b.setIdEmploye(employeeCombo->currentData().toString());
    b.setIdQuai(quaiCombo->currentData().toString());
    b.setEtat(etatCombo->currentText());
    b.setCodeSecret(codeSecInput->text().toInt());
    return b;
}

void BateauDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

QString BateauDialog::getInputStyle() const {
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

QString BateauDialog::getRadioStyle() const {
    return "QRadioButton { color: #2C3E50; spacing: 10px; font-size: 13px; font-family: 'Segoe UI'; }";
}
