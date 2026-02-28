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
#include <QMessageBox>

BateauDialog::BateauDialog(QWidget *parent, Bateau* bateauData)
    : QDialog(parent), bateauData(bateauData), isEdit(bateauData != nullptr)
{
    setupUi();

    if (isEdit) {
        populateFields();
    }
}

BateauDialog::~BateauDialog()
{
}

void BateauDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier un Bateau" : "Ajouter un Bateau");
    setFixedSize(700, 750);
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

    // Scroll Area pour le formulaire
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");

    // Form content
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(25);
    formLayout->setContentsMargins(40, 40, 40, 40);

    formLayout->addSpacing(5);

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    // Nom du bateau et Immatriculation
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(20);

    // Nom du bateau
    QVBoxLayout* nomCol = new QVBoxLayout();
    nomCol->setSpacing(8);
    QLabel* nomLabel = new QLabel("Nom du bateau");
    nomLabel->setFont(labelFont);
    nomLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    nomCol->addWidget(nomLabel);
    nomBateauInput = new QLineEdit();
    nomBateauInput->setPlaceholderText("Neptune");
    nomBateauInput->setFont(inputFont);
    nomBateauInput->setFixedHeight(50);
    nomBateauInput->setStyleSheet(getInputStyle());
    nomCol->addWidget(nomBateauInput);
    nameRow->addLayout(nomCol);

    // Immatriculation
    QVBoxLayout* immatCol = new QVBoxLayout();
    immatCol->setSpacing(8);
    QLabel* immatLabel = new QLabel("Immatriculation");
    immatLabel->setFont(labelFont);
    immatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    immatCol->addWidget(immatLabel);
    immatriculationInput = new QLineEdit();
    immatriculationInput->setPlaceholderText("TN-001");
    immatriculationInput->setFont(inputFont);
    immatriculationInput->setFixedHeight(50);
    immatriculationInput->setStyleSheet(getInputStyle());
    immatCol->addWidget(immatriculationInput);
    nameRow->addLayout(immatCol);
    formLayout->addLayout(nameRow);

    // Capacité et Longueur
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(20);
    QVBoxLayout* col1 = new QVBoxLayout();
    col1->setSpacing(8);
    QLabel* capaciteLabel = new QLabel("Capacité (tonnes)");
    capaciteLabel->setFont(labelFont);
    col1->addWidget(capaciteLabel);
    capaciteInput = new QLineEdit();
    capaciteInput->setPlaceholderText("50");
    capaciteInput->setFont(inputFont);
    capaciteInput->setFixedHeight(50);
    capaciteInput->setStyleSheet(getInputStyle());
    col1->addWidget(capaciteInput);
    row1->addLayout(col1);

    QVBoxLayout* col2 = new QVBoxLayout();
    col2->setSpacing(8);
    QLabel* longueurLabel = new QLabel("Longueur (m)");
    longueurLabel->setFont(labelFont);
    col2->addWidget(longueurLabel);
    longueurInput = new QLineEdit();
    longueurInput->setPlaceholderText("20");
    longueurInput->setFont(inputFont);
    longueurInput->setFixedHeight(50);
    longueurInput->setStyleSheet(getInputStyle());
    col2->addWidget(longueurInput);
    row1->addLayout(col2);
    formLayout->addLayout(row1);

    // Age et Date Maintenance
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(20);
    QVBoxLayout* col3 = new QVBoxLayout();
    col3->setSpacing(8);
    QLabel* ageLabel = new QLabel("Âge Bateau (Ans)");
    ageLabel->setFont(labelFont);
    col3->addWidget(ageLabel);
    ageInput = new QLineEdit();
    ageInput->setPlaceholderText("5");
    ageInput->setFont(inputFont);
    ageInput->setFixedHeight(50);
    ageInput->setStyleSheet(getInputStyle());
    col3->addWidget(ageInput);
    row2->addLayout(col3);

    QVBoxLayout* col4 = new QVBoxLayout();
    col4->setSpacing(8);
    QLabel* dateLabel = new QLabel("Dernière Maintenance");
    dateLabel->setFont(labelFont);
    col4->addWidget(dateLabel);
    dateMaintenanceInput = new QDateEdit();
    dateMaintenanceInput->setCalendarPopup(true);
    dateMaintenanceInput->setDate(QDate::currentDate());
    dateMaintenanceInput->setFont(inputFont);
    dateMaintenanceInput->setFixedHeight(50);
    dateMaintenanceInput->setStyleSheet(getInputStyle());
    dateMaintenanceInput->setDisplayFormat("dd/MM/yyyy");
    col4->addWidget(dateMaintenanceInput);
    row2->addLayout(col4);
    formLayout->addLayout(row2);

    // Employé et Quai
    QHBoxLayout* row3 = new QHBoxLayout();
    row3->setSpacing(20);
    QVBoxLayout* col5 = new QVBoxLayout();
    col5->setSpacing(8);
    QLabel* empLabel = new QLabel("Employé Responsable");
    empLabel->setFont(labelFont);
    col5->addWidget(empLabel);
    employeeCombo = new QComboBox();
    employeeCombo->setFont(inputFont);
    employeeCombo->setFixedHeight(50);
    employeeCombo->setStyleSheet(getInputStyle());
    col5->addWidget(employeeCombo);
    row3->addLayout(col5);

    QVBoxLayout* col6 = new QVBoxLayout();
    col6->setSpacing(8);
    QLabel* quaiLabel = new QLabel("Quai Affecté");
    quaiLabel->setFont(labelFont);
    col6->addWidget(quaiLabel);
    quaiCombo = new QComboBox();
    quaiCombo->setFont(inputFont);
    quaiCombo->setFixedHeight(50);
    quaiCombo->setStyleSheet(getInputStyle());
    col6->addWidget(quaiCombo);
    row3->addLayout(col6);
    formLayout->addLayout(row3);

    // Initialisation des ComboBoxes SQL
    QSqlQuery qEmp("SELECT ID_EMPLOYE, NOM_EMPLOYE FROM EMPLOYEE");
    while(qEmp.next()) employeeCombo->addItem(qEmp.value(1).toString(), qEmp.value(0));

    QSqlQuery qQuai("SELECT IDQUAI, NOMQUAI FROM QUAIS");
    while(qQuai.next()) quaiCombo->addItem(qQuai.value(1).toString(), qQuai.value(0));

    // Disponibilité
    QLabel* dispLabel = new QLabel("Disponibilité");
    dispLabel->setFont(labelFont);
    formLayout->addWidget(dispLabel);
    QHBoxLayout* dispLay = new QHBoxLayout();
    disponibleGroup = new QButtonGroup(this);
    disponibleOuiRadio = new QRadioButton("Oui");
    disponibleNonRadio = new QRadioButton("Non");
    disponibleOuiRadio->setStyleSheet(getRadioStyle());
    disponibleNonRadio->setStyleSheet(getRadioStyle());
    disponibleOuiRadio->setChecked(true);
    disponibleGroup->addButton(disponibleOuiRadio);
    disponibleGroup->addButton(disponibleNonRadio);
    dispLay->addWidget(disponibleOuiRadio);
    dispLay->addWidget(disponibleNonRadio);
    dispLay->addStretch();
    formLayout->addLayout(dispLay);

    formLayout->addStretch();

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    QPushButton* cancelBtn = new QPushButton("Annuler");
    QPushButton* saveBtn = new QPushButton("Enregistrer");
    cancelBtn->setFixedSize(140, 50);
    saveBtn->setFixedSize(160, 50);
    cancelBtn->setStyleSheet("background-color:#E8EEF5; color:#5A6C7D; border-radius:10px; font-weight:600;");
    saveBtn->setStyleSheet("background-color:#5D9CEC; color:white; border-radius:10px; font-weight:700;");
    
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &BateauDialog::onSave);
    
    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(saveBtn);
    formLayout->addLayout(btnLay);

    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);
}

