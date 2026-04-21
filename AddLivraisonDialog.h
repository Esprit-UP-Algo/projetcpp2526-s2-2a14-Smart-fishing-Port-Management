#ifndef ADDLIVRAISONDIALOG_H
#define ADDLIVRAISONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
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
    void onAddressValidationFinished(QNetworkReply* reply);
    void onReferenceChanged();
    void onAdresseChanged();
    void onPrixChanged();
    void onAddressDebounceTimeout();

private:
    void setupUi();
    void populateFields();
    bool validateInputs();
    void updateFieldStyle(QWidget* field, bool isValid);
    void validateAddressViaAPI(const QString& address);
    void populateLivreurCombo();

    QLineEdit* referenceEdit;
    QComboBox* livreurCombo;
    QTextEdit* adresseEdit;
    QDateEdit* dateEdit;
    QComboBox* vehiculeEdit;
    QComboBox* transportEdit;
    QLineEdit* prixEdit;

    QLabel* errorReference;
    QLabel* errorAdresse;
    QLabel* errorVehicule;
    QLabel* errorPrix;

    QNetworkAccessManager* networkManager;
    QTimer* addressDebounceTimer;
    Livraison* livraisonData;
    bool isEdit;
    bool addressValidating;
    bool addressFound;
    bool waitingForSave;
};

#endif // ADDLIVRAISONDIALOG_H
