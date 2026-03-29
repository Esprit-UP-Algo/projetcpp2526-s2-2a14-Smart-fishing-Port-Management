QT       += core gui sql printsupport charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    connection.cpp \
    contractgenerator.cpp \
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
    Livraisonwindow.cpp \
    AddLivraisonDialog.cpp \
    quaiswindow.cpp \
    addquaidialog.cpp \
    quai.cpp \
    StatisticsDialog.cpp \
    LivraisonStatisticsDialog.cpp \
    FrigoStatisticsDialog.cpp \
    EmployeeStatsWindow.cpp \
    livraison.cpp \
    peche.cpp \
    PecheStatisticsDialog.cpp \
    BateauStatisticsDialog.cpp \
    PecheExportDialog.cpp \
    employee.cpp \
    frigo.cpp \
    bateau.cpp \
    LivraisonTrackingDialog.cpp


HEADERS += \
    connection.h \
    contractgenerator.h \
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
    Livraisonwindow.h \
    AddLivraisonDialog.h \
    quaiswindow.h \
    addquaidialog.h \
    quai.h \
    StatisticsDialog.h \
    LivraisonStatisticsDialog.h \
    FrigoStatisticsDialog.h \
    EmployeeStatsWindow.h \
    livraison.h \
    peche.h \
    PecheStatisticsDialog.h \
    BateauStatisticsDialog.h \
    PecheExportDialog.h \
    employee.h \
    frigo.h \
    bateau.h \
    LivraisonTrackingDialog.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += resources.qrc
