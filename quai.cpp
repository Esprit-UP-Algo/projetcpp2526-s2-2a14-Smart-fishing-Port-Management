#include "quai.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <cmath>

// Default constructor
Quai::Quai()
    : idQuai(0), numero(0), ordreNom(0), capacite(0), etat("Disponible"),
    tarif(0.0), location(""), dureeLocation("")
{}

// Parameterized constructor
Quai::Quai(int numero, int capacite, QString etat, double tarif,
           QString location, const QString& dureeLocation)
    : idQuai(0), numero(numero), ordreNom(0), capacite(capacite), etat(etat), tarif(tarif),
    location(location), dureeLocation(dureeLocation)
{}

// --- THE DATABASE METHOD (This is what was in your slide) ---
bool Quai::ajouter()
{
    QSqlQuery query;

    // Prepare the query with placeholders
    query.prepare("INSERT INTO quai (NUMERO, CAPACITE, ETAT, TARIF, LOCATION, DUREELOCATION) "
                  "VALUES (:num, :cap, :etat, :tar, :loc, :dur)");

    // Bind the values from the class members
    query.bindValue(":num", numero);
    query.bindValue(":cap", capacite);
    query.bindValue(":etat", etat);
    query.bindValue(":tar", tarif);
    query.bindValue(":loc", location);
    query.bindValue(":dur", dureeLocation);

    if (!query.exec()) {
        qDebug() << "Error adding Quai:" << query.lastError().text();
        return false;
    }
    return true;
}

// Getters
int Quai::getIdQuai() const { return idQuai; }
int Quai::getNumero() const { return numero; }
int Quai::getOrdreNom() const { return ordreNom; }
int Quai::getCapacite() const { return capacite; }
QString Quai::getEtat() const { return etat; }
double Quai::getTarif() const { return tarif; }
QString Quai::getLocation() const { return location; }
QString Quai::getDureeLocation() const { return dureeLocation; }
QString Quai::getReference() const
{
    const QString numeroTexte = QString("%1").arg(std::abs(numero), 6, 10, QChar('0'));
    return QString("TUN-%1-%2").arg(numeroTexte.left(3), numeroTexte.mid(3, 3));
}
QString Quai::getNomQuai() const
{
    return QString("Quai %1").arg(ordreNom > 0 ? ordreNom : numero);
}

QList<HistoricData> Quai::historiqueParDefaut()
{
    return {
        {8, 35}, {10, 40}, {12, 45}, {14, 48}, {16, 55},
        {18, 58}, {20, 62}, {22, 68}, {24, 72}, {26, 79},
        {28, 83}, {30, 90}, {32, 95}, {35, 105}, {38, 115}
    };
}

int Quai::calculerTempsEstime(int longueurActuelle, const QList<HistoricData>& historique)
{
    const QList<HistoricData> data = historique.isEmpty() ? historiqueParDefaut() : historique;

    int sommeTemps = 0;
    int sommeLongueur = 0;
    int count = 0;

    for (const HistoricData& entree : data) {
        if (std::abs(entree.longueur - longueurActuelle) <= 10) {
            sommeTemps += entree.tempsDockage;
            sommeLongueur += entree.longueur;
            ++count;
        }
    }

    if (count > 0) {
        const int moyenneTemps = sommeTemps / count;
        const int moyenneLongueur = sommeLongueur / count;
        const int ajustement = (longueurActuelle - moyenneLongueur) * 2;
        return std::max(20, moyenneTemps + ajustement);
    }

    return 60;
}

int Quai::calculerCapaciteRecommandee(int longueurActuelle)
{
    return longueurActuelle;
}

bool Quai::peutAccueillirLongueur(int longueurActuelle) const
{
    return capacite >= longueurActuelle;
}

// Setters
void Quai::setIdQuai(int id) { idQuai = id; }
void Quai::setNumero(int n) { numero = n; }
void Quai::setOrdreNom(int ordre) { ordreNom = ordre; }
void Quai::setCapacite(int c) { capacite = c; }
void Quai::setEtat(const QString &e) { etat = e; }
void Quai::setTarif(double t) { tarif = t; }
void Quai::setLocation(const QString &l) { location = l; }
void Quai::setDureeLocation(const QString &d) { dureeLocation = d; }
