#include "bateau.h"
#include <QSqlError>
#include <QDebug>

QString Bateau::lastError = "";

Bateau::Bateau() : etat("Au port") {}

Bateau::Bateau(QString id, QString nom, QString imm, QString cap, QString lon, QString age, QString date, QString idE, QString idQ, QString etatC)
    : idBateau(id), nomBateau(nom), immatriculation(imm), capacite(cap), longueur(lon), 
      ageBateau(age), dateMaintenance(date), idEmploye(idE), idQuai(idQ), etat(etatC) {}

bool Bateau::ajouter() {
    QSqlQuery query;
    if (idQuai.isEmpty()) {
        query.prepare("INSERT INTO BATEAUX (IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, ID_EMPLOYE, IDQUAI, ETAT) "
                      "VALUES (:id, :nom, :imm, :cap, :lon, :age, TO_DATE(:date, 'DD/MM/YYYY'), :idE, NULL, :etat)");
    } else {
        query.prepare("INSERT INTO BATEAUX (IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, ID_EMPLOYE, IDQUAI, ETAT) "
                      "VALUES (:id, :nom, :imm, :cap, :lon, :age, TO_DATE(:date, 'DD/MM/YYYY'), :idE, :idQ, :etat)");
    }
    
    query.bindValue(":id", idBateau.toInt());
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite.isEmpty() ? QVariant() : capacite.toDouble());
    query.bindValue(":lon", longueur.isEmpty() ? QVariant() : longueur.toDouble());
    query.bindValue(":age", ageBateau.isEmpty() ? QVariant() : ageBateau.toInt());
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":idE", idEmploye.isEmpty() ? QVariant() : idEmploye.toInt());
    if (!idQuai.isEmpty()) {
        query.bindValue(":idQ", idQuai.toInt());
    }
    query.bindValue(":etat", etat);

    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

QSqlQueryModel* Bateau::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    // Index 0:ID, 1:Nom, 2:Immat, 3:Cap, 4:Lon, 5:Age, 6:Date, 7:EmpName, 8:QuaiLabel, 9:EmpID, 10:QuaiID, 11:Etat (String)
    model->setQuery("SELECT b.IDBATEAU, b.NOMBATEAU, b.IMMATRICULATION, b.CAPACITE, b.LONGEUR, b.AGE_BATEAU, "
                    "TO_CHAR(b.DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY'), e.NOM, "
                    "CASE "
                    "WHEN q.IDQUAI IS NULL THEN 'Aucun quai' "
                    "WHEN q.LOCATION IS NULL OR TRIM(q.LOCATION) = '' THEN 'Quai ' || q.NUMERO "
                    "ELSE 'Quai ' || q.NUMERO || ' (' || q.LOCATION || ')' "
                    "END, "
                    "b.ID_EMPLOYE, b.IDQUAI, b.ETAT "
                    "FROM BATEAUX b "
                    "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                    "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI");
    return model;
}

bool Bateau::supprimer(QString id) {
    QSqlQuery query;
    query.prepare("DELETE FROM BATEAUX WHERE IDBATEAU = :id");
    query.bindValue(":id", id);
    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

bool Bateau::modifier(QString id) {
    QSqlQuery query;
    if (idQuai.isEmpty()) {
        query.prepare("UPDATE BATEAUX SET NOMBATEAU=:nom, IMMATRICULATION=:imm, CAPACITE=:cap, LONGEUR=:lon, "
                      "AGE_BATEAU=:age, DATE_DERNIERE_MAINTENANCE=TO_DATE(:date, 'DD/MM/YYYY'), ID_EMPLOYE=:idE, IDQUAI=NULL, ETAT=:etat "
                      "WHERE IDBATEAU=:id");
    } else {
        query.prepare("UPDATE BATEAUX SET NOMBATEAU=:nom, IMMATRICULATION=:imm, CAPACITE=:cap, LONGEUR=:lon, "
                      "AGE_BATEAU=:age, DATE_DERNIERE_MAINTENANCE=TO_DATE(:date, 'DD/MM/YYYY'), ID_EMPLOYE=:idE, IDQUAI=:idQ, ETAT=:etat "
                      "WHERE IDBATEAU=:id");
    }
    
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite.isEmpty() ? QVariant() : capacite.toDouble());
    query.bindValue(":lon", longueur.isEmpty() ? QVariant() : longueur.toDouble());
    query.bindValue(":age", ageBateau.isEmpty() ? QVariant() : ageBateau.toInt());
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":idE", idEmploye.isEmpty() ? QVariant() : idEmploye.toInt());
    if (!idQuai.isEmpty()) {
        query.bindValue(":idQ", idQuai.toInt());
    }
    query.bindValue(":etat", etat);
    query.bindValue(":id", id.toInt());

    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

QSqlQueryModel* Bateau::trier(QString critere, QString ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString realCritere = critere;
    if (critere == "IDBATEAU" || critere == "NOMBATEAU" || critere == "CAPACITE" || critere == "AGE_BATEAU")
        realCritere = "b." + critere;
    else if (critere == "DATE_DERNIERE_MAINTENANCE")
        realCritere = "b.DATE_DERNIERE_MAINTENANCE";

    QString queryString = QString("SELECT b.IDBATEAU, b.NOMBATEAU, b.IMMATRICULATION, b.CAPACITE, b.LONGEUR, b.AGE_BATEAU, "
                                  "TO_CHAR(b.DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY'), e.NOM, "
                                  "CASE "
                                  "WHEN q.IDQUAI IS NULL THEN 'Aucun quai' "
                                  "WHEN q.LOCATION IS NULL OR TRIM(q.LOCATION) = '' THEN 'Quai ' || q.NUMERO "
                                  "ELSE 'Quai ' || q.NUMERO || ' (' || q.LOCATION || ')' "
                                  "END, "
                                  "b.ID_EMPLOYE, b.IDQUAI, b.ETAT "
                                  "FROM BATEAUX b "
                                  "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                                  "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI "
                                  "ORDER BY %1 %2")
        .arg(realCritere, ordre);
    model->setQuery(queryString);
    return model;
}

QSqlQueryModel* Bateau::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT b.IDBATEAU, b.NOMBATEAU, b.IMMATRICULATION, b.CAPACITE, b.LONGEUR, b.AGE_BATEAU, "
                  "TO_CHAR(b.DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY'), e.NOM, "
                  "CASE "
                  "WHEN q.IDQUAI IS NULL THEN 'Aucun quai' "
                  "WHEN q.LOCATION IS NULL OR TRIM(q.LOCATION) = '' THEN 'Quai ' || q.NUMERO "
                  "ELSE 'Quai ' || q.NUMERO || ' (' || q.LOCATION || ')' "
                  "END, "
                  "b.ID_EMPLOYE, b.IDQUAI, b.ETAT "
                  "FROM BATEAUX b "
                  "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                  "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI "
                  "WHERE b.NOMBATEAU LIKE :val OR b.IMMATRICULATION LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}
