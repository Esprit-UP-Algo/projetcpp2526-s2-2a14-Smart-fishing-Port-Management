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
    QPushButton *btnEmp;
    QPushButton *btnDashboard;
    QPushButton *btnFrigo;
    QPushButton *btnFishing;
    QPushButton *btnWeather;
    QPushButton *btnBoats;
    QPushButton *btnSettings;
    QSpacerItem *verticalSpacer;
    QStackedWidget *pagesStack;
    QWidget *frigoPage;
    QVBoxLayout *frigoLayout;
    QWidget *headerWidget;
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnAddFrigo;
    QVBoxLayout *frameLayout;
    QWidget *tableCard;
    QVBoxLayout *cardLayout;
    QTableWidget *frigoTable;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 800);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QWidget(centralWidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(260, 0));
        sidebar->setMaximumSize(QSize(260, 16777215));
        sideLayout = new QVBoxLayout(sidebar);
        sideLayout->setSpacing(8);
        sideLayout->setObjectName("sideLayout");
        sideLayout->setContentsMargins(10, 20, 10, 20);
        logoLabel = new QLabel(sidebar);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(0, 90));

        sideLayout->addWidget(logoLabel);

        btnEmp = new QPushButton(sidebar);
        btnEmp->setObjectName("btnEmp");

        sideLayout->addWidget(btnEmp);

        btnDashboard = new QPushButton(sidebar);
        btnDashboard->setObjectName("btnDashboard");

        sideLayout->addWidget(btnDashboard);

        btnFrigo = new QPushButton(sidebar);
        btnFrigo->setObjectName("btnFrigo");

        sideLayout->addWidget(btnFrigo);

        btnFishing = new QPushButton(sidebar);
        btnFishing->setObjectName("btnFishing");

        sideLayout->addWidget(btnFishing);

        btnWeather = new QPushButton(sidebar);
        btnWeather->setObjectName("btnWeather");

        sideLayout->addWidget(btnWeather);

        btnBoats = new QPushButton(sidebar);
        btnBoats->setObjectName("btnBoats");

        sideLayout->addWidget(btnBoats);

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
        headerWidget = new QWidget(frigoPage);
        headerWidget->setObjectName("headerWidget");
        headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(20, 15, 20, 15);
        titleLabel = new QLabel(headerWidget);
        titleLabel->setObjectName("titleLabel");

        headerLayout->addWidget(titleLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        headerLayout->addItem(horizontalSpacer);

        btnAddFrigo = new QPushButton(headerWidget);
        btnAddFrigo->setObjectName("btnAddFrigo");

        headerLayout->addWidget(btnAddFrigo);


        frigoLayout->addWidget(headerWidget);

        frameLayout = new QVBoxLayout();
        frameLayout->setSpacing(0);
        frameLayout->setObjectName("frameLayout");
        frameLayout->setContentsMargins(0, 0, 0, 0);
        tableCard = new QWidget(frigoPage);
        tableCard->setObjectName("tableCard");
        cardLayout = new QVBoxLayout(tableCard);
        cardLayout->setSpacing(0);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(15, 15, 15, 15);
        frigoTable = new QTableWidget(tableCard);
        frigoTable->setObjectName("frigoTable");
        frigoTable->setFrameShape(QFrame::NoFrame);
        frigoTable->setShowGrid(false);

        cardLayout->addWidget(frigoTable);


        frameLayout->addWidget(tableCard);


        frigoLayout->addLayout(frameLayout);

        pagesStack->addWidget(frigoPage);

        mainLayout->addWidget(pagesStack);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        pagesStack->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "PortFlow - Delivery Management", nullptr));
        logoLabel->setText(QString());
        btnEmp->setText(QCoreApplication::translate("MainWindow", "\360\237\221\244 User", nullptr));
        btnDashboard->setText(QCoreApplication::translate("MainWindow", "\342\232\223 Docks", nullptr));
        btnFrigo->setText(QCoreApplication::translate("MainWindow", "\342\235\204\357\270\217 Cold Storage", nullptr));
        btnFishing->setText(QCoreApplication::translate("MainWindow", "\360\237\232\232 Delivery", nullptr));
        btnWeather->setText(QCoreApplication::translate("MainWindow", "\360\237\220\237 Fishbatch", nullptr));
        btnBoats->setText(QCoreApplication::translate("MainWindow", "\342\233\265 Boat Management", nullptr));
        btnSettings->setText(QCoreApplication::translate("MainWindow", "\342\232\231\357\270\217 Param\303\250tres", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "Delivery Management", nullptr));
        btnAddFrigo->setText(QCoreApplication::translate("MainWindow", "\342\236\225 Add Delivery", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
