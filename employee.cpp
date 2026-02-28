#include "employee.h"
#include <QDebug>

EmployeeModel::EmployeeModel() : salaire(0) {}

EmployeeModel::EmployeeModel(QString id, QString c, double s, QDate d, QString st, QString pos, QString pre, QString n)
    : id_employe(id), cin(c), salaire(s), datederecrutement(d), statut(st), position(pos), prenom(pre), nom(n) {}

bool EmployeeModel::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYEES (ID_EMPLOYE, CIN, SALAIRE, DATE_RECRUTEMENT, STATUT, \"POSITION\", PRENOM, NOM) "
                  "VALUES (:id, :cin, :sal, :date, :stat, :pos, :pre, :nom)");
    query.bindValue(":id", id_employe);
    query.bindValue(":cin", cin);
    query.bindValue(":sal", salaire);
    query.bindValue(":date", datederecrutement.toString("yyyy-MM-dd"));
    query.bindValue(":stat", statut);
    query.bindValue(":pos", position);
    query.bindValue(":pre", prenom);
    query.bindValue(":nom", nom);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur ajout employé:" << m_lastError;
        return false;
    }
    m_lastError.clear();
    return true;
}

QSqlQueryModel* EmployeeModel::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLOYE, CIN, SALAIRE, DATE_RECRUTEMENT, STATUT, \"POSITION\", PRENOM, NOM FROM EMPLOYEES");
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "CIN");
    model->setHeaderData(2, Qt::Horizontal, "Salaire");
    model->setHeaderData(3, Qt::Horizontal, "Date Recrutement");
    model->setHeaderData(4, Qt::Horizontal, "Statut");
    model->setHeaderData(5, Qt::Horizontal, "Position");
    model->setHeaderData(6, Qt::Horizontal, "Prénom");
    model->setHeaderData(7, Qt::Horizontal, "Nom");
    return model;
}

bool EmployeeModel::supprimer(QString id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool EmployeeModel::modifier(QString id)
{
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYEES SET CIN=:cin, SALAIRE=:sal, DATE_RECRUTEMENT=:date, "
                  "STATUT=:stat, \"POSITION\"=:pos, PRENOM=:pre, NOM=:nom "
                  "WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", id);
    query.bindValue(":cin", cin);
    query.bindValue(":sal", salaire);
    query.bindValue(":date", datederecrutement.toString("yyyy-MM-dd"));
    query.bindValue(":stat", statut);
    query.bindValue(":pos", position);
    query.bindValue(":pre", prenom);
    query.bindValue(":nom", nom);
    return query.exec();
}

QSqlQueryModel* EmployeeModel::rechercher(QString val)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEES WHERE NOM LIKE :v OR PRENOM LIKE :v OR \"POSITION\" LIKE :v OR CIN LIKE :v");
    query.bindValue(":v", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel* EmployeeModel::trier(QString critere, QString ordre)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString qStr = QString("SELECT * FROM EMPLOYEES ORDER BY %1 %2").arg(critere, ordre);
    model->setQuery(qStr);
    return model;
}
