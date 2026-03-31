#ifndef BATEAUDIALOG_H
#define BATEAUDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QLabel>
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
    QLabel* nomErrorLabel;
    QLineEdit* immatriculationInput;
    QLabel* immatErrorLabel;
    QLineEdit* capaciteInput;
    QLabel* capErrorLabel;
    QLineEdit* longueurInput;
    QLabel* lonErrorLabel;
    QLineEdit* ageInput;
    QLabel* ageErrorLabel;
    QDateEdit* dateMaintenanceInput;
    QComboBox* employeeCombo;
    QComboBox* quaiCombo;
    QComboBox* etatCombo;

    Bateau* bateauData;
    bool isEdit;
};

#endif // BATEAUDIALOG_H
