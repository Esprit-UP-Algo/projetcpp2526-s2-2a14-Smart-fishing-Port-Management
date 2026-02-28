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
    FrigoModel(int id, QString ref, double cap, double temp, QString stat, QString type, double occ);

    // Getters
    int getId() const { return idFrigo; }
    QString getRef() const { return reference; }
    double getCap() const { return capacite; }
    double getTemp() const { return temperature; }
    QString getStat() const { return statut; }
    QString getType() const { return typePoisson; }
    double getOcc() const { return occupation; }

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* rechercher(QString val);
    QSqlQueryModel* trier(QString critere, QString ordre);

private:
    int idFrigo;
    QString reference;
    double capacite;
    double temperature;
    QString statut;
    QString typePoisson;
    double occupation;
};

#endif // FRIGO_H
