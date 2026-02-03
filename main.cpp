#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Load CSS stylesheet
    QFile styleFile(":/style.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        qDebug() << "CSS styles loaded successfully";
    } else {
        qDebug() << "Could not load CSS file, using default styling";
    }

    MainWindow window;
    window.show();

    return app.exec();
}
