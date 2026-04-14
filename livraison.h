#ifndef LIVRAISON_H
#define LIVRAISON_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Livraison
{
private:
    QString id, date, adresse, statut, transport, vehicule, prix;
    int dureeMinutes;
    static QString lastError;

public:
    Livraison();
    Livraison(QString id, QString date, QString adresse, QString statut, QString transport, QString vehicule, QString prix, int duree = 0);

    // Getters
    QString getID() const { return id; }
    QString getDate() const { return date; }
    QString getAdresse() const { return adresse; }
    QString getStatut() const { return statut; }
    QString getTransport() const { return transport; }
    QString getVehicule() const { return vehicule; }
    QString getPrix() const { return prix; }
    int getDuree() const { return dureeMinutes; }

    // Setters
    void setID(QString v) { id = v; }
    void setDate(QString v) { date = v; }
    void setAdresse(QString v) { adresse = v; }
    void setStatut(QString v) { statut = v; }
    void setTransport(QString v) { transport = v; }
    void setVehicule(QString v) { vehicule = v; }
    void setPrix(QString v) { prix = v; }
    void setDuree(int v) { dureeMinutes = v; }

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);
    
    static bool idExists(QString id);
    static QString getLastError() { return lastError; }
};

#endif // LIVRAISON_H
