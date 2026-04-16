#ifndef BATEAU_H
#define BATEAU_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Bateau {
    QString idBateau;
    QString nomBateau;
    QString immatriculation;
    QString capacite;
    QString longueur;
    QString ageBateau;
    QString dateMaintenance;
    QString idEmploye;
    QString idQuai;
    QString etat;
    static QString lastError;

public:
    Bateau();
    Bateau(QString id, QString nom, QString imm, QString cap, QString lon, QString age, QString date, QString idE, QString idQ, QString etatC = "Au port");

    // Getters
    QString getIdBateau() const { return idBateau; }
    QString getNomBateau() const { return nomBateau; }
    QString getImmatriculation() const { return immatriculation; }
    QString getCapacite() const { return capacite; }
    QString getLongueur() const { return longueur; }
    QString getAgeBateau() const { return ageBateau; }
    QString getDateMaintenance() const { return dateMaintenance; }
    QString getIdEmploye() const { return idEmploye; }
    QString getIdQuai() const { return idQuai; }
    QString getEtat() const { return etat; }

    // Setters
    void setIdBateau(QString s) { idBateau = s; }
    void setNomBateau(QString s) { nomBateau = s; }
    void setImmatriculation(QString s) { immatriculation = s; }
    void setCapacite(QString s) { capacite = s; }
    void setLongueur(QString s) { longueur = s; }
    void setAgeBateau(QString s) { ageBateau = s; }
    void setDateMaintenance(QString s) { dateMaintenance = s; }
    void setIdEmploye(QString s) { idEmploye = s; }
    void setIdQuai(QString s) { idQuai = s; }
    void setEtat(QString e) { etat = e; }

    // CRUD Methods (Architecture Modèle-Vue + Sécurité)
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);

    // [NOUVEAU] Unicité
    static bool immatriculationExiste(QString imm, int idBateauExclu = -1);

    static QString getLastError() { return lastError; }
};

#endif // BATEAU_H
