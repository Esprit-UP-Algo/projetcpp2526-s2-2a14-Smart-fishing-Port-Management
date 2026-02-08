#include <QApplication>
#include "dockswindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setFont(QFont("Times New Roman", 10));

    DocksWindow w;
    w.show();

    return a.exec();
}
