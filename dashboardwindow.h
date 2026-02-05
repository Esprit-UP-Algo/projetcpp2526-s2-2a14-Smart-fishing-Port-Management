#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QStackedWidget>

class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

private:
    QTableWidget *frigoTable;
    QStackedWidget *pages;
    QWidget *employeePage;  // Changed to generic QWidget or keep specific type if forward declared
    QWidget *frigoPage;

    QPushButton* createMenuBtn(QString text);
    void addRow(QString id, QString cap, QString hum,
                QString temp, QString stat, QString fish);
    void setupFrigoPage(QWidget *frigoPage);

private slots:
    void switchPage(QWidget *page);
};

#endif // DASHBOARDWINDOW_H
