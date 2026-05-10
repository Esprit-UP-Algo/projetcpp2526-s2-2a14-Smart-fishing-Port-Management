#include "AddLivraisonDialog.h"
#include <QApplication>
#include <cstdlib>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDateEdit>
#include <QStyle>
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QTimer>
#include <cstdlib>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

// ==================== HELPERS POUR DIALOGUE STYLÉ ====================
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

AddLivraisonDialog::AddLivraisonDialog(QWidget *parent, Livraison* livraisonData)
    : QDialog(parent), livraisonData(livraisonData), isEdit(livraisonData != nullptr)
{
    setupUi();
    populateLivreurCombo();
    if (isEdit) {
        populateFields();
    }
    this->update();
}

AddLivraisonDialog::~AddLivraisonDialog() {}

void AddLivraisonDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
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

    // Header avec Dégradé Premium (Violet/Indigo pour Livraison)
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

    QLabel* title = new QLabel(isEdit ? "Modifier Livraison" : "Nouvelle Livraison");
    title->setFont(QFont("Segoe UI", 22, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    titleCol->addWidget(title);

    QLabel* sub = new QLabel(isEdit ? "✏️  Mise à jour de l'itinéraire et du transport" : "📦  Planification d'une nouvelle expédition maritime");
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

    addLabel("RÉFÉRENCE LIVRAISON");
    referenceEdit = new QLineEdit();
    referenceEdit->setPlaceholderText("LIV-1234");
    referenceEdit->setStyleSheet(getInputStyle());
    QRegularExpression livRegex("^LIV(-\\d{0,6})?$");
    referenceEdit->setValidator(new QRegularExpressionValidator(livRegex, this));
    formLayout->addWidget(referenceEdit);

    errorReference = new QLabel("⚠ La référence doit commencer par LIV-");
    errorReference->setFont(QFont("Segoe UI", 8, QFont::Bold));
    errorReference->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    errorReference->hide();
    formLayout->addWidget(errorReference);
    connect(referenceEdit, &QLineEdit::textChanged, this, &AddLivraisonDialog::onReferenceChanged);

    addLabel("LIVREUR RESPONSABLE");
    livreurCombo = new QComboBox();
    livreurCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(livreurCombo);

    addLabel("ADRESSE DE DESTINATION");
    adresseEdit = new QTextEdit();
    adresseEdit->setPlaceholderText("Ex: 123 Rue de la Marine, Tunis");
    adresseEdit->setFixedHeight(80);
    adresseEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(adresseEdit);

    errorAdresse = new QLabel("⚠ L'adresse est obligatoire");
    errorAdresse->setFont(QFont("Segoe UI", 8, QFont::Bold));
    errorAdresse->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    errorAdresse->hide();
    formLayout->addWidget(errorAdresse);
    connect(adresseEdit, &QTextEdit::textChanged, this, [=](){
        onAdresseChanged();
    });

    addLabel("DATE PRÉVUE");
    dateEdit = new QDateEdit();
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(dateEdit);

    addLabel("VÉHICULE ASSIGNÉ");
    vehiculeEdit = new QLineEdit();
    vehiculeEdit->setPlaceholderText("Ex: Van-01 ou Camion-A");
    vehiculeEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(vehiculeEdit);

    connect(vehiculeEdit, &QLineEdit::textChanged, this, [=](const QString&){
        updateFieldStyle(vehiculeEdit, !vehiculeEdit->text().trimmed().isEmpty());
    });

    addLabel("TRANSPORT");
    transportEdit = new QComboBox();
    transportEdit->addItems({"Camion non frigorifique", "Camion frigorifique", "Motocyclette / Scooter", "Véhicule utilitaire léger", "Bateau"});
    transportEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(transportEdit);

    addLabel("PRIX (DT / $ / €)");
    prixEdit = new QLineEdit();
    prixEdit->setPlaceholderText("0.00 DT");
    prixEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(prixEdit);

    errorPrix = new QLabel("⚠ Doit finir par DT, $ ou €");
    errorPrix->setFont(QFont("Segoe UI", 8, QFont::Bold));
    errorPrix->setStyleSheet("color: #EF4444; margin-top: -10px; margin-bottom: 5px;");
    errorPrix->hide();
    formLayout->addWidget(errorPrix);
    
    QRegularExpression prixRegex("^\\d{0,8}(\\s?(DT|\\$|€))?$");
    prixEdit->setValidator(new QRegularExpressionValidator(prixRegex, this));
    connect(prixEdit, &QLineEdit::textChanged, this, &AddLivraisonDialog::onPrixChanged);

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

    QPushButton* saveBtn = new QPushButton(isEdit ? "💾  Mettre à jour" : "➕  Lancer l'expédition");
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
    connect(saveBtn, &QPushButton::clicked, this, &AddLivraisonDialog::handleSave);
    btnLay->addWidget(saveBtn, 2);

    mainLayout->addWidget(footer);
}

void AddLivraisonDialog::populateFields()
{
    if (!livraisonData) return;
    QDate d = QDate::fromString(livraisonData->getDate(), "dd/MM/yyyy");
    if (d.isValid()) dateEdit->setDate(d);
    adresseEdit->setPlainText(livraisonData->getAdresse());
    referenceEdit->setText(livraisonData->getReference());
    int lIdx = livreurCombo->findData(livraisonData->getIdEmploye());
    if (lIdx >= 0) livreurCombo->setCurrentIndex(lIdx);
    
    vehiculeEdit->setText(livraisonData->getVehicule());
    transportEdit->setCurrentText(livraisonData->getTransport());
    prixEdit->setText(livraisonData->getPrix());
}

void AddLivraisonDialog::populateLivreurCombo() {
    livreurCombo->clear();
    QSqlQuery query("SELECT ID_EMPLOYE, PRENOM, NOM FROM EMPLOYEES WHERE \"POSITION\" = 'Livreur' AND STATUT = 'Actif'");
    while (query.next()) {
        QString id = query.value(0).toString();
        livreurCombo->addItem(query.value(1).toString() + " " + query.value(2).toString() + " (" + id + ")", id);
    }
}

void AddLivraisonDialog::onReferenceChanged() {
    QString ref = referenceEdit->text().trimmed();
    QRegularExpression refRegex("^LIV-\\d{4}-\\d+$");
    bool valid = refRegex.match(ref).hasMatch();
    updateFieldStyle(referenceEdit, valid);
    if (!valid) { errorReference->setText("⚠ Format LIV-YYYY-ID requis"); errorReference->show(); }
    else errorReference->hide();
}

void AddLivraisonDialog::onAdresseChanged() {
    bool valid = !adresseEdit->toPlainText().trimmed().isEmpty();
    updateFieldStyle(adresseEdit, valid);
    if (!valid) { errorAdresse->setText("⚠ L'adresse est obligatoire"); errorAdresse->show(); }
    else errorAdresse->hide();
}

void AddLivraisonDialog::onPrixChanged() {
    QString p = prixEdit->text().trimmed();
    bool valid = !p.isEmpty() && (p.endsWith("DT") || p.endsWith("$") || p.endsWith("€"));
    updateFieldStyle(prixEdit, valid);
    if (!valid) { errorPrix->setText("⚠ Doit finir par DT, $ ou €"); errorPrix->show(); }
    else errorPrix->hide();
}

void AddLivraisonDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

bool AddLivraisonDialog::validateInputs() {
    bool ok = true;

    QString ref = referenceEdit->text().trimmed();
    QRegularExpression refRegex("^LIV-\\d{4}-\\d+$");
    if (!refRegex.match(ref).hasMatch()) { 
        updateFieldStyle(referenceEdit, false); 
        errorReference->show();
        ok = false; 
    }

    if (adresseEdit->toPlainText().trimmed().isEmpty()) { 
        updateFieldStyle(adresseEdit, false);
        errorAdresse->setText("⚠ L'adresse est obligatoire"); 
        errorAdresse->show();
        ok = false; 
    } else {
        errorAdresse->hide();
    }
    
    if (vehiculeEdit->text().trimmed().isEmpty()) { 
        updateFieldStyle(vehiculeEdit, false); 
        ok = false; 
    }

    QString p = prixEdit->text().trimmed();
    if (p.isEmpty() || !(p.endsWith("DT") || p.endsWith("$") || p.endsWith("€"))) { 
        updateFieldStyle(prixEdit, false); 
        errorPrix->show();
        ok = false; 
    }

    return ok;
}

void AddLivraisonDialog::handleSave() {
    if (validateInputs()) {
        accept();
    }
}


Livraison AddLivraisonDialog::getData() const {
    Livraison data;
    data.setDate(dateEdit->date().toString("dd/MM/yyyy"));
    data.setAdresse(adresseEdit->toPlainText());
    data.setVehicule(vehiculeEdit->text().trimmed());
    data.setTransport(transportEdit->currentText());
    data.setReference(referenceEdit->text().trimmed());
    data.setIdEmploye(livreurCombo->currentData().toString());
    data.setPrix(prixEdit->text().trimmed());
    if (isEdit && livraisonData) {
        data.setID(livraisonData->getID());
        data.setStatut(livraisonData->getStatut());
        data.setDuree(livraisonData->getDuree());
    } else {
        data.setStatut("En attente");
        data.setDuree(15 + (rand() % 45));
    }
    return data;
}

QString AddLivraisonDialog::getInputStyle() const {
    return R"(
        QLineEdit, QComboBox, QTextEdit, QDateEdit {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 10px 15px;
            color: #1F2937;
            font-size: 11pt;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTextEdit:focus {
            border: 2px solid #2563EB;
            background-color: white;
        }
        *[state="error"] { border: 2px solid #EF4444; background-color: #FEF2F2; }
        *[state="success"] { border: 2px solid #10B981; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: none; border-left: 6px solid transparent; border-right: 6px solid transparent; border-top: 6px solid #6B7280; margin-right: 15px; }
    )";
}
