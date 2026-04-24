#ifndef PECHE_H
#define PECHE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Peche {
    QString idLot;
    QString reference;
    QString espece;
    QString quantiteKg;
    QString dateCapture;
    QString idBateau;
    QString idFrigo;
    QString idPecheur;
    static QString lastError;

public:
    Peche();
    Peche(QString id, QString ref, QString esp, QString qte, QString date, QString idB, QString idF, QString idP);

    // Getters
    QString getIdLot() const { return idLot; }
    QString getReference() const { return reference; }
    QString getEspece() const { return espece; }
    QString getQuantiteKg() const { return quantiteKg; }
    QString getDateCapture() const { return dateCapture; }
    QString getIdBateau() const { return idBateau; }
    QString getIdFrigo() const { return idFrigo; }
    QString getIdPecheur() const { return idPecheur; }

    // Setters
    void setIdLot(QString s) { idLot = s; }
    void setReference(QString s) { reference = s; }
    void setEspece(QString s) { espece = s; }
    void setQuantiteKg(QString s) { quantiteKg = s; }
    void setDateCapture(QString s) { dateCapture = s; }
    void setIdBateau(QString s) { idBateau = s; }
    void setIdFrigo(QString s) { idFrigo = s; }
    void setIdPecheur(QString s) { idPecheur = s; }

    // CRUD Methods
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    
    // Sort & Search
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);

    // [NOUVEAU] Unicité
    static bool referenceExiste(QString ref, int idLotExclu = -1);

    // Error Reporting
    static QString getLastError() { return lastError; }
};

#endif // PECHE_H
