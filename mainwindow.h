<<<<<<< HEAD
﻿#ifndef MAINWINDOW_H
=======
#ifndef MAINWINDOW_H
>>>>>>> master
#define MAINWINDOW_H

#include "pechewindow.h"
#include "Livraisonwindow.h"
#include "quaiswindow.h"
#include "Bateaudialog.h"
#include "Bateauwindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QPropertyAnimation>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavigateToEmployees();
    void onNavigateToFrigos();
    void onNavigateToPeches();
    void onNavigateToDashboard();
    void onNavigateToBateaux();
    void onNavigateToLivraison();
    void onNavigateToQuais();
    void toggleGlobalTheme();
    void toggleLanguage();
    void onLogout();
<<<<<<< HEAD
=======
    void onSettingsClicked();
>>>>>>> master

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createDashboardPage();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setActiveButton(QPushButton* activeBtn);
    void switchPage(int index);
    
    void updateThemeRecursive(QWidget* widget, bool isDark);
    void translateRecursive(QWidget* widget, bool toEnglish);

    QStackedWidget* stackedWidget;
    QVector<QPair<QString, QString>> notifications;
    QPushButton* dashboardBtn;
    QPushButton* employeesBtn;
    QPushButton* pechesBtn;
    QPushButton* frigosBtn;
    QPushButton* bateauxBtn;
    QPushButton* livraisonBtn;
    QPushButton* quaisBtn;
    QPushButton* darkModeBtn;
    QPushButton* langBtn;
    
    bool isDarkMode = false;
    bool isEnglish = false;
    QPushButton* currentActiveBtn;

    QWidget* employeePage;
    QWidget* pechePage;
    QWidget* frigoPage;
    QWidget* bateauPage;
    QWidget* livraisonPage;
    QWidget* quaisPage;

};

#endif // MAINWINDOW_H
