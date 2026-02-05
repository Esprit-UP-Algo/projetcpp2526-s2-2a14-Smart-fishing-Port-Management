#include "loginwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("PortFlow");
    app.setOrganizationName("PortFlow");
    app.setApplicationDisplayName("PortFlow - Smart Fishing Port Management");

    // Set application style
    app.setStyle("Fusion");

    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
