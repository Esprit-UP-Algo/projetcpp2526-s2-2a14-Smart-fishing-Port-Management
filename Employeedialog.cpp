#include "employeedialog.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QLocale>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>

EmployeeDialog::EmployeeDialog(QWidget *parent, EmployeeModel* employeeData)
    : QDialog(parent), employeeData(employeeData), isEdit(employeeData != nullptr)
{
    setupUi();

    if (isEdit) {
        populateFields();
    }
}

EmployeeDialog::~EmployeeDialog()
{
}

void EmployeeDialog::setupUi()
{
    setWindowTitle("Ajouter / Modifier Employé");
    setFixedSize(700, 650);
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
    header->setFixedHeight(80);
    header->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            padding: 20px;
        }
    )");
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel* title = new QLabel(isEdit ? "Modifier Employé" : "Ajouter Employé");
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: white;");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

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

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    // Prénom et Nom (côte à côte)
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(20);

    // Prénom
    QVBoxLayout* prenomCol = new QVBoxLayout();
    prenomCol->setSpacing(8);
    QLabel* prenomLabel = new QLabel("Prénom");
    prenomLabel->setFont(labelFont);
    prenomLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    prenomCol->addWidget(prenomLabel);

    firstNameInput = new QLineEdit();
    firstNameInput->setPlaceholderText("Ahmed");
    firstNameInput->setFont(inputFont);
    firstNameInput->setFixedHeight(50);
    firstNameInput->setStyleSheet(getInputStyle());
    prenomCol->addWidget(firstNameInput);
    nameRow->addLayout(prenomCol);

    // Nom
    QVBoxLayout* nomCol = new QVBoxLayout();
    nomCol->setSpacing(8);
    QLabel* nomLabel = new QLabel("Nom");
    nomLabel->setFont(labelFont);
    nomLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    nomCol->addWidget(nomLabel);

    lastNameInput = new QLineEdit();
    lastNameInput->setPlaceholderText("Khalil");
    lastNameInput->setFont(inputFont);
    lastNameInput->setFixedHeight(50);
    lastNameInput->setStyleSheet(getInputStyle());
    nomCol->addWidget(lastNameInput);
    nameRow->addLayout(nomCol);

    formLayout->addLayout(nameRow);
    formLayout->addSpacing(10);

    // CIN
    QLabel* cinLabel = new QLabel("CIN");
    cinLabel->setFont(labelFont);
    cinLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(cinLabel);

    cinInput = new QLineEdit();
    cinInput->setPlaceholderText("01234567");
    cinInput->setValidator(new QIntValidator(0, 99999999, this));
    cinInput->setFont(inputFont);
    cinInput->setFixedHeight(50);
    cinInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(cinInput);

    formLayout->addSpacing(10);

    // Position
    QLabel* positionLabel = new QLabel("Position");
    positionLabel->setFont(labelFont);
    positionLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(positionLabel);

    positionInput = new QLineEdit();
    positionInput->setPlaceholderText("Marin");
    positionInput->setFont(inputFont);
    positionInput->setFixedHeight(50);
    positionInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(positionInput);

    formLayout->addSpacing(10);

    // Salaire
    QLabel* salaireLabel = new QLabel("Salaire");
    salaireLabel->setFont(labelFont);
    salaireLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(salaireLabel);

    salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("Ex: 1200.50");
    salaryInput->setValidator(new QDoubleValidator(0, 100000, 2, this));
    salaryInput->setFont(inputFont);
    salaryInput->setFixedHeight(50);
    salaryInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(salaryInput);

    formLayout->addSpacing(10);

    // Date de recrutement
    QLabel* dateLabel = new QLabel("Date de recrutement");
    dateLabel->setFont(labelFont);
    dateLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(dateLabel);

    dateInput = new QDateEdit();
    dateInput->setDate(QDate::currentDate());
    dateInput->setCalendarPopup(true);
    dateInput->setFont(inputFont);
    dateInput->setFixedHeight(50);
    dateInput->setStyleSheet(getInputStyle());
    dateInput->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(dateInput);

    formLayout->addSpacing(10);

    // Statut
    QLabel* statusLabel = new QLabel("Statut");
    statusLabel->setFont(labelFont);
    statusLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(statusLabel);

    statusCombo = new QComboBox();
    statusCombo->addItems({"Actif", "Congé", "Inactif"});
    statusCombo->setFont(inputFont);
    statusCombo->setFixedHeight(50);
    statusCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(statusCombo);

    formLayout->addStretch();

    // Buttons
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
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(saveBtn);

    formLayout->addLayout(buttonLayout);

    // Ajouter le contenu au scroll area
    scrollArea->setWidget(content);

    // Ajouter le scroll area au layout principal
    mainLayout->addWidget(scrollArea);
}

QString EmployeeDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F8F9FA;
            border: 2px solid #E1E8ED;
            border-radius: 10px;
            padding: 12px 15px;
            color: #2C3E50;
            font-size: 14px;
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

void EmployeeDialog::populateFields()
{
    if (!employeeData) return;

    cinInput->setText(employeeData->getCin());
    salaryInput->setText(QString::number(employeeData->getSalaire()));
    dateInput->setDate(employeeData->getDate());
    
    int index = statusCombo->findText(employeeData->getStatut());
    if (index >= 0) statusCombo->setCurrentIndex(index);
    
    positionInput->setText(employeeData->getPosition());
    firstNameInput->setText(employeeData->getPrenom());
    lastNameInput->setText(employeeData->getNom());
}

EmployeeModel EmployeeDialog::getData() const
{
    return EmployeeModel(
        "", // ID is handled by caller
        cinInput->text(),
        QLocale().toDouble(salaryInput->text()),
        dateInput->date(),
        statusCombo->currentText(),
        positionInput->text(),
        firstNameInput->text(),
        lastNameInput->text()
    );
}
