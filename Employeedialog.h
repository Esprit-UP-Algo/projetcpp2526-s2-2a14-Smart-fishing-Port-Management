#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include "Employeewindow.h"

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr, Employee* employeeData = nullptr);
    ~EmployeeDialog();

    Employee getData() const;

private slots:
    void validateCin(const QString &text);
    void onSaveClicked();

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;

    QLineEdit* firstNameInput;
    QLineEdit* lastNameInput;
    QLineEdit* cinInput;
    QLabel* cinErrorLabel;
    QLineEdit* positionInput;
    QLineEdit* salaryInput;
    QDateEdit* dateInput;
    QComboBox* statusCombo;

    Employee* employeeData;
    bool isEdit;
};

#endif // EMPLOYEEDIALOG_H
