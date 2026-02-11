#include <QApplication>
#include <QFont>
#include "loginwindow.h"
#include "dockswindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Global application configuration
    app.setApplicationName("PortFlow - Gestion Bateaux");
    app.setOrganizationName("PortFlow");
    app.setApplicationDisplayName("PortFlow - Gestion des Pêches");
    app.setStyle("Fusion");
    QApplication::setFont(QFont("Times New Roman", 10));

    // Choose which window to show first
    LoginWindow loginWindow;
    loginWindow.show();

    // If you want to show DocksWindow instead, comment the above two lines and uncomment below:
    // DocksWindow w;
    // w.show();

    return app.exec();
}
