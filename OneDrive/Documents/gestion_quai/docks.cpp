#include "dockswindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    docksWindow w;
    w.show();
    return a.exec();
}
