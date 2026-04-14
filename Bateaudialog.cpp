#include "bateaudialog.h"
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

BateauDialog::BateauDialog(QWidget *parent, Bateau* bateauData)
    : QDialog(parent), bateauData(bateauData), isEdit(bateauData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

BateauDialog::~BateauDialog() {}

void BateauDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier un Bateau" : "Ajouter un Bateau");
    setFixedSize(700, 600);
    setModal(true);

    setStyleSheet(R"(
        QDialog {
            background-color: #F0F4F8;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(100);
    header->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            padding: 20px;
        }
    )");
    QVBoxLayout* headerVLayout = new QVBoxLayout(header);
    headerVLayout->setContentsMargins(30, 10, 30, 10);
    headerVLayout->setSpacing(5);

    QLabel* title = new QLabel(isEdit ? "Modifier Bateau" : "Ajouter Bateau");
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: white;");
    headerVLayout->addWidget(title);

    QLabel* subTitle = new QLabel(isEdit ? "✏️  Modifier les informations du bateau" : "⛵  Informations du bateau");
    subTitle->setFont(QFont("Segoe UI", 11));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerVLayout->addWidget(subTitle);

    mainLayout->addWidget(header);

    // Scroll Area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: white; border: none; }"
        "QScrollBar:vertical { background: #F0F4F8; width: 10px; border-radius: 5px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #5D9CEC; border-radius: 5px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #3b82f6; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    // Form content
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(25);
    formLayout->setContentsMargins(40, 40, 40, 40);
    formLayout->addSpacing(5);

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    // --- Nom ---
    QLabel* nomLabel = new QLabel("Nom du Bateau");
    nomLabel->setFont(labelFont);
    nomLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(nomLabel);

    nomBateauInput = new QLineEdit();
    nomBateauInput->setPlaceholderText("ex: Le Marin");
    nomBateauInput->setFont(inputFont);
    nomBateauInput->setFixedHeight(50);
    nomBateauInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(nomBateauInput);

    QLabel* nomErrorLabel = new QLabel("⚠️ Attention: Le nom ne doit pas contenir de chiffres");
    nomErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    nomErrorLabel->hide();
    formLayout->addWidget(nomErrorLabel);

    connect(nomBateauInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            nomErrorLabel->hide(); 
            nomBateauInput->setStyleSheet(getInputStyle()); 
            return; 
        }
        QRegularExpression rx("^[a-zA-ZÀ-ÿ\\s-]*$");
        if(!rx.match(text).hasMatch()) {
            nomErrorLabel->show();
            nomBateauInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            nomErrorLabel->hide();
            nomBateauInput->setStyleSheet(getInputStyle());
        }
    });

    formLayout->addSpacing(10);

    // --- Immatriculation ---
    QLabel* immatLabel = new QLabel("Immatriculation");
    immatLabel->setFont(labelFont);
    immatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(immatLabel);

    immatriculationInput = new QLineEdit();
    immatriculationInput->setPlaceholderText("ex: TN-2025-001");
    immatriculationInput->setFont(inputFont);
    immatriculationInput->setFixedHeight(50);
    immatriculationInput->setStyleSheet(getInputStyle());

    // Bloquer les caractères non conformes au format TN-YYYY-NUMERO
    QRegularExpression immatParialRegex("^TN(-\\d{0,4}(-\\d{0,6})?)?$");
    QRegularExpressionValidator* immatValidator = new QRegularExpressionValidator(immatParialRegex, this);
    immatriculationInput->setValidator(immatValidator);

    formLayout->addWidget(immatriculationInput);

    QLabel* immatErrorLabel = new QLabel("⚠️ Format invalide ! Attendu : TN-YYYY-NUMERO (ex: TN-2025-001)");
    immatErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    immatErrorLabel->hide();
    formLayout->addWidget(immatErrorLabel);

    connect(immatriculationInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) {
            immatErrorLabel->hide();
            immatriculationInput->setStyleSheet(getInputStyle());
            return;
        }
        QRegularExpression fullImmat("^TN-\\d{4}-\\d+$");
        if(!fullImmat.match(text).hasMatch()) {
            immatErrorLabel->show();
            immatriculationInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            immatErrorLabel->hide();
            immatriculationInput->setStyleSheet(getInputStyle());
        }
    });

    formLayout->addSpacing(10);

    // --- Capacité ---
    QLabel* capLabel = new QLabel("Capacité (T)");
    capLabel->setFont(labelFont);
    capLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(capLabel);

    capaciteInput = new QLineEdit();
    capaciteInput->setPlaceholderText("ex: 50.5");
    capaciteInput->setFont(inputFont);
    capaciteInput->setFixedHeight(50);
    capaciteInput->setStyleSheet(getInputStyle());
    // Bloquer les lettres : nombres positifs uniquement
    QDoubleValidator* capValidator = new QDoubleValidator(0.01, 99999.99, 2, this);
    capValidator->setNotation(QDoubleValidator::StandardNotation);
    capaciteInput->setValidator(capValidator);
    formLayout->addWidget(capaciteInput);

    QLabel* capErrorLabel = new QLabel("⚠️ Veuillez saisir un nombre valide (> 0).");
    capErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    capErrorLabel->hide();
    formLayout->addWidget(capErrorLabel);

    connect(capaciteInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            capErrorLabel->hide(); 
            capaciteInput->setStyleSheet(getInputStyle()); 
            return; 
        }
        QRegularExpression rx("^[0-9]*[.,]?[0-9]*$");
        if(!rx.match(text).hasMatch()) {
            capErrorLabel->show();
            capaciteInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            capErrorLabel->hide();
            capaciteInput->setStyleSheet(getInputStyle());
        }
    });

    formLayout->addSpacing(10);

    // --- Longueur ---
    QLabel* lonLabel = new QLabel("Longueur (m)");
    lonLabel->setFont(labelFont);
    lonLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(lonLabel);

    longueurInput = new QLineEdit();
    longueurInput->setPlaceholderText("ex: 25.5");
    longueurInput->setFont(inputFont);
    longueurInput->setFixedHeight(50);
    longueurInput->setStyleSheet(getInputStyle());
    // Bloquer les lettres : nombres positifs uniquement
    QDoubleValidator* lonValidator = new QDoubleValidator(0.01, 9999.99, 2, this);
    lonValidator->setNotation(QDoubleValidator::StandardNotation);
    longueurInput->setValidator(lonValidator);
    formLayout->addWidget(longueurInput);

    QLabel* lonErrorLabel = new QLabel("⚠️ Veuillez saisir un nombre valide (> 0).");
    lonErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    lonErrorLabel->hide();
    formLayout->addWidget(lonErrorLabel);

    connect(longueurInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            lonErrorLabel->hide(); 
            longueurInput->setStyleSheet(getInputStyle()); 
            return; 
        }
        QRegularExpression rx("^[0-9]*[.,]?[0-9]*$");
        if(!rx.match(text).hasMatch()) {
            lonErrorLabel->show();
            longueurInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            lonErrorLabel->hide();
            longueurInput->setStyleSheet(getInputStyle());
        }
    });

    formLayout->addSpacing(10);

    // --- Âge ---
    QLabel* ageLabel = new QLabel("Âge (Ans)");
    ageLabel->setFont(labelFont);
    ageLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(ageLabel);

    ageInput = new QLineEdit();
    ageInput->setPlaceholderText("ex: 5");
    ageInput->setFont(inputFont);
    ageInput->setFixedHeight(50);
    ageInput->setStyleSheet(getInputStyle());
    // Bloquer les lettres : entiers uniquement (0-150)
    QIntValidator* ageValidator = new QIntValidator(0, 150, this);
    ageInput->setValidator(ageValidator);
    formLayout->addWidget(ageInput);

    QLabel* ageErrorLabel = new QLabel("⚠️ Veuillez saisir un nombre entier (pas de lettres).");
    ageErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    ageErrorLabel->hide();
    formLayout->addWidget(ageErrorLabel);

    connect(ageInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            ageErrorLabel->hide(); 
            ageInput->setStyleSheet(getInputStyle()); 
            return; 
        }
        QRegularExpression rx("^[0-9]*$");
        if(!rx.match(text).hasMatch()) {
            ageErrorLabel->show();
            ageInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            ageErrorLabel->hide();
            ageInput->setStyleSheet(getInputStyle());
        }
    });

    formLayout->addSpacing(10);

    // --- Dernière Maintenance ---
    QLabel* dateLabel = new QLabel("Dernière Maintenance");
    dateLabel->setFont(labelFont);
    dateLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(dateLabel);

    dateMaintenanceInput = new QDateEdit();
    dateMaintenanceInput->setCalendarPopup(true);
    dateMaintenanceInput->setDisplayFormat("dd/MM/yyyy");
    dateMaintenanceInput->setFont(inputFont);
    dateMaintenanceInput->setFixedHeight(50);
    dateMaintenanceInput->setStyleSheet(getInputStyle());
    dateMaintenanceInput->setDate(QDate::currentDate());
    formLayout->addWidget(dateMaintenanceInput);

    formLayout->addSpacing(10);

    // --- Employé ---
    QLabel* empLabel = new QLabel("Employé");
    empLabel->setFont(labelFont);
    empLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(empLabel);

    employeeCombo = new QComboBox();
    employeeCombo->setFont(inputFont);
    employeeCombo->setFixedHeight(50);
    employeeCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(employeeCombo);

    formLayout->addSpacing(10);

    // --- Quai ---
    QLabel* quaiLabel = new QLabel("Quai");
    quaiLabel->setFont(labelFont);
    quaiLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(quaiLabel);

    quaiCombo = new QComboBox();
    quaiCombo->setFont(inputFont);
    quaiCombo->setFixedHeight(50);
    quaiCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(quaiCombo);

    formLayout->addSpacing(10);

    // --- État du Bateau ---
    QLabel* etatLabel = new QLabel("État du Bateau");
    etatLabel->setFont(labelFont);
    etatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(etatLabel);

    etatCombo = new QComboBox();
    etatCombo->addItem("Au port");
    etatCombo->addItem("En mer");
    etatCombo->addItem("En maintenance");
    etatCombo->setFont(inputFont);
    etatCombo->setFixedHeight(50);
    etatCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(etatCombo);

    formLayout->addStretch();

    // --- Buttons ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    buttonLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Annuler");
    QFont btnFont("Segoe UI", 13, QFont::Medium);
    cancelBtn->setFont(btnFont);
    cancelBtn->setFixedSize(140, 50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E8EEF5;
            color: #5A6C7D;
            border: none;
            border-radius: 10px;
            padding: 12px 30px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #D8DEE5;
        }
        QPushButton:pressed {
            background-color: #C8CED5;
        }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton("Enregistrer");
    QFont saveBtnFont("Segoe UI", 13, QFont::Bold);
    saveBtn->setFont(saveBtnFont);
    saveBtn->setFixedSize(160, 50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 35px;
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #4A89DC;
        }
        QPushButton:pressed {
            background-color: #3B77C4;
        }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &BateauDialog::onSave);
    buttonLayout->addWidget(saveBtn);

    formLayout->addLayout(buttonLayout);

    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);
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
    // 1. Nom
    QString nom = nomBateauInput->text().trimmed();
    if(nom.isEmpty() || nom.length() < 2){ 
        showError("Le nom du bateau est obligatoire et doit contenir au moins 2 caractères (sans chiffres)."); 
        return false; 
    }

    // 2. Immatriculation - format TN-YYYY-NUMERO obligatoire
    QString immat = immatriculationInput->text().trimmed();
    QRegularExpression fullImmat("^TN-\\d{4}-\\d+$");
    if (immat.isEmpty()) {
        showError("L'immatriculation est obligatoire.");
        return false;
    } else if (!fullImmat.match(immat).hasMatch()) {
        showError("Format d'immatriculation invalide !\nAttendu : TN-YYYY-NUMERO\nExemple : TN-2025-001");
        return false;
    }

    // [NOUVEAU] Contrôle unicité via le modèle
    int currentIdB = (isEdit && bateauData) ? bateauData->getIdBateau().toInt() : -1;
    if (Bateau::immatriculationExiste(immat, currentIdB)) {
        showError("Cette immatriculation est déjà utilisée par un autre bateau."); 
        return false; 
    }

    // 3. Capacité
    bool ok;
    QString capStr = capaciteInput->text().replace(",", ".");
    double cap = capStr.toDouble(&ok);
    if(!ok || cap <= 0){ 
        showError("La capacité doit être un nombre strictement positif."); 
        return false; 
    }

    // 4. Longueur
    QString lonStr = longueurInput->text().replace(",", ".");
    double lon = lonStr.toDouble(&ok);
    if(!ok || lon <= 0){ 
        showError("La longueur doit être un nombre strictement positif."); 
        return false; 
    }

    // 5. Age
    int age = ageInput->text().toInt(&ok);
    if(!ok || age < 0 || age > 150){ 
        showError("L'âge doit être un entier positif (0-150)."); 
        return false; 
    }

    // 6. Maintenance Date
    if(dateMaintenanceInput->date() > QDate::currentDate()) {
        showError("La date de dernière maintenance ne peut pas être dans le futur.");
        return false;
    }

    return true;
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
    // État
    QString etatStr = bateauData->getEtat();
    if (etatStr == "En mer") etatCombo->setCurrentIndex(1);
    else if (etatStr == "En maintenance") etatCombo->setCurrentIndex(2);
    else etatCombo->setCurrentIndex(0); // Au port (default)
}

