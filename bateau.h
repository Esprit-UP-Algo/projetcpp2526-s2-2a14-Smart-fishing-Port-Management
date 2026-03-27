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
    QString disponible;
    QString idEmploye;
    QString idQuai;
    static QString lastError;

public:
    Bateau();
    Bateau(QString id, QString nom, QString imm, QString cap, QString lon, QString age, QString date, QString disp, QString idE, QString idQ);

    // Getters
    QString getIdBateau() const { return idBateau; }
    QString getNomBateau() const { return nomBateau; }
    QString getImmatriculation() const { return immatriculation; }
    QString getCapacite() const { return capacite; }
    QString getLongueur() const { return longueur; }
    QString getAgeBateau() const { return ageBateau; }
    QString getDateMaintenance() const { return dateMaintenance; }
    QString getDisponible() const { return disponible; }
    QString getIdEmploye() const { return idEmploye; }
    QString getIdQuai() const { return idQuai; }

    // Setters
    void setIdBateau(QString s) { idBateau = s; }
    void setNomBateau(QString s) { nomBateau = s; }
    void setImmatriculation(QString s) { immatriculation = s; }
    void setCapacite(QString s) { capacite = s; }
    void setLongueur(QString s) { longueur = s; }
    void setAgeBateau(QString s) { ageBateau = s; }
    void setDateMaintenance(QString s) { dateMaintenance = s; }
    void setDisponible(QString s) { disponible = s; }
    void setIdEmploye(QString s) { idEmploye = s; }
    void setIdQuai(QString s) { idQuai = s; }

    // CRUD Methods (Architecture Modèle-Vue + Sécurité)
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    
    QSqlQueryModel* trier(QString critere, QString ordre);
    QSqlQueryModel* rechercher(QString val);

    static QString getLastError() { return lastError; }
};

#endif // BATEAU_H
