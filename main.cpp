#include "pechewindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("PortFlow");
    app.setOrganizationName("PortFlow");
    app.setApplicationDisplayName("PortFlow - Gestion des Pêches");

    // Set application style
    app.setStyle("Fusion");

    // Create and show peche window
    PecheWindow pecheWindow;
    pecheWindow.show();

    return app.exec();
}
