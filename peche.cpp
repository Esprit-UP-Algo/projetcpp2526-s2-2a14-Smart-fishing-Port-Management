#include "peche.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QDate>

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
    QSqlQuery query;
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
        qDebug() << "Erreur lors de l'ajout de la peche:" << lastError;
        return false;
    }
    query.finish();
    return true;
}

QSqlQueryModel* Peche::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT 'LOT' || LPAD(IDLOT, 3, '0') as ID, reference as \"Référence\", "
                    "espece as \"Espèce\", quantite as \"Quantité\", datecapture as \"Date\", "
                    "idbateau as \"Bateau\", idfrigo as \"Frigo\" FROM PECHES");
    
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
    if (critere == "ID") dbCritere = "IDLOT";
    else if (critere == "Référence") dbCritere = "reference";
    else if (critere == "Espèce") dbCritere = "espece";
    else if (critere == "Quantité") dbCritere = "quantite";
    else if (critere == "Date") dbCritere = "datecapture";

    QString queryString = QString("SELECT 'LOT' || LPAD(IDLOT, 3, '0') as ID, reference as \"Référence\", "
                                  "espece as \"Espèce\", quantite as \"Quantité\", datecapture as \"Date\", "
                                  "idbateau as \"Bateau\", idfrigo as \"Frigo\" "
                                  "FROM PECHES ORDER BY %1 %2").arg(dbCritere, ordre);
    model->setQuery(queryString);
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Référence"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Bateau"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Frigo"));

    return model;
}

QSqlQueryModel* Peche::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT 'LOT' || LPAD(IDLOT, 3, '0') as ID, reference as \"Référence\", "
                  "espece as \"Espèce\", quantite as \"Quantité\", datecapture as \"Date\", "
                  "idbateau as \"Bateau\", idfrigo as \"Frigo\" "
                  "FROM PECHES WHERE reference LIKE :val OR espece LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Référence"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Bateau"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Frigo"));

    return model;
}
