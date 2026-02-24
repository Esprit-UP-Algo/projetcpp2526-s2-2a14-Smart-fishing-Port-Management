#ifndef CONTRACTDIALOG_H
#define CONTRACTDIALOG_H

#include "quai.h"
#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>

class ContractDialog : public QDialog {
    Q_OBJECT
public:
    ContractDialog(const Quai& quai, QWidget* parent = nullptr);

    QString getClientName() const;
    QString getCompany() const;
    QString getDuration() const;
    QDate getStartDate() const;

private:
    QLineEdit* clientNameField = nullptr;
    QLineEdit* companyField    = nullptr;
    QLineEdit* emailField      = nullptr;
    QLineEdit* phoneField      = nullptr;
    QDateEdit* startDateField  = nullptr;
    QComboBox* durationField   = nullptr;
    Quai m_quai;
};

#endif // CONTRACTDIALOG_H
