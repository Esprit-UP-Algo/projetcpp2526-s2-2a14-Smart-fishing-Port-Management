#include "quai.h"

// Default constructor
Quai::Quai()
    : numero(0), capacite(0), etat("Disponible"),
    tarif(0.0), location(""), dureeLocation("")
{}

// Parameterized constructor
Quai::Quai(int numero, int capacite, QString etat, double tarif,
           QString location, const QString& dureeLocation)
    : numero(numero), capacite(capacite), etat(etat), tarif(tarif),
    location(location), dureeLocation(dureeLocation)
{}

// Getters
int Quai::getNumero() const { return numero; }
int Quai::getCapacite() const { return capacite; }
QString Quai::getEtat() const { return etat; }
double Quai::getTarif() const { return tarif; }
QString Quai::getLocation() const { return location; }
QString Quai::getDureeLocation() const { return dureeLocation; }


// Setters
void Quai::setNumero(int n) { numero = n; }
void Quai::setCapacite(int c) { capacite = c; }
void Quai::setEtat(const QString &e) { etat = e; }
void Quai::setTarif(double t) { tarif = t; }
void Quai::setLocation(const QString &l) { location = l; }
void Quai::setDureeLocation(const QString &d) { dureeLocation = d; }
