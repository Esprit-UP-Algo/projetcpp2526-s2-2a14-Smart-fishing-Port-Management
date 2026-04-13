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
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateEdit>

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

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");

    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(25);
    formLayout->setContentsMargins(40, 40, 40, 40);

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(20);

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
    formLayout->addWidget(cinInput);
    cinErrorLabel = new QLabel("");
    cinErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    cinErrorLabel->hide();
    formLayout->addWidget(cinErrorLabel);
    connect(cinInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateCin);

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

    QLabel* salaireLabel = new QLabel("Salaire");
    salaireLabel->setFont(labelFont);
    salaireLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(salaireLabel);
    salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("1200");
    salaryInput->setFont(inputFont);
    salaryInput->setFixedHeight(50);
    salaryInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(salaryInput);
    salaryErrorLabel = new QLabel("");
    salaryErrorLabel->setStyleSheet("color: #E74C3C; font-size: 11px; font-weight: bold; margin-top: -5px;");
    salaryErrorLabel->hide();
    formLayout->addWidget(salaryErrorLabel);
    connect(salaryInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateSalary);

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

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedSize(140, 50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background-color: #E8EEF5; color: #5A6C7D; border-radius: 10px; font-weight: 600; }
        QPushButton:hover { background-color: #D8DEE5; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton("Enregistrer");
    saveBtn->setFixedSize(160, 50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #5D9CEC; color: white; border-radius: 10px; font-weight: 700; }
        QPushButton:hover { background-color: #4A89DC; }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &EmployeeDialog::onSaveClicked);
    buttonLayout->addWidget(saveBtn);
    formLayout->addLayout(buttonLayout);

    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);
}

QString EmployeeDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F8F9FA; border: 2px solid #E1E8ED; border-radius: 10px;
            padding: 12px 15px; color: #2C3E50; font-size: 12px;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 2px solid #5D9CEC; background-color: white; }
        QComboBox::drop-down, QDateEdit::drop-down { border: none; width: 30px; }
    )";
}

void EmployeeDialog::populateFields()
{
    if (!employeeData) return;
    firstNameInput->setText(employeeData->firstName);
    lastNameInput->setText(employeeData->lastName);
    cinInput->setText(employeeData->cin);
    int posIndex = positionCombo->findText(employeeData->position);
    if (posIndex >= 0) positionCombo->setCurrentIndex(posIndex);
    QString cleanSal = employeeData->salary;
    cleanSal.remove(QRegularExpression("[^0-9]"));
    salaryInput->setText(cleanSal);
    QStringList dateParts = employeeData->date.split("/");
    if (dateParts.size() == 3) dateInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    int index = statusCombo->findText(employeeData->status);
    if (index >= 0) statusCombo->setCurrentIndex(index);
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
    if (text.isEmpty()) { firstNameErrorLabel->setText("Le prénom est obligatoire"); firstNameErrorLabel->show(); }
    else if (text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"))) { firstNameErrorLabel->setText("utiliser que des lettres"); firstNameErrorLabel->show(); }
    else firstNameErrorLabel->hide();
}

void EmployeeDialog::validateLastName(const QString &text)
{
    if (text.isEmpty()) { lastNameErrorLabel->setText("Le nom est obligatoire"); lastNameErrorLabel->show(); }
    else if (text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"))) { lastNameErrorLabel->setText("utiliser que des lettres"); lastNameErrorLabel->show(); }
    else lastNameErrorLabel->hide();
}

void EmployeeDialog::validateCin(const QString &text)
{
    if (text.isEmpty()) { cinErrorLabel->setText("Le CIN est obligatoire"); cinErrorLabel->show(); }
    else if (text.contains(QRegularExpression("[^0-9]"))) { cinErrorLabel->setText("utiliser que des chiffres"); cinErrorLabel->show(); }
    else if (text.length() != 8) { cinErrorLabel->setText(QString("Le CIN doit comporter 8 chiffres (%1/8)").arg(text.length())); cinErrorLabel->show(); }
    else cinErrorLabel->hide();
}

void EmployeeDialog::validateSalary(const QString &text)
{
    if (text.isEmpty()) { salaryErrorLabel->setText("Le salaire est obligatoire"); salaryErrorLabel->show(); }
    else if (text.contains(QRegularExpression("[^0-9]"))) { salaryErrorLabel->setText("utiliser que des chiffres"); salaryErrorLabel->show(); }
    else if (text.toInt() < 1000 || text.toInt() > 10000) { salaryErrorLabel->setText("le salaire doit être entre 1000 et 10000"); salaryErrorLabel->show(); }
    else salaryErrorLabel->hide();
}

void EmployeeDialog::onSaveClicked()
{
    validateCin(cinInput->text());
    validateFirstName(firstNameInput->text());
    validateLastName(lastNameInput->text());
    validateSalary(salaryInput->text());

    if (!cinErrorLabel->isHidden() || !firstNameErrorLabel->isHidden() || !lastNameErrorLabel->isHidden() || !salaryErrorLabel->isHidden()) return;

    QString currentCin = cinInput->text();
    QSqlQuery checkQuery;
    QString sql = "SELECT COUNT(*) FROM EMPLOYEES WHERE CIN = :cin";
    if (isEdit && employeeData) sql += " AND ID_EMPLOYE != :id";
    checkQuery.prepare(sql);
    checkQuery.bindValue(":cin", currentCin);
    if (isEdit && employeeData) checkQuery.bindValue(":id", employeeData->id.toInt());

    bool exists = false;
    if (checkQuery.exec() && checkQuery.next()) { if (checkQuery.value(0).toInt() > 0) exists = true; }
    else {
        QSqlQuery checkQuery2;
        QString sql2 = "SELECT COUNT(*) FROM EMPLOYEE WHERE CIN = :cin";
        if (isEdit && employeeData) sql2 += " AND ID_EMPLOYE != :id";
        checkQuery2.prepare(sql2);
        checkQuery2.bindValue(":cin", currentCin);
        if (isEdit && employeeData) checkQuery2.bindValue(":id", employeeData->id.toInt());
        if (checkQuery2.exec() && checkQuery2.next()) { if (checkQuery2.value(0).toInt() > 0) exists = true; }
    }

    if (exists) {
        QMessageBox::critical(this, "Erreur", "Le CIN existe déjà.");
        return;
    }
    accept();
}
