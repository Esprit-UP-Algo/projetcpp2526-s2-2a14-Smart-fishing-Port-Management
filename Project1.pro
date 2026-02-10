QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
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
    AddLivraisonDialog.cpp

HEADERS += \
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
    AddLivraisonDialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
