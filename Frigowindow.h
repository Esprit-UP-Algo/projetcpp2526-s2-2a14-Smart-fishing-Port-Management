#ifndef FRIGOWINDOW_H
#define FRIGOWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QVector>
#include <QComboBox>
#include <QSqlRecord>
#include <QCalendarWidget>
#include <QListWidget>
#include <QMap>
#include <QDate>
#include <QShowEvent>
#include <QTextCharFormat>
#include <QBrush>
#include <QColor>
#include "frigo.h"

// Structure Frigo removed, using FrigoModel class

class FrigoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FrigoWindow(QWidget *parent = nullptr);
    ~FrigoWindow();

    void setDarkMode(bool dark);
    bool isDark() const { return isDarkMode; }

private slots:
    void onSearch(const QString& text);
    void onAddFrigo();
    void onEditFrigo(int row);
    void onDeleteFrigo(int row);
    void onDeleteFrigoById(const QString& id);
    void onLogout();
    void onSort(int index);
    void onGeneratePDF();
    void onShowStatistics();
    void onSendSMS();
    void onShowClassification();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createToolbar();
    QFrame* createTableCard();
    QFrame* createCalendarCard();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setupTable();
    void populateTable(const QString& filterText = "");
    void loadCalendarData();
    void updateCalendarDetails(const QDate &date);
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row, const QString& dbId);
    QString generateFrigoId();

    FrigoModel      frigoModel;
    QTableWidget*   table;
    QLineEdit*      searchInput;
    QComboBox*      sortCombo;

    QCalendarWidget* calendar;
    QListWidget*      calendarDetails;
    QMap<QDate, QStringList> maintenanceDates;
    QMap<QDate, QStringList> deliveryDates;
    bool isDarkMode = false;
};

#endif // FRIGOWINDOW_H
