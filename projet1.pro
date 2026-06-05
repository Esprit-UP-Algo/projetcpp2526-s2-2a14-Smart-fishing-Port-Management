QT       += core gui widgets sql printsupport charts network serialport
QT += network

CONFIG   += c++17
TEMPLATE = app
TARGET = PortFlow

# Source files
SOURCES += main.cpp \
           aiintegration.cpp \
           mainwindow.cpp \
           arduino.cpp \
           AddLivraisonDialog.cpp \
           BateauStatisticsDialog.cpp \
           Bateaudialog.cpp \
           Bateauwindow.cpp \
           EmployeeStatsWindow.cpp \
           Employeedialog.cpp \
           Employeewindow.cpp \
           FrigoStatisticsDialog.cpp \
           Frigowindow.cpp \
           LivraisonStatisticsDialog.cpp \
           LivraisonTrackingDialog.cpp \
           Livraisonwindow.cpp \
           PecheExportDialog.cpp \
           PecheStatisticsDialog.cpp \
           Pechedialog.cpp \
           Pechewindow.cpp \
           PredictMaintenanceDialog.cpp \
           StatisticsDialog.cpp \
           addfrigodialog.cpp \
           addquaidialog.cpp \
           bateau.cpp \
           connection.cpp \
           employee.cpp \
           frigo.cpp \
           livraison.cpp \
           loginwindow.cpp \
           ortools_optimizer.cpp \
           peche.cpp \
           quai.cpp \
           quaimanager.cpp \
           quaiswindow.cpp \
           speech.cpp \
           temperaturealert.cpp \
           SmtpClient.cpp \
           StorageAlert.cpp

# Header files
HEADERS += mainwindow.h \
           aiintegration.h \
           arduino.h \
           AddLivraisonDialog.h \
           BateauStatisticsDialog.h \
           Bateaudialog.h \
           Bateauwindow.h \
           EmployeeStatsWindow.h \
           Employeedialog.h \
           Employeewindow.h \
           FrigoStatisticsDialog.h \
           Frigowindow.h \
           LivraisonStatisticsDialog.h \
           LivraisonTrackingDialog.h \
           Livraisonwindow.h \
           PecheExportDialog.h \
           PecheStatisticsDialog.h \
           Pechedialog.h \
           Pechewindow.h \
           PredictMaintenanceDialog.h \
           StatisticsDialog.h \
           addfrigodialog.h \
           addquaidialog.h \
           bateau.h \
           connection.h \
           employee.h \
           frigo.h \
           livraison.h \
           loginwindow.h \
           ortools_optimizer.h \
           peche.h \
           quai.h \
           quaimanager.h \
           quaiswindow.h \
           speech.h \
           temperaturealert.h \
           SmtpClient.h \
           StorageAlert.h

# UI Form files
FORMS += mainwindow.ui \
         Pechewindow.ui \
         bateau.ui \
         loginwindow.ui

# Resources
RESOURCES += resources.qrc
