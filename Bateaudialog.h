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
    void updateFieldStyle(QWidget* field, bool isValid);
    QString getRadioStyle() const;

    QLineEdit* nomBateauInput;
    QLabel* nomErrorLabel;
    QLineEdit* immatriculationInput;
    QLabel* immatErrorLabel;
    QLabel* immatUnicityLabel;
    QLineEdit* capaciteInput;
    QLabel* capErrorLabel;
    QLineEdit* codeSecInput;
    QLabel* codeErrorLabel;
    QLineEdit* longueurInput;
    QLabel* longErrorLabel;
    QLineEdit* ageInput;
    QLabel* ageErrorLabel;
    QDateEdit* dateMaintenanceInput;
    QLabel* dateErrorLabel;
    QComboBox* employeeCombo;
    QComboBox* quaiCombo;
    QLabel* quaiErrorLabel;
    QComboBox* etatCombo;

    Bateau* bateauData;
    bool isEdit;
};

#endif // BATEAUDIALOG_H
