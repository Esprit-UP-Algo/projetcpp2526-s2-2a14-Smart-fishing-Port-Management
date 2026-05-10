#ifndef LIVRAISON_H
#define LIVRAISON_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Livraison
{
private:
    QString date, adresse, statut, transport, vehicule, prix, reference, idEmploye;
    int id, dureeMinutes;
    static QString lastError;

public:
    Livraison();
    Livraison(int id, QString date, QString adresse, QString statut, QString transport, QString vehicule, QString prix, int duree, QString ref, QString idEmp);

    // Getters
    int getID() const { return id; }
    QString getDate() const { return date; }
    QString getAdresse() const { return adresse; }
    QString getStatut() const { return statut; }
    QString getTransport() const { return transport; }
    QString getVehicule() const { return vehicule; }
    QString getPrix() const { return prix; }
    QString getReference() const { return reference; }
    QString getIdEmploye() const { return idEmploye; }
    int getDuree() const { return dureeMinutes; }
    QString getETA() const; // New method for ETA

    // Setters
    void setID(int v) { id = v; }
    void setDate(QString v) { date = v; }
    void setAdresse(QString v) { adresse = v; }
    void setStatut(QString v) { statut = v; }
    void setTransport(QString v) { transport = v; }
    void setVehicule(QString v) { vehicule = v; }
    void setPrix(QString v) { prix = v; }
    void setDuree(int v) { dureeMinutes = v; }
    void setReference(QString v) { reference = v; }
    void setIdEmploye(QString v) { idEmploye = v; }

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);
    
    static bool idExists(int id);
    static bool refExists(QString ref);
    static QString getLastError() { return lastError; }
};

#endif // LIVRAISON_H
