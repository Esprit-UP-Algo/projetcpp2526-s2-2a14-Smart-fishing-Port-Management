#include "bateau.h"
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QVariant>

QString Bateau::lastError = "";

Bateau::Bateau() : etat("Au port") {}

Bateau::Bateau(QString id, QString nom, QString imm, QString cap, QString lon, QString age, QString date, QString idE, QString idQ, QString etatC, int codeSec)
    : idBateau(id), nomBateau(nom), immatriculation(imm), capacite(cap), longueur(lon), 
      ageBateau(age), dateMaintenance(date), idEmploye(idE), idQuai(idQ), etat(etatC), codeSecret(codeSec) {}

bool Bateau::ajouter() {
    // Validation du format (TN-YYYY-NUMERO)
    QRegularExpression immatRegex("^TN-\\d{4}-\\d+$");
    if (!immatRegex.match(immatriculation).hasMatch()) {
        lastError = "Format d'immatriculation invalide (Attendu: TN-YYYY-NUMERO).";
        return false;
    }

    // [NOUVEAU] Unicité
    if (immatriculationExiste(immatriculation)) {
        lastError = "Cette immatriculation appartient déjà à un autre bateau.";
        return false;
    }
    if (codeSecretExiste(codeSecret)) {
        lastError = "Ce code secret est déjà utilisé par un autre bateau.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        lastError = "Impossible de démarrer la transaction.";
        return false;
    }

    QSqlQuery query;
    if (idQuai.isEmpty()) {
        query.prepare("INSERT INTO BATEAUX (IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, ID_EMPLOYE, IDQUAI, ETAT, CODE_SECRET) "
                      "VALUES (:id, :nom, :imm, :cap, :lon, :age, TO_DATE(:date, 'DD/MM/YYYY'), :idE, NULL, :etat, :codeSec)");
    } else {
        query.prepare("INSERT INTO BATEAUX (IDBATEAU, NOMBATEAU, IMMATRICULATION, CAPACITE, LONGEUR, AGE_BATEAU, DATE_DERNIERE_MAINTENANCE, ID_EMPLOYE, IDQUAI, ETAT, CODE_SECRET) "
                      "VALUES (:id, :nom, :imm, :cap, :lon, :age, TO_DATE(:date, 'DD/MM/YYYY'), :idE, :idQ, :etat, :codeSec)");
    }
    
    query.bindValue(":id", idBateau.toInt());
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite.isEmpty() ? QVariant(QMetaType(QMetaType::Double)) : capacite.toDouble());
    query.bindValue(":lon", longueur.isEmpty() ? QVariant(QMetaType(QMetaType::Double)) : longueur.toDouble());
    query.bindValue(":age", ageBateau.isEmpty() ? QVariant(QMetaType(QMetaType::Int)) : ageBateau.toInt());
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":idE", idEmploye.isEmpty() ? QVariant(QMetaType(QMetaType::Int)) : idEmploye.toInt());
    if (!idQuai.isEmpty()) {
        query.bindValue(":idQ", idQuai.toInt());
    }
    query.bindValue(":etat", etat);
    query.bindValue(":codeSec", codeSecret);

    if (query.exec()) {
        // [NOUVEAU] Occuper le quai si le bateau est au port
        if (etat.trimmed().compare("Au port", Qt::CaseInsensitive) == 0 && !idQuai.isEmpty()) {
            QSqlQuery occQ;
            occQ.prepare("UPDATE QUAIS SET ETAT = 'Occupé' WHERE IDQUAI = :idq");
            occQ.bindValue(":idq", idQuai.toInt());
            if (!occQ.exec()) {
                db.rollback();
                lastError = "Erreur lors de la mise à jour du quai : " + occQ.lastError().text();
                return false;
            }
        }
        if (db.commit()) return true;
        lastError = "Erreur lors du commit : " + db.lastError().text();
        db.rollback();
        return false;
    }

    db.rollback();
    lastError = query.lastError().text();
    return false;
}

QSqlQueryModel* Bateau::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    // Index 0:ID, 1:Nom, 2:Immat, 3:Cap, 4:Lon, 5:Age, 6:Date, 7:EmpName, 8:QuaiLoc, 9:EmpID, 10:QuaiID, 11:Etat (String)
    model->setQuery("SELECT b.IDBATEAU, b.NOMBATEAU, b.IMMATRICULATION, b.CAPACITE, b.LONGEUR, b.AGE_BATEAU, "
                    "TO_CHAR(b.DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY'), e.NOM, "
                    "CASE "
                    "WHEN q.IDQUAI IS NULL THEN 'Aucun quai' "
                    "WHEN q.LOCATION IS NULL OR TRIM(q.LOCATION) = '' THEN 'Quai ' || q.NUMERO "
                    "ELSE 'Quai ' || q.NUMERO || ' (' || q.LOCATION || ')' "
                    "END, "
                    "b.ID_EMPLOYE, b.IDQUAI, b.ETAT, "
                    "TO_CHAR(b.DATE_PROCHAINE_MAINTENANCE, 'DD/MM/YYYY'), b.CODE_SECRET "
                    "FROM BATEAUX b "
                    "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                    "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI");
    return model;
}

