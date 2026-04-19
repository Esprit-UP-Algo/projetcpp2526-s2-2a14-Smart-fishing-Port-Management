#include <QApplication>
#include "bateauwindow.h"
#include "loginwindow.h"
#include "connection.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Propriétés de l'application (doivent être définies tôt pour les notifications)
    app.setApplicationName("PortFlow");
    app.setApplicationDisplayName("PortFlow");
    app.setOrganizationName("PortFlow");
    app.setStyle("Fusion");

    // Initialisation de la connexion à la base de données
    bool test = Connection::getInstance().createconnect();

    if (test) {
        qDebug() << "Connexion à la base de données réussie !";
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                              QObject::tr("Impossible de se connecter à la base de données.\n"
                                          "Vérifiez votre configuration ODBC."), QMessageBox::Cancel);
    }

    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();


    return app.exec();
}
