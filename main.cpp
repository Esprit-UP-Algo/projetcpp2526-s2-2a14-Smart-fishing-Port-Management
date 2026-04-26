#include <QApplication>
#include "bateauwindow.h"
#include "loginwindow.h"
#include "connection.h"
#include "quaimanager.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialisation de la connexion à la base de données
    bool test = Connection::getInstance().createconnect();

    if (test) {
        qDebug() << "Connexion à la base de données réussie !";
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                              QObject::tr("Impossible de se connecter à la base de données.\n"
                                          "Vérifiez votre configuration ODBC."), QMessageBox::Cancel);
        // On peut choisir de continuer ou de quitter ici
        // return -1;
    }


    // Configuration de l'application
    app.setApplicationName("PortFlow - Gestion Bateaux");
    app.setOrganizationName("PortFlow");
    app.setStyle("Fusion");



    // Set application properties
    app.setApplicationName("PortFlow");
    app.setOrganizationName("PortFlow");
    app.setApplicationDisplayName("PortFlow");

    // Set application style
    app.setStyle("Fusion");

    // Create and show login window
    QuaiManager quaiManager;
    quaiManager.start();

    LoginWindow loginWindow;
    loginWindow.show();


    return app.exec();
}
