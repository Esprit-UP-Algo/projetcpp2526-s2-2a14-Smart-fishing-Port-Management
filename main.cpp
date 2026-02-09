#include <QApplication>
#include "dockswindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setFont(QFont("Times New Roman", 10));

    DocksWindow w;
    w.show();

    return a.exec();

#include <QApplication>
#include "bateauwindow.h"

#include "loginwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    // Configuration de l'application
    app.setApplicationName("PortFlow - Gestion Bateaux");
    app.setOrganizationName("PortFlow");
    app.setStyle("Fusion");



    // Set application properties
    app.setApplicationName("PortFlow");
    app.setOrganizationName("PortFlow");
    app.setApplicationDisplayName("PortFlow - Gestion des Pêches");

    // Set application style
    app.setStyle("Fusion");

    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();


    return app.exec();
}