Bateau BateauDialog::getData() const {
    Bateau b;
    b.setNomBateau(nomBateauInput->text().trimmed());
    b.setImmatriculation(immatriculationInput->text().trimmed());
    b.setCapacite(capaciteInput->text().trimmed());
    b.setLongueur(longueurInput->text().trimmed());
    b.setAgeBateau(ageInput->text().trimmed());
    b.setDateMaintenance(dateMaintenanceInput->date().toString("dd/MM/yyyy"));
    b.setIdEmploye(employeeCombo->currentData().toString());
    b.setIdQuai(quaiCombo->currentData().toString());
    b.setEtat(etatCombo->currentText());
    return b;
}

QString BateauDialog::getInputStyle() const {
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F8F9FA;
            border: 2px solid #E1E8ED;
            border-radius: 10px;
            padding: 12px 15px;
            color: #2C3E50;
            font-size: 12px;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #5D9CEC;
            background-color: white;
        }
        QLineEdit:hover, QComboBox:hover, QDateEdit:hover {
            border: 2px solid #B8D4F1;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #5A6C7D;
            margin-right: 12px;
        }
        QDateEdit::drop-down {
            border: none;
            width: 30px;
        }
        QDateEdit::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #5A6C7D;
            margin-right: 12px;
        }
    )";
}

QString BateauDialog::getRadioStyle() const {
    return "QRadioButton { color: #2C3E50; spacing: 10px; font-size: 13px; font-family: 'Segoe UI'; }";
}
