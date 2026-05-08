#include "frigo.h"
#include <QDebug>
#include <QRegularExpression>

FrigoModel::FrigoModel() : idFrigo(""), capacite(0), temperature(0), occupation(0), telephone("") {}

FrigoModel::FrigoModel(QString id, QString ref, double cap, QString type, QString stat, QString dateRes, double temp, double occ, QString tel)
    : idFrigo(id), reference(ref), capacite(cap), typePoisson(type), statut(stat), dateReservation(dateRes), temperature(temp), occupation(occ), telephone(tel) {}

bool FrigoModel::ajouter()
{
    // Validation du format de référence (FRG-NUMERO)
    QRegularExpression frgRegex("^FRG-\\d+$");
    if (reference.trimmed().isEmpty()) {
        m_lastError = "La référence ne peut pas être vide.";
        return false;
    }
    if (!frgRegex.match(reference).hasMatch()) {
        m_lastError = "Format de référence invalide (Attendu: FRG-NUMERO, ex: FRG-001).";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO FRIGOS (IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE) "
                  "VALUES (:id, :ref, :cap, :type, :stat, TO_DATE(:dateRes, 'DD/MM/YYYY'), :temp, :occ, :tel)");
    query.bindValue(":id", idFrigo);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":type", typePoisson);
    query.bindValue(":stat", statut);
    query.bindValue(":dateRes", dateReservation);
    query.bindValue(":temp", temperature);
    query.bindValue(":occ", occupation);
    query.bindValue(":tel", telephone);

    if (!query.exec()) {
        query.finish(); // Libérer la connexion
        query.clear();
        QSqlQuery fb;
        fb.prepare("INSERT INTO FRIGOS (IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE) "
                   "VALUES (:id, :ref, :cap, :type, :stat, TO_DATE(:dateRes, 'DD/MM/YYYY'), :temp, :occ, :tel)");
        fb.bindValue(":id", idFrigo);
        fb.bindValue(":ref", reference);
        fb.bindValue(":cap", capacite);
        fb.bindValue(":type", typePoisson);
        fb.bindValue(":stat", statut);
        fb.bindValue(":dateRes", dateReservation);
        fb.bindValue(":temp", temperature);
        fb.bindValue(":occ", occupation);
        fb.bindValue(":tel", telephone);
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
    model->setQuery("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE FROM FRIGOS");
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Référence");
    model->setHeaderData(2, Qt::Horizontal, "Capacité");
    model->setHeaderData(3, Qt::Horizontal, "Type Poisson");
    model->setHeaderData(4, Qt::Horizontal, "Statut");
    model->setHeaderData(5, Qt::Horizontal, "Date Réservation");
    model->setHeaderData(6, Qt::Horizontal, "Température");
    model->setHeaderData(7, Qt::Horizontal, "Occupation");
    model->setHeaderData(8, Qt::Horizontal, "Téléphone");
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
    // Validation du format de référence (FRG-NUMERO)
    QRegularExpression frgRegex("^FRG-\\d+$");
    if (reference.trimmed().isEmpty()) {
        m_lastError = "La référence ne peut pas être vide.";
        return false;
    }
    if (!frgRegex.match(reference).hasMatch()) {
        m_lastError = "Format de référence invalide (Attendu: FRG-NUMERO, ex: FRG-001).";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE FRIGOS SET REFERENCE=:ref, CAPACITE=:cap, TYPE_POISSON=:type, "
                  "STATUT=:stat, DATE_RESERVATION=TO_DATE(:dateRes, 'DD/MM/YYYY'), TEMPERATURE=:temp, OCCUPATION=:occ, TELEPHONE=:tel "
                  "WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    query.bindValue(":ref", reference);
    query.bindValue(":cap", capacite);
    query.bindValue(":type", typePoisson);
    query.bindValue(":stat", statut);
    query.bindValue(":dateRes", dateReservation);
    query.bindValue(":temp", temperature);
    query.bindValue(":occ", occupation);
    query.bindValue(":tel", telephone);

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
    query.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE FROM FRIGOS WHERE REFERENCE LIKE :v OR STATUT LIKE :v OR TYPE_POISSON LIKE :v OR TELEPHONE LIKE :v");
    query.bindValue(":v", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel* FrigoModel::trier(QString critere, QString ordre)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString qStr = QString("SELECT IDFRIGO, REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE FROM FRIGOS ORDER BY %1 %2").arg(critere, ordre);
    model->setQuery(qStr);
    return model;
}
