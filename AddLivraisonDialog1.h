#ifndef ADDLIVRAISONDIALOG1_H
#define ADDLIVRAISONDIALOG1_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
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

private:
    void setupUi();
    void populateFields();
    bool validateInputs();
    void updateFieldStyle(QWidget* field, bool isValid);

    QTextEdit* adresseEdit;
    QLineEdit* vehiculeEdit;
    QComboBox* transportEdit;
    QLineEdit* prixEdit;
    QDateEdit* dateEdit;

    // Error Labels
    QLabel* errorAdresse;
    QLabel* errorVehicule;
    QLabel* errorPrix;

    Livraison* livraisonData;
    bool isEdit;

private slots:
    void onAdresseChanged();
    void onVehiculeChanged();
    void onPrixChanged();
    void handleSave();
};

#endif // ADDLIVRAISONDIALOG_H
