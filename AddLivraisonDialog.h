#ifndef ADDLIVRAISONDIALOG_H
#define ADDLIVRAISONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include "livraison.h"

class AddLivraisonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddLivraisonDialog(QWidget *parent = nullptr, Livraison* livraisonData = nullptr);
    ~AddLivraisonDialog();

    Livraison getData() const;

private slots:
    void handleSave();
    void onReferenceChanged();
    void onAdresseChanged();
    void onPrixChanged();

private:
    void setupUi();
    void populateFields();
    bool validateInputs();
    void updateFieldStyle(QWidget* field, bool isValid);
    void populateLivreurCombo();
    QString getInputStyle() const;

    QLineEdit* referenceEdit;
    QComboBox* livreurCombo;
    QTextEdit* adresseEdit;
    QDateEdit* dateEdit;
    QLineEdit* vehiculeEdit;
    QComboBox* transportEdit;
    QLineEdit* prixEdit;

    QLabel* errorReference;
    QLabel* errorAdresse;
    QLabel* errorVehicule;
    QLabel* errorPrix;

    Livraison* livraisonData;
    bool isEdit;
};

#endif // ADDLIVRAISONDIALOG_H
