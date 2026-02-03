QT += core gui widgets

TARGET = Project1
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
           mainwindow.cpp \
           addfrigodialog.cpp

HEADERS += mainwindow.h \
           addfrigodialog.h

FORMS += mainwindow.ui \
         addfrigodialog.ui

RESOURCES += resources.qrc