void BateauDialog::onSave() {
    if(validateInputs()) accept();
}

bool BateauDialog::validateInputs() {
    // Contrôle de saisie obligatoire (Consigne cours)
    if(nomBateauInput->text().trimmed().isEmpty()){ showError("Le nom est obligatoire."); return false; }
    if(immatriculationInput->text().trimmed().isEmpty()){ showError("L'immatriculation est obligatoire."); return false; }
    
    bool ok;
    double cap = capaciteInput->text().toDouble(&ok);
    if(!ok || cap <= 0){ showError("Capacité invalide."); return false; }
    
    int age = ageInput->text().toInt(&ok);
    if(!ok || age < 0){ showError("Âge invalide."); return false; }

    if(employeeCombo->currentIndex() == -1){ showError("Sélectionnez un employé."); return false; }
    if(quaiCombo->currentIndex() == -1){ showError("Sélectionnez un quai."); return false; }

    return true;
}

void BateauDialog::showError(const QString& msg) {
    QMessageBox::warning(this, "Validation", msg);
}

void BateauDialog::populateFields() {
    if(!bateauData) return;
    nomBateauInput->setText(bateauData->getNomBateau());
    immatriculationInput->setText(bateauData->getImmatriculation());
    capaciteInput->setText(bateauData->getCapacite());
    longueurInput->setText(bateauData->getLongueur());
    ageInput->setText(bateauData->getAgeBateau());
    
    dateMaintenanceInput->setDate(QDate::fromString(bateauData->getDateMaintenance(), "dd/MM/yyyy"));
    if(bateauData->getDisponible() == "Oui") disponibleOuiRadio->setChecked(true);
    else disponibleNonRadio->setChecked(true);
    
    int eIdx = employeeCombo->findData(bateauData->getIdEmploye());
    if(eIdx != -1) employeeCombo->setCurrentIndex(eIdx);
    
    int qIdx = quaiCombo->findData(bateauData->getIdQuai());
    if(qIdx != -1) quaiCombo->setCurrentIndex(qIdx);
}

Bateau BateauDialog::getData() const {
    Bateau b;
    b.setNomBateau(nomBateauInput->text().trimmed());
    b.setImmatriculation(immatriculationInput->text().trimmed());
    b.setCapacite(capaciteInput->text().trimmed());
    b.setLongueur(longueurInput->text().trimmed());
    b.setAgeBateau(ageInput->text().trimmed());
    b.setDateMaintenance(dateMaintenanceInput->date().toString("dd/MM/yyyy"));
    b.setDisponible(disponibleOuiRadio->isChecked() ? "Oui" : "Non");
    b.setIdEmploye(employeeCombo->currentData().toString());
    b.setIdQuai(quaiCombo->currentData().toString());
    return b;
}

QString BateauDialog::getInputStyle() const {
    return "background-color:#F8F9FA; border:2px solid #E1E8ED; border-radius:10px; padding:12px; font-size:12px;";
}

QString BateauDialog::getRadioStyle() const {
    return "QRadioButton { color:#2C3E50; spacing:10px; padding:8px; }";
}

