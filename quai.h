#ifndef QUAI_H
#define QUAI_H

#include <QString>

class Quai {
private:
    int numero;
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
    int getNumero() const;
    int getCapacite() const;
    QString getEtat() const;
    double getTarif() const;
    QString getLocation() const;
    QString getDureeLocation() const;


    // Setters
    void setNumero(int numero);
    void setCapacite(int capacite);
    void setEtat(const QString &etat);
    void setTarif(double tarif);
    void setLocation(const QString &location);
    void setDureeLocation(const QString &duree);

};

#endif
