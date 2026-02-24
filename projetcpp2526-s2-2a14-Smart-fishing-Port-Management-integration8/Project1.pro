QT       += core gui sql printsupport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    loginwindow.cpp \
    Bateauwindow.cpp \
    Bateaudialog.cpp \
    Pechewindow.cpp \
    Pechedialog.cpp \
    Employeewindow.cpp \
    Employeedialog.cpp \
    Frigowindow.cpp \
    addfrigodialog.cpp \
    StatisticsDialog.cpp \
    LivraisonStatisticsDialog.cpp \
    Livraisonwindow.cpp \
    AddLivraisonDialog.cpp \
    ../livraison.cpp

HEADERS += \
    connection.h \
    mainwindow.h \
    loginwindow.h \
    Bateauwindow.h \
    Bateaudialog.h \
    Pechewindow.h \
    Pechedialog.h \
    Employeewindow.h \
    Employeedialog.h \
    Frigowindow.h \
    addfrigodialog.h \
    StatisticsDialog.h \
    LivraisonStatisticsDialog.h \
    Livraisonwindow.h \
    AddLivraisonDialog.h \
    ../livraison.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += resources.qrc
