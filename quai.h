#ifndef QUAI_H
#define QUAI_H

#include <QList>
#include <QString>

struct HistoricData {
    int longueur;
    int tempsDockage;
};

class Quai {
private:
    int idQuai;
    int numero;
    int ordreNom;
    int capacite;
    QString etat;
    double tarif;
    QString location;
    QString dureeLocation;
    double tailleMax;

public:
    Quai(); // default
    Quai(int numero, int capacite, QString etat, double tarif, QString location,
         const QString& dureeLocation);
    // quai.h
public:
    bool ajouter(); // Add this line in the public section
    // Getters
    int getIdQuai() const;
    int getNumero() const;
    int getOrdreNom() const;
    int getCapacite() const;
    QString getEtat() const;
    double getTarif() const;
    QString getLocation() const;
    QString getDureeLocation() const;
    QString getReference() const;
    QString getNomQuai() const;
    static QList<HistoricData> historiqueParDefaut();
    static int calculerTempsEstime(int longueurActuelle, const QList<HistoricData>& historique = {});
    static int calculerCapaciteRecommandee(int longueurActuelle);
    bool peutAccueillirLongueur(int longueurActuelle) const;


    // Setters
    void setIdQuai(int id);
    void setNumero(int numero);
    void setOrdreNom(int ordre);
    void setCapacite(int capacite);
    void setEtat(const QString &etat);
    void setTarif(double tarif);
    void setLocation(const QString &location);
    void setDureeLocation(const QString &duree);

};

#endif
