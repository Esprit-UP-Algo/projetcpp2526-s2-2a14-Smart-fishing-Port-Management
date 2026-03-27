#ifndef BATEAUDIALOG_H
#define BATEAUDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QButtonGroup>
#include <QMessageBox>
#include "bateau.h"

class BateauDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BateauDialog(QWidget *parent = nullptr, Bateau* bateauData = nullptr);
    ~BateauDialog();

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
    QRadioButton* disponibleOuiRadio;
    QRadioButton* disponibleNonRadio;
    QButtonGroup* disponibleGroup;
    QComboBox* employeeCombo;
    QComboBox* quaiCombo;

    Bateau* bateauData;
    bool isEdit;
};

#endif // BATEAUDIALOG_H