bool Bateau::supprimer(QString id) {
    QSqlQuery query;
    
    // [NOUVEAU] Libérer le quai avant suppression
    QSqlQuery freeQ;
    freeQ.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE IDQUAI = (SELECT IDQUAI FROM BATEAUX WHERE IDBATEAU = :id)");
    freeQ.bindValue(":id", id.toInt());
    freeQ.exec();

    query.prepare("DELETE FROM BATEAUX WHERE IDBATEAU = :id");
    query.bindValue(":id", id);
    if (query.exec()) return true;
    lastError = query.lastError().text();
    return false;
}

bool Bateau::modifier(QString id) {
    // Validation du format (TN-YYYY-NUMERO)
    QRegularExpression immatRegex("^TN-\\d{4}-\\d+$");
    if (!immatRegex.match(immatriculation).hasMatch()) {
        lastError = "Format d'immatriculation invalide (Attendu: TN-YYYY-NUMERO).";
        return false;
    }

    // [NOUVEAU] Unicité (en excluant le bateau actuel)
    if (immatriculationExiste(immatriculation, id.toInt())) {
        lastError = "Cette immatriculation appartient déjà à un autre bateau.";
        return false;
    }
    if (codeSecretExiste(codeSecret, id.toInt())) {
        lastError = "Ce code secret est déjà utilisé par un autre bateau.";
        return false;
    }

    // Vérifier si le bateau passe "En mer" pour la première fois (ou de nouveau) pour incrémenter le compteur
    QSqlQuery oldQuery;
    oldQuery.prepare("SELECT ETAT FROM BATEAUX WHERE IDBATEAU = :id");
    oldQuery.bindValue(":id", id.toInt());
    QString etatActuel = "";
    if (oldQuery.exec() && oldQuery.next()) {
        etatActuel = oldQuery.value(0).toString();
    }

    if (etatActuel != "En mer" && etat == "En mer") {
        QSqlQuery incQuery;
        incQuery.prepare("UPDATE BATEAUX SET FREQUENCE_SORTIES = COALESCE(FREQUENCE_SORTIES, 0) + 1 WHERE IDBATEAU = :id");
        incQuery.bindValue(":id", id.toInt());
        incQuery.exec();
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        lastError = "Impossible de démarrer la transaction.";
        return false;
    }

    // [NOUVEAU] Gérer l'état du quai
    QSqlQuery oldQ;
    oldQ.prepare("SELECT IDQUAI FROM BATEAUX WHERE IDBATEAU = :id");
    oldQ.bindValue(":id", id.toInt());
    QString oldIdQuai = "";
    if (oldQ.exec() && oldQ.next()) {
        oldIdQuai = oldQ.value(0).toString();
    }

    if (etat.trimmed().compare("Au port", Qt::CaseInsensitive) != 0) {
        // Le bateau n'est plus au port : libérer l'ancien quai s'il existait
        if (!oldIdQuai.isEmpty() && oldIdQuai != "0") {
            QSqlQuery freeQ;
            freeQ.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE IDQUAI = :idq");
            freeQ.bindValue(":idq", oldIdQuai.toInt());
            if (!freeQ.exec()) {
                db.rollback();
                lastError = "Erreur lors de la libération du quai : " + freeQ.lastError().text();
                return false;
            }
        }
        idQuai = ""; 
    } else {
        // Le bateau est au port :
        // 1. Si le quai a changé, libérer l'ancien
        if (!oldIdQuai.isEmpty() && oldIdQuai != "0" && oldIdQuai != idQuai) {
            QSqlQuery freeQ;
            freeQ.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE IDQUAI = :idq");
            freeQ.bindValue(":idq", oldIdQuai.toInt());
            if (!freeQ.exec()) {
                db.rollback();
                lastError = "Erreur lors de la libération de l'ancien quai : " + freeQ.lastError().text();
                return false;
            }
        }
        // 2. Occuper le nouveau quai
        if (!idQuai.isEmpty() && idQuai != "0") {
            QSqlQuery occQ;
            occQ.prepare("UPDATE QUAIS SET ETAT = 'Occupé' WHERE IDQUAI = :idq");
            occQ.bindValue(":idq", idQuai.toInt());
            if (!occQ.exec()) {
                db.rollback();
                lastError = "Erreur lors de l'occupation du nouveau quai : " + occQ.lastError().text();
                return false;
            }
        }
    }

    QSqlQuery query;
    if (idQuai.isEmpty() || idQuai == "0") {
        query.prepare("UPDATE BATEAUX SET NOMBATEAU=:nom, IMMATRICULATION=:imm, CAPACITE=:cap, LONGEUR=:lon, "
                      "AGE_BATEAU=:age, DATE_DERNIERE_MAINTENANCE=TO_DATE(:date, 'DD/MM/YYYY'), ID_EMPLOYE=:idE, IDQUAI=NULL, ETAT=:etat, CODE_SECRET=:codeSec "
                      "WHERE IDBATEAU=:id");
    } else {
        query.prepare("UPDATE BATEAUX SET NOMBATEAU=:nom, IMMATRICULATION=:imm, CAPACITE=:cap, LONGEUR=:lon, "
                      "AGE_BATEAU=:age, DATE_DERNIERE_MAINTENANCE=TO_DATE(:date, 'DD/MM/YYYY'), ID_EMPLOYE=:idE, IDQUAI=:idQ, ETAT=:etat, CODE_SECRET=:codeSec "
                      "WHERE IDBATEAU=:id");
    }
    
    query.bindValue(":nom", nomBateau);
    query.bindValue(":imm", immatriculation);
    query.bindValue(":cap", capacite.isEmpty() ? QVariant(QMetaType(QMetaType::Double)) : capacite.toDouble());
    query.bindValue(":lon", longueur.isEmpty() ? QVariant(QMetaType(QMetaType::Double)) : longueur.toDouble());
    query.bindValue(":age", ageBateau.isEmpty() ? QVariant(QMetaType(QMetaType::Int)) : ageBateau.toInt());
    query.bindValue(":date", dateMaintenance);
    query.bindValue(":idE", idEmploye.isEmpty() ? QVariant(QMetaType(QMetaType::Int)) : idEmploye.toInt());
    if (!idQuai.isEmpty()) {
        query.bindValue(":idQ", idQuai.toInt());
    }
    query.bindValue(":etat", etat);
    query.bindValue(":codeSec", codeSecret);
    query.bindValue(":id", id.toInt());

    if (query.exec()) {
        if (db.commit()) return true;
        lastError = "Erreur lors du commit : " + db.lastError().text();
        db.rollback();
        return false;
    }

    db.rollback();
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
                                  "b.ID_EMPLOYE, b.IDQUAI, b.ETAT, "
                                  "TO_CHAR(b.DATE_PROCHAINE_MAINTENANCE, 'DD/MM/YYYY'), b.CODE_SECRET "
                                  "FROM BATEAUX b "
                                  "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                                  "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI "
                                  "ORDER BY %1 %2").arg(realCritere, ordre);
    QSqlQuery query;
    query.prepare(queryString);
    query.exec();
    model->setQuery(std::move(query));
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
                  "b.ID_EMPLOYE, b.IDQUAI, b.ETAT, "
                  "TO_CHAR(b.DATE_PROCHAINE_MAINTENANCE, 'DD/MM/YYYY'), b.CODE_SECRET "
                  "FROM BATEAUX b "
                  "LEFT JOIN EMPLOYEES e ON b.ID_EMPLOYE = e.ID_EMPLOYE "
                  "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI "
                  "WHERE b.NOMBATEAU LIKE :val OR b.IMMATRICULATION LIKE :val");
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

