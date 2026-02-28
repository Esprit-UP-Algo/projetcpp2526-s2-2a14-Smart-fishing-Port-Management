#include "bateau.h"
#include <QSqlError>
#include <QDebug>

QString Bateau::lastError = "";

Bateau::Bateau() {}

Bateau::Bateau(QString id, QString nom, QString imm, QString cap, QString lon, QString age, QString date, QString disp, QString idE, QString idQ)
    : idBateau(id), nomBateau(nom), immatriculation(imm), capacite(cap), longueur(lon), 
      ageBateau(age), dateMaintenance(date), disponible(disp), idEmploye(idE), idQuai(idQ) {}

bool Bateau::ajouter() {
    QSqlQuery query;
    // Architecture Modèle-Vue + Sécurité (prepare/bindValue)
    query.prepare("INSERT INTO BATEAUX (IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, DISPONIBLE, ID_EMPLOYE, IDQUAI) "
                  "VALUES (:id, :nom, :imm, :cap, :lon, :age, :date, :disp, :idE, :idQ)");
    
    query.bindValue(":id", idBateau);
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite);
    query.bindValue(":lon", longueur);
    query.bindValue(":age", ageBateau);
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":disp", disponible);
    query.bindValue(":idE", idEmploye);
    query.bindValue(":idQ", idQuai);

    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

QSqlQueryModel* Bateau::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    // Utilisation de setHeaderData selon les standards du cours
    model->setQuery("SELECT IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, DISPONIBLE, ID_EMPLOYE, IDQUAI FROM BATEAUX");
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Immat."));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Longueur"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Age"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Maintenance"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Dispo"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Employé"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Quai"));

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
    query.prepare("UPDATE BATEAUX SET NOMBATEAU=:nom, IMMATRICULATION=:imm, CAPACITE=:cap, LONGEUR=:lon, "
                  "AGE_BATEAU=:age, DATE_DERNIERE_MAINTENANCE=:date, DISPONIBLE=:disp, ID_EMPLOYE=:idE, IDQUAI=:idQ "
                  "WHERE IDBATEAU=:id");
    
    query.bindValue(":id", id);
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite);
    query.bindValue(":lon", longueur);
    query.bindValue(":age", ageBateau);
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":disp", disponible);
    query.bindValue(":idE", idEmploye);
    query.bindValue(":idQ", idQuai);

    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

QSqlQueryModel* Bateau::trier(QString critere, QString ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString queryString = QString("SELECT IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, DISPONIBLE, ID_EMPLOYE, IDQUAI FROM BATEAUX ORDER BY %1 %2")
        .arg(critere, ordre);
    model->setQuery(queryString);
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Immat."));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Longueur"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Age"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Maintenance"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Dispo"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Employé"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Quai"));
    
    return model;
}

QSqlQueryModel* Bateau::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT * FROM BATEAUX WHERE NOMBATEAU LIKE :val OR IMMATRICULATION LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}
