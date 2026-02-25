#include "quai.h"

// Default constructor
Quai::Quai()
{
    capacite = 0;
    tailleMax = 0.0;
    tarif = 0.0;
}

// Parameterized constructor
Quai::Quai(QString id, QString nom, int capacite,
           double tailleMax, QString statut,
           double tarif, QString client)
{
    this->id = id;
    this->nom = nom;
    this->capacite = capacite;
    this->tailleMax = tailleMax;
    this->statut = statut;
    this->tarif = tarif;
    this->client = client;
}

// Getters
QString Quai::getId() const { return id; }
QString Quai::getNom() const { return nom; }
int Quai::getCapacite() const { return capacite; }
double Quai::getTailleMax() const { return tailleMax; }
QString Quai::getStatut() const { return statut; }
double Quai::getTarif() const { return tarif; }
QString Quai::getClient() const { return client; }

// Setters
void Quai::setId(const QString &id) { this->id = id; }
void Quai::setNom(const QString &nom) { this->nom = nom; }
void Quai::setCapacite(int capacite) { this->capacite = capacite; }
void Quai::setTailleMax(double tailleMax) { this->tailleMax = tailleMax; }
void Quai::setStatut(const QString &statut) { this->statut = statut; }
void Quai::setTarif(double tarif) { this->tarif = tarif; }
void Quai::setClient(const QString &client) { this->client = client; }
