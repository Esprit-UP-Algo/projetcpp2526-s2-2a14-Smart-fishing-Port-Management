#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void onNavigateToDashboard();
    void onLogout();

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createDashboardPage();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setActiveButton(QPushButton* activeBtn);
    void switchPage(int index);

    QStackedWidget* stackedWidget;
    QPushButton* dashboardBtn;
    QPushButton* employeesBtn;
    QPushButton* frigosBtn;
    QPushButton* currentActiveBtn;

    QWidget* employeePage;
    QWidget* frigoPage;
};

#endif // MAINWINDOW_H
