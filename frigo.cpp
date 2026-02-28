#include "frigo.h"
#include <QDebug>

FrigoModel::FrigoModel() : idFrigo(0), capacite(0), temperature(0), occupation(0) {}

FrigoModel::FrigoModel(int id, QString ref, double cap, double temp, QString stat, QString type, double occ)
    : idFrigo(id), reference(ref), capacite(cap), temperature(temp), statut(stat), typePoisson(type), occupation(occ) {}

bool FrigoModel::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO FRIGOS (IDFRIGO, REFERENCE, CAPACITE, TEMPERATURE, STATUT, TYPEPOISSON, OCCUPATION) "
                  "VALUES (:id, :ref, :cap, :temp, :stat, :type, :occ)");
    query.bindValue(":id", idFrigo);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":temp", temperature);
    query.bindValue(":stat", statut);
    query.bindValue(":type", typePoisson);
    query.bindValue(":occ", occupation);

    if (!query.exec()) {
        qDebug() << "Erreur ajout frigo:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* FrigoModel::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT IDFRIGO, REFERENCE, CAPACITE, TEMPERATURE, STATUT, TYPEPOISSON, OCCUPATION FROM FRIGOS");
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Référence");
    model->setHeaderData(2, Qt::Horizontal, "Capacité");
    model->setHeaderData(3, Qt::Horizontal, "Température");
    model->setHeaderData(4, Qt::Horizontal, "Statut");
    model->setHeaderData(5, Qt::Horizontal, "Type Poisson");
    model->setHeaderData(6, Qt::Horizontal, "Occupation");
    return model;
}

bool FrigoModel::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FRIGOS WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool FrigoModel::modifier(int id)
{
    QSqlQuery query;
    query.prepare("UPDATE FRIGOS SET REFERENCE=:ref, CAPACITE=:cap, TEMPERATURE=:temp, "
                  "STATUT=:stat, TYPEPOISSON=:type, OCCUPATION=:occ "
                  "WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":temp", temperature);
    query.bindValue(":stat", statut);
    query.bindValue(":type", typePoisson);
    query.bindValue(":occ", occupation);
    return query.exec();
}

QSqlQueryModel* FrigoModel::rechercher(QString val)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT * FROM FRIGOS WHERE REFERENCE LIKE :v OR STATUT LIKE :v OR TYPEPOISSON LIKE :v");
    query.bindValue(":v", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel* FrigoModel::trier(QString critere, QString ordre)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString qStr = QString("SELECT * FROM FRIGOS ORDER BY %1 %2").arg(critere, ordre);
    model->setQuery(qStr);
    return model;
}
