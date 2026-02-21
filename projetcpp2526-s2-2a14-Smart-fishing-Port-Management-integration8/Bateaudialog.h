#ifndef BATEAUDIALOG_H
#define BATEAUDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include "bateauwindow.h"

class BateauDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BateauDialog(QWidget *parent = nullptr, Bateau* bateauData = nullptr);
    ~BateauDialog();

    Bateau getData() const;

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;
    QString getRadioStyle() const;

    QLineEdit* nomBateauInput;
    QLineEdit* immatriculationInput;
    QLineEdit* capacitePecheInput;
    QLineEdit* longueurInput;
    QLineEdit* ageBateauInput;
    QLineEdit* proprietaireInput;
    QComboBox* etatBateauCombo;
    QDateEdit* dateMaintenanceInput;
    QRadioButton* disponibleOuiRadio;
    QRadioButton* disponibleNonRadio;
    QButtonGroup* disponibleGroup;

    Bateau* bateauData;
    bool isEdit;
};

#endif // BATEAUDIALOG_H
