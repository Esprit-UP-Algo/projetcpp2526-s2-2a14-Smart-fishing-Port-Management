#ifndef MAINWINDOW_H
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
#include <QSystemTrayIcon>
#include "arduino.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& userName = "", const QString& userRole = "", QWidget *parent = nullptr);
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
    void onSettingsClicked();
    void onTrayMessageClicked();
    void handleSerialData(); // Unified slot for Arduino data
    void processPortAccess(const QString& code);

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createDashboardPage();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setActiveButton(QPushButton* activeBtn);
    void switchPage(int index);
    
    void updateThemeRecursive(QWidget* widget, bool isDark);
    void translateRecursive(QWidget* widget, bool toEnglish);
    void checkMaintenanceAlerts();

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
    QSystemTrayIcon* trayIcon = nullptr;

    Arduino A;
    QByteArray serialBuffer;
    void checkFridgeTemperature(int sensorId, double currentTemp);
    QString loggedUserName;
    QString loggedUserRole;
    QLabel* welcomeLabel;

};

#endif // MAINWINDOW_H
