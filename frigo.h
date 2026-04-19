#ifndef FRIGO_H
#define FRIGO_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>

class FrigoModel
{
public:
    FrigoModel();
    FrigoModel(QString id, QString ref, double cap, QString type, QString stat, QString dateRes, double temp, double occ, QString tel);

    // Getters
    QString getId() const { return idFrigo; }
    QString getRef() const { return reference; }
    double getCap() const { return capacite; }
    QString getType() const { return typePoisson; }
    QString getStat() const { return statut; }
    QString getDateRes() const { return dateReservation; }
    double getTemp() const { return temperature; }
    double getOcc() const { return occupation; }
    QString getTelephone() const { return telephone; }

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    QSqlQueryModel* rechercher(QString val);
    QSqlQueryModel* trier(QString critere, QString ordre);
    QString getLastError() const { return m_lastError; }

private:
    QString idFrigo;
    QString reference;
    double capacite;
    QString typePoisson;
    QString statut;
    QString dateReservation;
    double temperature;
    double occupation;
    QString telephone;
    QString m_lastError;
};

#endif // FRIGO_H
