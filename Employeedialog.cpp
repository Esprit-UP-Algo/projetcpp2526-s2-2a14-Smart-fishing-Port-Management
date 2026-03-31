#include "employeedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QMessageBox>
EmployeeDialog::EmployeeDialog(QWidget *parent, Employee* employeeData)
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

    firstNameErrorLabel = new QLabel("");
    firstNameErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    firstNameErrorLabel->hide();
    prenomCol->addWidget(firstNameErrorLabel);

    connect(firstNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateFirstName);

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
    
    lastNameErrorLabel = new QLabel("");
    lastNameErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    lastNameErrorLabel->hide();
    nomCol->addWidget(lastNameErrorLabel);

    connect(lastNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateLastName);

    nameRow->addLayout(nomCol);

    formLayout->addLayout(nameRow);
    formLayout->addSpacing(10);

    // CIN
    QLabel* cinLabel = new QLabel("CIN");
    cinLabel->setFont(labelFont);
    cinLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(cinLabel);

    cinInput = new QLineEdit();
    cinInput->setPlaceholderText("12345678");
    cinInput->setMaxLength(8);
    cinInput->setFont(inputFont);
    cinInput->setFixedHeight(50);
    cinInput->setStyleSheet(getInputStyle());
    // Bloquer les lettres : chiffres uniquement
    QIntValidator* cinValidator = new QIntValidator(0, 99999999, this);
    cinInput->setValidator(cinValidator);
    formLayout->addWidget(cinInput);

    cinErrorLabel = new QLabel("");
    cinErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    cinErrorLabel->hide();
    formLayout->addWidget(cinErrorLabel);

    connect(cinInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateCin);

    formLayout->addSpacing(10);

    // Position
    QLabel* positionLabel = new QLabel("Position");
    positionLabel->setFont(labelFont);
    positionLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(positionLabel);

    positionCombo = new QComboBox();
    positionCombo->addItems({"Marin", "RH", "Technicien", "Sécurité"});
    positionCombo->setFont(inputFont);
    positionCombo->setFixedHeight(50);
    positionCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(positionCombo);

    formLayout->addSpacing(10);

    // Salaire
    QLabel* salaireLabel = new QLabel("Salaire");
    salaireLabel->setFont(labelFont);
    salaireLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(salaireLabel);

    salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("1200");
    salaryInput->setFont(inputFont);
    salaryInput->setFixedHeight(50);
    salaryInput->setStyleSheet(getInputStyle());
    // Bloquer les lettres : entiers uniquement (1000-10000)
    QIntValidator* salaryValidator = new QIntValidator(0, 99999, this);
    salaryInput->setValidator(salaryValidator);
    formLayout->addWidget(salaryInput);
    
    salaryErrorLabel = new QLabel("");
    salaryErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    salaryErrorLabel->hide();
    formLayout->addWidget(salaryErrorLabel);

    connect(salaryInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateSalary);

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
    connect(saveBtn, &QPushButton::clicked, this, &EmployeeDialog::onSaveClicked);
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

void EmployeeDialog::populateFields()
{
    if (!employeeData) return;

    firstNameInput->setText(employeeData->firstName);
    lastNameInput->setText(employeeData->lastName);
    cinInput->setText(employeeData->cin);
    
    int posIndex = positionCombo->findText(employeeData->position);
    if (posIndex >= 0) {
        positionCombo->setCurrentIndex(posIndex);
    }
    
    QString cleanSal = employeeData->salary;
    cleanSal.remove(QRegularExpression("[^0-9]"));
    salaryInput->setText(cleanSal);

    // Parse date
    QStringList dateParts = employeeData->date.split("/");
    if (dateParts.size() == 3) {
        dateInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    }

    int index = statusCombo->findText(employeeData->status);
    if (index >= 0) {
        statusCombo->setCurrentIndex(index);
    }
}

Employee EmployeeDialog::getData() const
{
    Employee employee;
    employee.firstName = firstNameInput->text();
    employee.lastName = lastNameInput->text();
    employee.cin = cinInput->text();
    employee.position = positionCombo->currentText();
    employee.salary = salaryInput->text();
    employee.date = dateInput->date().toString("dd/MM/yyyy");
    employee.status = statusCombo->currentText();

    return employee;
}

void EmployeeDialog::validateFirstName(const QString &text)
{
    // Autoriser les lettres, accents, espaces et tirets
    bool hasInvalidChars = text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"));

    if (text.isEmpty()) {
        firstNameErrorLabel->setText("Le prénom est obligatoire");
        firstNameErrorLabel->show();
        firstNameInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else if (hasInvalidChars) {
        firstNameErrorLabel->setText("utiliser que des lettres");
        firstNameErrorLabel->show();
        firstNameInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else {
        firstNameErrorLabel->hide();
        firstNameInput->setStyleSheet(getInputStyle());
    }
}

void EmployeeDialog::validateLastName(const QString &text)
{
    bool hasInvalidChars = text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"));

    if (text.isEmpty()) {
        lastNameErrorLabel->setText("Le nom est obligatoire");
        lastNameErrorLabel->show();
        lastNameInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else if (hasInvalidChars) {
        lastNameErrorLabel->setText("utiliser que des lettres");
        lastNameErrorLabel->show();
        lastNameInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else {
        lastNameErrorLabel->hide();
        lastNameInput->setStyleSheet(getInputStyle());
    }
}

void EmployeeDialog::validateCin(const QString &text)
{
    bool hasLetters = text.contains(QRegularExpression("[^0-9]"));

    if (text.isEmpty()) {
        cinErrorLabel->setText("Le CIN est obligatoire");
        cinErrorLabel->show();
        cinInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else if (hasLetters) {
        cinErrorLabel->setText("utiliser que des chiffres");
        cinErrorLabel->show();
        cinInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else if (text.length() != 8) {
        cinErrorLabel->setText(QString("Le CIN doit comporter exactement 8 chiffres (%1/8)").arg(text.length()));
        cinErrorLabel->show();
        cinInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else {
        cinErrorLabel->hide();
        cinInput->setStyleSheet(getInputStyle());
    }
}

void EmployeeDialog::validateSalary(const QString &text)
{
    bool hasLetters = text.contains(QRegularExpression("[^0-9]"));

    if (text.isEmpty()) {
        salaryErrorLabel->setText("Le salaire est obligatoire");
        salaryErrorLabel->show();
        salaryInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else if (hasLetters) {
        salaryErrorLabel->setText("utiliser que des chiffres");
        salaryErrorLabel->show();
        salaryInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
    } else {
        int salary = text.toInt();
        if (salary < 1000 || salary > 10000) {
            salaryErrorLabel->setText("le salaire doit être entre 1000 et 10000");
            salaryErrorLabel->show();
            salaryInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
        } else {
            salaryErrorLabel->hide();
            salaryInput->setStyleSheet(getInputStyle());
        }
    }
}

void EmployeeDialog::onSaveClicked()
{
    validateCin(cinInput->text());
    validateFirstName(firstNameInput->text());
    validateLastName(lastNameInput->text());
    validateSalary(salaryInput->text());

    // Check if error labels are hidden (valid)
    if (!cinErrorLabel->isHidden() || !firstNameErrorLabel->isHidden() || !lastNameErrorLabel->isHidden() || !salaryErrorLabel->isHidden()) {
        showError("Veuillez corriger les erreurs de saisie avant d'enregistrer.");
        return;
    }

    accept();
}

void EmployeeDialog::showError(const QString& msg)
{
    QMessageBox::warning(this, "Validation", msg);
}