bool Bateau::immatriculationExiste(QString imm, int idBateauExclu) {
    QSqlQuery query;
    if (idBateauExclu == -1) {
        query.prepare("SELECT COUNT(*) FROM BATEAUX WHERE IMMATRICULATION = :imm");
    } else {
        query.prepare("SELECT COUNT(*) FROM BATEAUX WHERE IMMATRICULATION = :imm AND IDBATEAU != :id");
        query.bindValue(":id", idBateauExclu);
    }
    query.bindValue(":imm", imm);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool Bateau::codeSecretExiste(int code, int idBateauExclu) {
    QSqlQuery query;
    if (idBateauExclu == -1) {
        query.prepare("SELECT COUNT(*) FROM BATEAUX WHERE CODE_SECRET = :code");
    } else {
        query.prepare("SELECT COUNT(*) FROM BATEAUX WHERE CODE_SECRET = :code AND IDBATEAU != :id");
        query.bindValue(":id", idBateauExclu);
    }
    query.bindValue(":code", code);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QStringList Bateau::getUpcomingMaintenanceAlerts() {
    QStringList alerts;
    QSqlQuery query;
    // Sélectionne les bateaux dont la date de prochaine maintenance est aujourd'hui ou demain (J-1)
    query.prepare("SELECT NOMBATEAU, TO_CHAR(DATE_PROCHAINE_MAINTENANCE, 'DD/MM/YYYY') FROM BATEAUX "
                  "WHERE DATE_PROCHAINE_MAINTENANCE BETWEEN CURRENT_DATE AND (CURRENT_DATE + 1)");
    
    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            QString date = query.value(1).toString();
            alerts << QString("%1 (%2)").arg(name, date);
        }
    } else {
        qDebug() << "Erreur lors de la récupération des alertes de maintenance:" << query.lastError().text();
    }
    return alerts;
}
