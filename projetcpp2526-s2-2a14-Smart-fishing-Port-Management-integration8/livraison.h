#ifndef LIVRAISON_H
#define LIVRAISON_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Livraison {
    QString id, date, adresse, statut, transport, vehicule, prix;
    int dureeMinutes;
    static QString lastError;

public:
    Livraison();
    Livraison(QString, QString, QString, QString, QString, QString, QString, int);

    // Getters
    QString getID() { return id; }
    QString getDate() { return date; }
    QString getAdresse() { return adresse; }
    QString getStatut() { return statut; }
    QString getTransport() { return transport; }
    QString getVehicule() { return vehicule; }
    QString getPrix() { return prix; }
    int getDuree() { return dureeMinutes; }

    // Setters
    void setID(QString s) { id = s; }
    void setDate(QString s) { date = s; }
    void setAdresse(QString s) { adresse = s; }
    void setStatut(QString s) { statut = s; }
    void setTransport(QString s) { transport = s; }
    void setVehicule(QString s) { vehicule = s; }
    void setPrix(QString s) { prix = s; }
    void setDuree(int d) { dureeMinutes = d; }

    // CRUD Methods
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString);
    bool modifier(QString);
    
    // Sort & Search
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);

    // Error Reporting
    static QString getLastError() { return lastError; }
};

#endif // LIVRAISON_H
