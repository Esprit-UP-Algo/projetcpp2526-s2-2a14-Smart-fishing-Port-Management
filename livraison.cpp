#include "livraison.h"
#include <QDebug>
#include <QSqlError>
#include <QDate>

QString Livraison::lastError = "";

Livraison::Livraison() {
    id = 0; date = ""; adresse = ""; statut = ""; transport = ""; vehicule = ""; prix = ""; reference = ""; idEmploye = "";
    dureeMinutes = 0;
}

Livraison::Livraison(int id, QString date, QString adresse, QString statut, QString transport, QString vehicule, QString prix, int duree, QString ref, QString idEmp) {
    this->id = id;
    this->date = date;
    this->adresse = adresse;
    this->statut = statut;
    this->transport = transport;
    this->vehicule = vehicule;
    this->prix = prix;
    this->dureeMinutes = duree;
    this->reference = ref;
    this->idEmploye = idEmp;
}

bool Livraison::ajouter() {
    QSqlQuery query;
    // Map to actual DB columns: IDLIVRAISON (Sequence), DATELIVRAISON, ADRESSELIVRAISON, STATUT, TYPETRANSPORT, PRIXLIVRAISON, VEHICULE, DUREE, REFERENCE, ID_EMPLOYE
    query.prepare("INSERT INTO LIVRAISONS (IDLIVRAISON, DATELIVRAISON, ADRESSELIVRAISON, STATUT, TYPETRANSPORT, PRIXLIVRAISON, VEHICULE, DUREE, REFERENCE, ID_EMPLOYE) "
                  "VALUES (LIVRAISON_SEQ.NEXTVAL, :date, :adresse, :statut, :transport, :prix, :vehicule, :duree, :ref, :idEmp)");

    // id is handled by sequence
    query.bindValue(":ref", reference);
    query.bindValue(":idEmp", idEmploye);
    
    // Parse date string to QDate for DATELIVRAISON
    QDate qdate = QDate::fromString(date, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(date, Qt::ISODate);
    query.bindValue(":date", qdate);
    
    query.bindValue(":adresse", adresse);
    query.bindValue(":statut", statut);
    query.bindValue(":transport", transport);
    
    QString p = prix;
    p = p.replace("DT", "").replace("$", "").replace("€", "").trimmed();
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
    // Use stable uppercase aliases for reliable mapping in UI
    model->setQuery("SELECT IDLIVRAISON as ID, REFERENCE, DATELIVRAISON as DATELIV, "
                    "ADRESSELIVRAISON as ADRESSE, ID_EMPLOYE, STATUT as STATUT, TYPETRANSPORT as TRANSPORT, "
                    "VEHICULE as VEHICULE, PRIXLIVRAISON as PRIX, DUREE as DUREE, "
                    "DATE_DEPART, DUREE_ESTIMEE FROM LIVRAISONS");
    return model;
}

bool Livraison::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

bool Livraison::modifier(int id) {
    QSqlQuery query;
    query.prepare("UPDATE LIVRAISONS SET DATELIVRAISON = :date, ADRESSELIVRAISON = :adresse, STATUT = :statut, "
                  "TYPETRANSPORT = :transport, PRIXLIVRAISON = :prix, VEHICULE = :vehicule, DUREE = :duree, "
                  "REFERENCE = :ref, ID_EMPLOYE = :idEmp "
                  "WHERE IDLIVRAISON = :id");
 
    query.bindValue(":id", id);
    
    QDate qdate = QDate::fromString(date, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(date, Qt::ISODate);
    query.bindValue(":date", qdate);
    
    query.bindValue(":adresse", adresse);
    query.bindValue(":statut", statut);
    query.bindValue(":transport", transport);
    
    QString p = prix;
    p = p.replace("DT", "").replace("$", "").replace("€", "").trimmed();
    query.bindValue(":prix", p.toDouble());
    
    query.bindValue(":vehicule", vehicule);
    query.bindValue(":duree", dureeMinutes);
    query.bindValue(":ref", reference);
    query.bindValue(":idEmp", idEmploye);

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

    QString queryString = QString("SELECT IDLIVRAISON as ID, REFERENCE, DATELIVRAISON as DATELIV, "
                                  "ADRESSELIVRAISON as ADRESSE, ID_EMPLOYE, STATUT as STATUT, TYPETRANSPORT as TRANSPORT, "
                                  "VEHICULE as VEHICULE, PRIXLIVRAISON as PRIX, DUREE as DUREE, "
                                  "DATE_DEPART, DUREE_ESTIMEE "
                                  "FROM LIVRAISONS ORDER BY %1 %2").arg(dbCritere, ordre);
    model->setQuery(queryString);
    return model;
}

QSqlQueryModel* Livraison::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT IDLIVRAISON as ID, REFERENCE, DATELIVRAISON as DATELIV, "
                  "ADRESSELIVRAISON as ADRESSE, ID_EMPLOYE, STATUT as STATUT, TYPETRANSPORT as TRANSPORT, "
                  "VEHICULE as VEHICULE, PRIXLIVRAISON as PRIX, DUREE as DUREE, "
                  "DATE_DEPART, DUREE_ESTIMEE "
                  "FROM LIVRAISONS WHERE IDLIVRAISON LIKE :val OR REFERENCE LIKE :val OR ADRESSELIVRAISON LIKE :val OR STATUT LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(query);
    return model;
}
bool Livraison::idExists(int id) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool Livraison::refExists(QString ref) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM LIVRAISONS WHERE REFERENCE = :ref");
    query.bindValue(":ref", ref);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QString Livraison::getETA() const {
    QSqlQuery query;
    query.prepare("SELECT DATE_DEPART, DUREE_ESTIMEE FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        QDateTime depart = query.value(0).toDateTime();
        double totalDureeSecs = query.value(1).toDouble();
        
        if (depart.isValid() && totalDureeSecs > 0) {
            QDateTime arrivalTime = depart.addSecs((int)totalDureeSecs);
            qint64 diff = QDateTime::currentDateTime().secsTo(arrivalTime);
            
            if (diff <= 0) return "Arrivé";
            
            // Format logic based on user rules
            if (diff < 60) {
                return QString::number(diff) + "s";
            } else if (diff < 3600) {
                return QString::number(diff / 60) + "m";
            } else if (diff < 86400) {
                return QString::number(diff / 3600) + "h";
            } else if (diff < 604800) {
                return QString::number(diff / 86400) + "j";
            } else {
                return QString::number(diff / 604800) + " sem";
            }
        }
    }
    return "N/A";
}

