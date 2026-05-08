#ifndef BATEAUDIALOG_H
#define BATEAUDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QMessageBox>
#include <QList>
#include <QPair>
#include "bateau.h"

class BateauDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BateauDialog(QWidget *parent = nullptr, Bateau* bateauData = nullptr);
    ~BateauDialog();

    void setEmployeeList(const QList<QPair<QString, QString>>& employees);
    void setQuaiList(const QList<QPair<QString, QString>>& quais);
    Bateau getData() const;

private slots:
    void onSave();

private:
    void setupUi();
    void populateFields();
    bool validateInputs();
    void showError(const QString& msg);
    QString getInputStyle() const;
    QString getRadioStyle() const;

    QLineEdit* nomBateauInput;
    QLineEdit* immatriculationInput;
    QLineEdit* capaciteInput;
    QLineEdit* longueurInput;
    QLineEdit* ageInput;
    QDateEdit* dateMaintenanceInput;
    QComboBox* employeeCombo;
    QComboBox* quaiCombo;
    QComboBox* etatCombo;
    QLineEdit* codeSecInput;

    Bateau* bateauData;
    bool isEdit;
};

#endif // BATEAUDIALOG_H
