#ifndef QUAI_H
#define QUAI_H

#include <QString>

class Quai {
public:
    Quai(); // default constructor
    Quai(QString id, QString nom, int capacite,
         double tailleMax, QString statut,
         double tarif, QString client); // parameterized constructor

    QString getId() const;
    QString getNom() const;
    int getCapacite() const;
    double getTailleMax() const;
    QString getStatut() const;
    double getTarif() const;
    QString getClient() const;

    void setId(const QString &id);
    void setNom(const QString &nom);
    void setCapacite(int capacite);
    void setTailleMax(double tailleMax);
    void setStatut(const QString &statut);
    void setTarif(double tarif);
    void setClient(const QString &client);

private:
    QString id;
    QString nom;
    int capacite;
    double tailleMax;
    QString statut;
    double tarif;
    QString client;
};
#endif
