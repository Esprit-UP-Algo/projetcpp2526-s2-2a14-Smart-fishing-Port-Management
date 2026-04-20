#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDate>

class EmployeeModel
{
public:
    EmployeeModel();
    EmployeeModel(QString id, QString cin, double salaire, QDate date, QString statut, QString position, QString prenom, QString nom, QString telephone="", QString email="");

    // Getters
    QString getId() const { return id_employe; }
    QString getCin() const { return cin; }
    double getSalaire() const { return salaire; }
    QDate getDate() const { return datederecrutement; }
    QString getStatut() const { return statut; }
    QString getPosition() const { return position; }
    QString getPrenom() const { return prenom; }
    QString getNom() const { return nom; }
    QString getTelephone() const { return telephone; }
    QString getEmail() const { return email; }

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
    QSqlQueryModel* rechercher(QString val);
    QSqlQueryModel* trier(QString critere, QString ordre);
    QString getLastError() const { return m_lastError; }

private:
    QString id_employe;
    QString cin;
    double salaire;
    QDate datederecrutement;
    QString statut;
    QString position;
    QString prenom;
    QString nom;
    QString telephone;
    QString email;
    QString m_lastError;
};

#endif // EMPLOYEE_H
