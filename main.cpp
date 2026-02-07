#include <QApplication>
#include "bateauwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Configuration de l'application
    app.setApplicationName("PortFlow - Gestion Bateaux");
    app.setOrganizationName("PortFlow");
    app.setStyle("Fusion");

    // Lancer directement la fenêtre Bateaux
    BateauWindow bateauWindow;
    bateauWindow.show();

    return app.exec();
}
