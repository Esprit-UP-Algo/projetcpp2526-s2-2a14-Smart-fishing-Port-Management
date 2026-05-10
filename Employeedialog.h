#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include "Employeewindow.h"

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr, Employee* employeeData = nullptr);
    ~EmployeeDialog();

    Employee getData() const;

private slots:
    void validateFirstName(const QString &text);
    void validateLastName(const QString &text);
    void validateCin(const QString &text);
    void validateSalary(const QString &text);
    void validatePhone(const QString &text);
    void validateEmail(const QString &text);
    void onSaveClicked();
    void onScanRfid();
    void updateRfidField(const QString& uid);

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;
    void updateFieldStyle(QWidget* field, bool isValid);

    QLineEdit* firstNameInput;
    QLabel* firstNameErrorLabel;
    QLineEdit* lastNameInput;
    QLabel* lastNameErrorLabel;
    QLineEdit* cinInput;
    QLabel* cinErrorLabel;
    QComboBox* positionCombo;
    QLineEdit* salaryInput;
    QLabel* salaryErrorLabel;
    QLineEdit* phoneInput;
    QLabel* phoneErrorLabel;
    QLineEdit* emailInput;
    QLabel* emailErrorLabel;
    QDateEdit* dateInput;
    QComboBox* statusCombo;
    QLineEdit* rfidInput;
    QPushButton* scanBtn;

    Employee* employeeData;
    bool isEdit;
};

#endif // EMPLOYEEDIALOG_H
