#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connection
{
private:
    Connection(); // Private constructor
    static Connection* instance;

public:
    static Connection& getInstance();
    bool createconnect();
};

#endif // CONNECTION_H
