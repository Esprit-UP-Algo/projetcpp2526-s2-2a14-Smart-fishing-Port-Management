#include "connection.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{bool test=false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("source_projet2A");//inserer le nom de la source de données
    db.setUserName("eya");//inserer nom de l'utilisateur
    db.setPassword("zayati");//inserer mot de passe de cet utilisateur

    if (db.open())
        test=true;





    return  test;
}
