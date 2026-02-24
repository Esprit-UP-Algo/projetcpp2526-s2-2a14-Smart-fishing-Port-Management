#include "livraison.h"
#include <QDebug>
#include <QSqlError>
#include <QDate>

QString Livraison::lastError = "";

Livraison::Livraison() {
    id = ""; date = ""; adresse = ""; statut = ""; transport = ""; vehicule = ""; prix = "";
    dureeMinutes = 0;
}

Livraison::Livraison(QString id, QString date, QString adresse, QString statut, QString transport, QString vehicule, QString prix, int duree) {
    this->id = id;
    this->date = date;
    this->adresse = adresse;
    this->statut = statut;
    this->transport = transport;
    this->vehicule = vehicule;
    this->prix = prix;
    this->dureeMinutes = duree;
}

bool Livraison::ajouter() {
    QSqlQuery query;
    // Map to actual DB columns: IDLIVRAISON, DATELIVRAISON, ADRESSELIVRAISON, STATUT, TYPETRANSPORT, PRIXLIVRAISON, VEHICULE, DUREE
    query.prepare("INSERT INTO LIVRAISONS (IDLIVRAISON, DATELIVRAISON, ADRESSELIVRAISON, STATUT, TYPETRANSPORT, PRIXLIVRAISON, VEHICULE, DUREE) "
                  "VALUES (:id, :date, :adresse, :statut, :transport, :prix, :vehicule, :duree)");

    // Convert "LIV001" to number for IDLIVRAISON
    int idNum = id.startsWith("LIV") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    
    // Parse date string to QDate for DATELIVRAISON
    QDate qdate = QDate::fromString(date, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(date, Qt::ISODate);
    query.bindValue(":date", qdate);
    
    query.bindValue(":adresse", adresse);
    query.bindValue(":statut", statut);
    query.bindValue(":transport", transport);
    
    QString p = prix;
    p.replace("DT", "").trimmed();
    query.bindValue(":prix", p.toDouble());
    
    query.bindValue(":vehicule", vehicule);
    query.bindValue(":duree", dureeMinutes);

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        qDebug() << "Erreur lors de l'ajout de la livraison:" << lastError;
        return false;
    }
    query.finish();
    return true;
}

QSqlQueryModel* Livraison::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    // Use correct column names and format numeric ID back to LIVxxx for UI consistency
    model->setQuery("SELECT 'LIV' || LPAD(IDLIVRAISON, 3, '0') as ID, DATELIVRAISON as \"Date\", "
                    "ADRESSELIVRAISON as \"Adresse\", STATUT, TYPETRANSPORT as \"Transport\", "
                    "VEHICULE as \"Véhicule\", PRIXLIVRAISON as \"Prix\", DUREE as \"Durée\" FROM LIVRAISONS");
    return model;
}

bool Livraison::supprimer(QString id) {
    QSqlQuery query;
    query.prepare("DELETE FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    int idNum = id.startsWith("LIV") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

bool Livraison::modifier(QString id) {
    QSqlQuery query;
    query.prepare("UPDATE LIVRAISONS SET DATELIVRAISON = :date, ADRESSELIVRAISON = :adresse, STATUT = :statut, "
                  "TYPETRANSPORT = :transport, PRIXLIVRAISON = :prix, VEHICULE = :vehicule, DUREE = :duree "
                  "WHERE IDLIVRAISON = :id");

    int idNum = id.startsWith("LIV") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    
    QDate qdate = QDate::fromString(date, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(date, Qt::ISODate);
    query.bindValue(":date", qdate);
    
    query.bindValue(":adresse", adresse);
    query.bindValue(":statut", statut);
    query.bindValue(":transport", transport);
    
    QString p = prix;
    p.replace("DT", "").trimmed();
    query.bindValue(":prix", p.toDouble());
    
    query.bindValue(":vehicule", vehicule);
    query.bindValue(":duree", dureeMinutes);

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

QSqlQueryModel* Livraison::trier(QString critere, QString ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    // Map UI criteria to DB columns
    QString dbCritere = critere;
    if (critere == "ID") dbCritere = "IDLIVRAISON";
    else if (critere == "DATELIV") dbCritere = "DATELIVRAISON";
    else if (critere == "ADRESSE") dbCritere = "ADRESSELIVRAISON";
    else if (critere == "TRANSPORT") dbCritere = "TYPETRANSPORT";
    else if (critere == "PRIX") dbCritere = "PRIXLIVRAISON";

    QString queryString = QString("SELECT 'LIV' || LPAD(IDLIVRAISON, 3, '0') as ID, DATELIVRAISON as \"Date\", "
                                  "ADRESSELIVRAISON as \"Adresse\", STATUT, TYPETRANSPORT as \"Transport\", "
                                  "VEHICULE as \"Véhicule\", PRIXLIVRAISON as \"Prix\", DUREE as \"Durée\" "
                                  "FROM LIVRAISONS ORDER BY %1 %2").arg(dbCritere, ordre);
    model->setQuery(queryString);
    return model;
}

QSqlQueryModel* Livraison::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT 'LIV' || LPAD(IDLIVRAISON, 3, '0') as ID, DATELIVRAISON as \"Date\", "
                  "ADRESSELIVRAISON as \"Adresse\", STATUT, TYPETRANSPORT as \"Transport\", "
                  "VEHICULE as \"Véhicule\", PRIXLIVRAISON as \"Prix\", DUREE as \"Durée\" "
                  "FROM LIVRAISONS WHERE IDLIVRAISON LIKE :val OR ADRESSELIVRAISON LIKE :val OR STATUT LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(query);
    return model;
}
