#include "frigo.h"
#include <QDebug>

FrigoModel::FrigoModel() : idFrigo(""), capacite(0), temperature(0), occupation(0) {}

FrigoModel::FrigoModel(QString id, QString ref, double cap, QString type, QString stat, QString dateRes, double temp, double occ)
    : idFrigo(id), reference(ref), capacite(cap), typePoisson(type), statut(stat), dateReservation(dateRes), temperature(temp), occupation(occ) {}

bool FrigoModel::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO FRIGOS (IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION) "
                  "VALUES (:id, :ref, :cap, :type, :stat, TO_DATE(:dateRes, 'DD/MM/YYYY'), :temp, :occ)");
    query.bindValue(":id", idFrigo);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":type", typePoisson);
    query.bindValue(":stat", statut);
    query.bindValue(":dateRes", dateReservation);
    query.bindValue(":temp", temperature);
    query.bindValue(":occ", occupation);

    if (!query.exec()) {
        query.finish(); // Libérer la connexion
        query.clear();
        QSqlQuery fb;
        fb.prepare("INSERT INTO FRIGOS (IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION) "
                   "VALUES (:id, :ref, :cap, :type, :stat, TO_DATE(:dateRes, 'DD/MM/YYYY'), :temp, :occ)");
        fb.bindValue(":id", idFrigo);
        fb.bindValue(":ref", reference);
        fb.bindValue(":cap", capacite);
        fb.bindValue(":type", typePoisson);
        fb.bindValue(":stat", statut);
        fb.bindValue(":dateRes", dateReservation);
        fb.bindValue(":temp", temperature);
        fb.bindValue(":occ", occupation);
        if (!fb.exec()) {
            m_lastError = query.lastError().text() + " | " + fb.lastError().text();
            qDebug() << "Erreur ajout frigo:" << m_lastError;
            return false;
        }
    }
    m_lastError.clear();
    return true;
}

QSqlQueryModel* FrigoModel::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION FROM FRIGOS");
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Référence");
    model->setHeaderData(2, Qt::Horizontal, "Capacité");
    model->setHeaderData(3, Qt::Horizontal, "Type Poisson");
    model->setHeaderData(4, Qt::Horizontal, "Statut");
    model->setHeaderData(5, Qt::Horizontal, "Date Réservation");
    model->setHeaderData(6, Qt::Horizontal, "Température");
    model->setHeaderData(7, Qt::Horizontal, "Occupation");
    return model;
}

bool FrigoModel::supprimer(QString id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FRIGOS WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool FrigoModel::modifier(QString id)
{
    QSqlQuery query;
    query.prepare("UPDATE FRIGOS SET REFERENCE=:ref, CAPACITE=:cap, TYPE_POISSON=:type, "
                  "STATUT=:stat, DATE_RESERVATION=TO_DATE(:dateRes, 'DD/MM/YYYY'), TEMPERATURE=:temp, OCCUPATION=:occ "
                  "WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":type", typePoisson);
    query.bindValue(":stat", statut);
    query.bindValue(":dateRes", dateReservation);
    query.bindValue(":temp", temperature);
    query.bindValue(":occ", occupation);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* FrigoModel::rechercher(QString val)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION FROM FRIGOS WHERE REFERENCE LIKE :v OR STATUT LIKE :v OR TYPE_POISSON LIKE :v");
    query.bindValue(":v", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel* FrigoModel::trier(QString critere, QString ordre)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString qStr = QString("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION FROM FRIGOS ORDER BY %1 %2").arg(critere, ordre);
    model->setQuery(qStr);
    return model;
}
