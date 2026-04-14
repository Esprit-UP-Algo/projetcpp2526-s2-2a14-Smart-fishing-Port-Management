#include "peche.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QDate>
#include <QRegularExpression>

QString Peche::lastError = "";

Peche::Peche() {
    idLot = ""; reference = ""; espece = ""; quantiteKg = ""; dateCapture = ""; idBateau = ""; idFrigo = "";
}

Peche::Peche(QString id, QString ref, QString esp, QString qte, QString date, QString idB, QString idF) {
    this->idLot = id;
    this->reference = ref;
    this->espece = esp;
    this->quantiteKg = qte;
    this->dateCapture = date;
    this->idBateau = idB;
    this->idFrigo = idF;
}

bool Peche::ajouter() {
    // Validation de base
    if (reference.trimmed().isEmpty()) {
        lastError = "La référence ne peut pas être vide.";
        return false;
    }

    // Validation du format (REF-YYYY-NOMBRE)
    QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
    if (!refRegex.match(reference).hasMatch()) {
        lastError = "Format de référence invalide (Attendu: REF-YYYY-NOMBRE).";
        return false;
    }

    // [NOUVEAU] Contrôle d'unicité
    if (referenceExiste(reference)) {
        lastError = "Cette référence existe déjà.";
        return false;
    }

    bool ok;
    double val = quantiteKg.toDouble(&ok);
    if (!ok || val <= 0) {
        lastError = "La quantité doit être un nombre positif.";
        return false;
    }

    QSqlQuery query;
    
    // [LOGIQUE INNOVANTE] Vérification du Quota Mensuel (ex: 5000 Kg)
    QSqlQuery quotaQuery;
    quotaQuery.prepare("SELECT SUM(QUANTITE) FROM PECHES WHERE TO_CHAR(DATECAPTURE, 'MM/YYYY') = :monthYear");
    quotaQuery.bindValue(":monthYear", QDate::currentDate().toString("MM/yyyy"));
    if (quotaQuery.exec() && quotaQuery.next()) {
        double currentTotal = quotaQuery.value(0).toDouble();
        double nextTotal = currentTotal + quantiteKg.toDouble();
        if (nextTotal > 5000.0) {
            // [ALERTE EMAIL SIMULÉE] Déclenchement automatique 
            qDebug() << "⚠️ ALERTE QUOTA : Dépassement de la limite mensuelle (5000kg)!";
            qDebug() << "Simulation d'envoi d'email à l'administration portuaire...";
        }
    }

    query.prepare("INSERT INTO PECHES (IDLOT, REFERENCE, ESPECE, QUANTITE, DATECAPTURE, IDBATEAU, IDFRIGO) "
                  "VALUES (:id, :ref, :esp, :qte, :date, :idb, :idf)");

    int idNum = idLot.startsWith("LOT") ? idLot.mid(3).toInt() : idLot.toInt();
    query.bindValue(":id", idNum);
    query.bindValue(":ref", reference);
    query.bindValue(":esp", espece);
    query.bindValue(":qte", quantiteKg.toDouble());
    
    QDate qdate = QDate::fromString(dateCapture, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(dateCapture, Qt::ISODate);
    query.bindValue(":date", qdate);

    query.bindValue(":idb", idBateau);
    query.bindValue(":idf", idFrigo);

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

QSqlQueryModel* Peche::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT 'LOT' || LPAD(p.IDLOT, 3, '0') as ID, p.REFERENCE as \"Référence\", "
                    "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
                    "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
                    "p.IDBATEAU, p.IDFRIGO "
                    "FROM PECHES p "
                    "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                    "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO");
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Référence"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Bateau"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Frigo"));

    return model;
}

bool Peche::supprimer(QString id) {
    QSqlQuery query;
    query.prepare("DELETE FROM PECHES WHERE IDLOT = :id");
    int idNum = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

bool Peche::modifier(QString id) {
    // Validation de base
    if (reference.trimmed().isEmpty()) {
        lastError = "La référence ne peut pas être vide.";
        return false;
    }

    // Validation du format (REF-YYYY-NOMBRE)
    QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
    if (!refRegex.match(reference).hasMatch()) {
        lastError = "Format de référence invalide (Attendu: REF-YYYY-NOMBRE).";
        return false;
    }

    // [NOUVEAU] Contrôle d'unicité (en excluant le lot actuel)
    int idNumForCheck = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();
    if (referenceExiste(reference, idNumForCheck)) {
        lastError = "Cette référence est déjà utilisée par un autre lot.";
        return false;
    }

    bool ok;
    double val = quantiteKg.toDouble(&ok);
    if (!ok || val <= 0) {
        lastError = "La quantité doit être un nombre positif.";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE PECHES SET reference = :ref, espece = :esp, quantite = :qte, datecapture = :date, "
                  "idbateau = :idb, idfrigo = :idf "
                  "WHERE IDLOT = :id");

    int idNum = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    query.bindValue(":ref", reference);
    query.bindValue(":esp", espece);
    query.bindValue(":qte", quantiteKg.toDouble());
    
    QDate qdate = QDate::fromString(dateCapture, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(dateCapture, Qt::ISODate);
    query.bindValue(":date", qdate);

    query.bindValue(":idb", idBateau);
    query.bindValue(":idf", idFrigo);

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    query.finish();
    return true;
}

QSqlQueryModel* Peche::trier(QString critere, QString ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString dbCritere = critere;
    
    if (critere == "ID") dbCritere = "p.IDLOT";
    else if (critere == "Référence") dbCritere = "p.REFERENCE";
    else if (critere == "Espèce" || critere == "Catégorie") dbCritere = "p.ESPECE";
    else if (critere == "Quantité") dbCritere = "p.QUANTITE";
    else if (critere == "Date") dbCritere = "p.DATECAPTURE";

    QString queryString = QString("SELECT 'LOT' || LPAD(p.IDLOT, 3, '0') as ID, p.REFERENCE as \"Référence\", "
                                   "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
                                   "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
                                   "p.IDBATEAU, p.IDFRIGO "
                                   "FROM PECHES p "
                                   "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                                   "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO "
                                   "ORDER BY %1 %2").arg(dbCritere, ordre);
    model->setQuery(queryString);
    return model;
}

QSqlQueryModel* Peche::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    
    // Recherche étendue : référence, espèce, quantité, bateau ou frigo
    query.prepare("SELECT 'LOT' || LPAD(p.IDLOT, 3, '0') as ID, p.REFERENCE as \"Référence\", "
                  "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
                  "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
                  "p.IDBATEAU, p.IDFRIGO "
                  "FROM PECHES p "
                  "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                  "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO "
                  "WHERE LOWER(p.REFERENCE) LIKE LOWER(:val) "
                  "OR LOWER(p.ESPECE) LIKE LOWER(:val) "
                  "OR LOWER(b.NOMBATEAU) LIKE LOWER(:val) "
                  "OR LOWER(f.REFERENCE) LIKE LOWER(:val) "
                  "OR TO_CHAR(p.QUANTITE) LIKE :val");
                  
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

bool Peche::referenceExiste(QString ref, int idLotExclu) {
    QSqlQuery query;
    if (idLotExclu == -1) {
        query.prepare("SELECT COUNT(*) FROM PECHES WHERE REFERENCE = :ref");
    } else {
        query.prepare("SELECT COUNT(*) FROM PECHES WHERE REFERENCE = :ref AND IDLOT != :id");
        query.bindValue(":id", idLotExclu);
    }
    query.bindValue(":ref", ref);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
