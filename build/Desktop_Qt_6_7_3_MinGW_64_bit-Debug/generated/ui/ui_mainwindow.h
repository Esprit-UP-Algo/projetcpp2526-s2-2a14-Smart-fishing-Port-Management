/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QWidget *sidebar;
    QVBoxLayout *sideLayout;
    QLabel *logoLabel;
    QPushButton *btnDashboard;
    QPushButton *btnWeather;
    QPushButton *btnBoats;
    QPushButton *btnFishing;
    QPushButton *btnEmp;
    QPushButton *btnFrigo;
    QPushButton *btnSettings;
    QSpacerItem *verticalSpacer;
    QStackedWidget *pagesStack;
    QWidget *frigoPage;
    QVBoxLayout *frigoLayout;
    QHBoxLayout *topBar;
    QWidget *titleFrame;
    QHBoxLayout *titleLayout;
    QLabel *titleLabel;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnAddFrigo;
    QWidget *tableFrame;
    QVBoxLayout *frameLayout;
    QTableWidget *frigoTable;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 800);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        centralWidget->setStyleSheet(QString::fromUtf8("background:#000000;"));
        mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(15);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(15, 15, 15, 15);
        sidebar = new QWidget(centralWidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(230, 0));
        sidebar->setMaximumSize(QSize(230, 16777215));
        sideLayout = new QVBoxLayout(sidebar);
        sideLayout->setSpacing(5);
        sideLayout->setObjectName("sideLayout");
        sideLayout->setContentsMargins(0, 0, 0, 0);
        logoLabel = new QLabel(sidebar);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(0, 90));
        logoLabel->setMaximumSize(QSize(16777215, 90));

        sideLayout->addWidget(logoLabel);

        btnDashboard = new QPushButton(sidebar);
        btnDashboard->setObjectName("btnDashboard");

        sideLayout->addWidget(btnDashboard);

        btnWeather = new QPushButton(sidebar);
        btnWeather->setObjectName("btnWeather");

        sideLayout->addWidget(btnWeather);

        btnBoats = new QPushButton(sidebar);
        btnBoats->setObjectName("btnBoats");

        sideLayout->addWidget(btnBoats);

        btnFishing = new QPushButton(sidebar);
        btnFishing->setObjectName("btnFishing");

        sideLayout->addWidget(btnFishing);

        btnEmp = new QPushButton(sidebar);
        btnEmp->setObjectName("btnEmp");

        sideLayout->addWidget(btnEmp);

        btnFrigo = new QPushButton(sidebar);
        btnFrigo->setObjectName("btnFrigo");

        sideLayout->addWidget(btnFrigo);

        btnSettings = new QPushButton(sidebar);
        btnSettings->setObjectName("btnSettings");

        sideLayout->addWidget(btnSettings);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        sideLayout->addItem(verticalSpacer);


        mainLayout->addWidget(sidebar);

        pagesStack = new QStackedWidget(centralWidget);
        pagesStack->setObjectName("pagesStack");
        frigoPage = new QWidget();
        frigoPage->setObjectName("frigoPage");
        frigoLayout = new QVBoxLayout(frigoPage);
        frigoLayout->setSpacing(15);
        frigoLayout->setObjectName("frigoLayout");
        frigoLayout->setContentsMargins(20, 20, 20, 20);
        topBar = new QHBoxLayout();
        topBar->setObjectName("topBar");
        titleFrame = new QWidget(frigoPage);
        titleFrame->setObjectName("titleFrame");
        titleFrame->setMinimumSize(QSize(1000, 55));
        titleLayout = new QHBoxLayout(titleFrame);
        titleLayout->setObjectName("titleLayout");
        titleLayout->setContentsMargins(15, 8, 15, 8);
        titleLabel = new QLabel(titleFrame);
        titleLabel->setObjectName("titleLabel");

        titleLayout->addWidget(titleLabel);


        topBar->addWidget(titleFrame);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        topBar->addItem(horizontalSpacer);

        btnAddFrigo = new QPushButton(frigoPage);
        btnAddFrigo->setObjectName("btnAddFrigo");

        topBar->addWidget(btnAddFrigo);


        frigoLayout->addLayout(topBar);

        tableFrame = new QWidget(frigoPage);
        tableFrame->setObjectName("tableFrame");
        frameLayout = new QVBoxLayout(tableFrame);
        frameLayout->setSpacing(0);
        frameLayout->setObjectName("frameLayout");
        frameLayout->setContentsMargins(20, 20, 20, 20);
        frigoTable = new QTableWidget(tableFrame);
        frigoTable->setObjectName("frigoTable");

        frameLayout->addWidget(frigoTable);


        frigoLayout->addWidget(tableFrame);

        pagesStack->addWidget(frigoPage);

        mainLayout->addWidget(pagesStack);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        pagesStack->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "PortFlow Dashboard", nullptr));
        logoLabel->setText(QCoreApplication::translate("MainWindow", "LOGO", nullptr));
        btnDashboard->setText(QCoreApplication::translate("MainWindow", "\360\237\217\240 Dashboard", nullptr));
        btnWeather->setText(QCoreApplication::translate("MainWindow", "\360\237\214\244\357\270\217 Weather", nullptr));
        btnBoats->setText(QCoreApplication::translate("MainWindow", "\342\233\265 Bateaux", nullptr));
        btnFishing->setText(QCoreApplication::translate("MainWindow", "\360\237\220\237 P\303\252che", nullptr));
        btnEmp->setText(QCoreApplication::translate("MainWindow", "\360\237\221\245 Employ\303\251s", nullptr));
        btnFrigo->setText(QCoreApplication::translate("MainWindow", "\360\237\247\212 Frigos", nullptr));
        btnSettings->setText(QCoreApplication::translate("MainWindow", "\342\232\231\357\270\217 Param\303\250tres", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "Gestion des Frigos", nullptr));
        btnAddFrigo->setText(QCoreApplication::translate("MainWindow", "\342\236\225 Ajouter Frigo", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
