#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include "employee.h"

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr, EmployeeModel* employeeData = nullptr);
    ~EmployeeDialog();

    EmployeeModel getData() const;

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;

    QLineEdit* firstNameInput;
    QLineEdit* lastNameInput;
    QLineEdit* cinInput;
    QLineEdit* positionInput;
    QLineEdit* salaryInput;
    QDateEdit* dateInput;
    QComboBox* statusCombo;

    EmployeeModel* employeeData;
    bool isEdit;
};

#endif // EMPLOYEEDIALOG_H
