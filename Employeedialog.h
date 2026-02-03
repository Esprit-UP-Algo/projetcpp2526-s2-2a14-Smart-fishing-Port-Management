#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include "employeewindow.h"

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr, Employee* employeeData = nullptr);
    ~EmployeeDialog();

    Employee getData() const;

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;
    QString getRadioStyle() const;

    QLineEdit* firstNameInput;  // Changed from idInput
    QLineEdit* lastNameInput;   // Added
    QComboBox* positionCombo;
    QLineEdit* salaryInput;
    QDateEdit* dateInput;
    QRadioButton* actifRadio;
    QRadioButton* congeRadio;
    QRadioButton* inactifRadio;
    QButtonGroup* statusGroup;

    Employee* employeeData;
    bool isEdit;
};

#endif // EMPLOYEEDIALOG_H
