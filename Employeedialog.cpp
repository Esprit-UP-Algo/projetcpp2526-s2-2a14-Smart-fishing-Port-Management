#include "employeedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>

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

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(40, 40);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
            font-size: 24px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
            border-radius: 20px;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(header);

    // Form content
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(25);
    formLayout->setContentsMargins(40, 40, 40, 40);

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    // First Name and Last Name (side by side)
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(20);

    // First Name
    QVBoxLayout* firstNameCol = new QVBoxLayout();
    firstNameCol->setSpacing(8);
    QLabel* firstNameLabel = new QLabel("Prénom");
    firstNameLabel->setFont(labelFont);
    firstNameLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    firstNameCol->addWidget(firstNameLabel);

    firstNameInput = new QLineEdit();
    firstNameInput->setPlaceholderText("Mohamed");
    firstNameInput->setFont(inputFont);
    firstNameInput->setFixedHeight(50);
    firstNameInput->setStyleSheet(getInputStyle());
    firstNameCol->addWidget(firstNameInput);
    nameRow->addLayout(firstNameCol);

    // Last Name
    QVBoxLayout* lastNameCol = new QVBoxLayout();
    lastNameCol->setSpacing(8);
    QLabel* lastNameLabel = new QLabel("Nom");
    lastNameLabel->setFont(labelFont);
    lastNameLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    lastNameCol->addWidget(lastNameLabel);

    lastNameInput = new QLineEdit();
    lastNameInput->setPlaceholderText("Ben Ali");
    lastNameInput->setFont(inputFont);
    lastNameInput->setFixedHeight(50);
    lastNameInput->setStyleSheet(getInputStyle());
    lastNameCol->addWidget(lastNameInput);
    nameRow->addLayout(lastNameCol);

    formLayout->addLayout(nameRow);
    formLayout->addSpacing(10);

    // Position and Salary (side by side)
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(20);

    QVBoxLayout* col1 = new QVBoxLayout();
    col1->setSpacing(8);
    QLabel* posLabel = new QLabel("Position");
    posLabel->setFont(labelFont);
    posLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    col1->addWidget(posLabel);

    positionCombo = new QComboBox();
    positionCombo->addItems({"Marin", "RH", "Technicien", "Sécurité"});
    positionCombo->setFont(inputFont);
    positionCombo->setFixedHeight(50);
    positionCombo->setStyleSheet(getInputStyle());
    col1->addWidget(positionCombo);
    row1->addLayout(col1);

    QVBoxLayout* col2 = new QVBoxLayout();
    col2->setSpacing(8);
    QLabel* salLabel = new QLabel("Salaire");
    salLabel->setFont(labelFont);
    salLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    col2->addWidget(salLabel);

    salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("1200 DT");
    salaryInput->setFont(inputFont);
    salaryInput->setFixedHeight(50);
    salaryInput->setStyleSheet(getInputStyle());
    col2->addWidget(salaryInput);
    row1->addLayout(col2);

    formLayout->addLayout(row1);

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
    statusLabel->setStyleSheet("color: #2C3E50; margin-bottom: 10px;");
    formLayout->addWidget(statusLabel);

    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(25);

    statusGroup = new QButtonGroup(this);

    actifRadio = new QRadioButton("Actif");
    QFont radioFont("Segoe UI", 11);
    actifRadio->setFont(radioFont);
    actifRadio->setChecked(true);
    actifRadio->setStyleSheet(getRadioStyle());
    actifRadio->setFixedHeight(40);
    statusGroup->addButton(actifRadio);
    statusLayout->addWidget(actifRadio);

    congeRadio = new QRadioButton("Congé");
    congeRadio->setFont(radioFont);
    congeRadio->setStyleSheet(getRadioStyle());
    congeRadio->setFixedHeight(40);
    statusGroup->addButton(congeRadio);
    statusLayout->addWidget(congeRadio);

    inactifRadio = new QRadioButton("Inactif");
    inactifRadio->setFont(radioFont);
    inactifRadio->setStyleSheet(getRadioStyle());
    inactifRadio->setFixedHeight(40);
    statusGroup->addButton(inactifRadio);
    statusLayout->addWidget(inactifRadio);

    statusLayout->addStretch();
    formLayout->addLayout(statusLayout);

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

    mainLayout->addWidget(content);
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

QString EmployeeDialog::getRadioStyle() const
{
    return R"(
        QRadioButton {
            color: #2C3E50;
            spacing: 10px;
            padding: 8px;
        }
        QRadioButton::indicator {
            width: 22px;
            height: 22px;
        }
        QRadioButton::indicator:unchecked {
            border: 3px solid #BDC3C7;
            border-radius: 11px;
            background-color: white;
        }
        QRadioButton::indicator:unchecked:hover {
            border: 3px solid #5D9CEC;
        }
        QRadioButton::indicator:checked {
            border: 3px solid #5D9CEC;
            border-radius: 11px;
            background-color: white;
        }
        QRadioButton::indicator:checked::after {
            width: 12px;
            height: 12px;
            border-radius: 6px;
            background-color: #5D9CEC;
        }
    )";
}

void EmployeeDialog::populateFields()
{
    if (!employeeData) return;

    firstNameInput->setText(employeeData->firstName);
    lastNameInput->setText(employeeData->lastName);

    int index = positionCombo->findText(employeeData->position);
    if (index >= 0) {
        positionCombo->setCurrentIndex(index);
    }

    salaryInput->setText(employeeData->salary);

    // Parse date
    QStringList dateParts = employeeData->date.split("/");
    if (dateParts.size() == 3) {
        dateInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    }

    // Set status
    if (employeeData->status == "Actif") {
        actifRadio->setChecked(true);
    } else if (employeeData->status == "Congé") {
        congeRadio->setChecked(true);
    } else {
        inactifRadio->setChecked(true);
    }
}

Employee EmployeeDialog::getData() const
{
    Employee emp;
    // ID will be auto-generated in EmployeeWindow
    emp.firstName = firstNameInput->text();
    emp.lastName = lastNameInput->text();
    emp.position = positionCombo->currentText();
    emp.salary = salaryInput->text();
    emp.date = dateInput->date().toString("dd/MM/yyyy");

    if (actifRadio->isChecked()) {
        emp.status = "Actif";
    } else if (congeRadio->isChecked()) {
        emp.status = "Congé";
    } else {
        emp.status = "Inactif";
    }

    return emp;
}
