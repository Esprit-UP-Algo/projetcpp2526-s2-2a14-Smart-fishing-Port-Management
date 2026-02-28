#ifndef EMPLOYEEWINDOW_H
#define EMPLOYEEWINDOW_H

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
#include "employee.h"

// Structure removed, using EmployeeModel now

class EmployeeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EmployeeWindow(QWidget *parent = nullptr);
    ~EmployeeWindow();

private slots:
    void onSearch(const QString& text);
    void onAddEmployee();
    void onEditEmployee(int row);
    void onDeleteEmployee(int row);
    void onLogout();
    void onSort(int index);
    void onReglementInterieur();
    void onDemandeConge();
    void onAttestationTravail();
    void onViewStats();

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createToolbar();
    QFrame* createTableCard();
    QFrame* createSideActionsPanel();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setupTable();
    void populateTable(const QString& filterText = "");
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QString generateEmployeeId();

    EmployeeModel   employeeModel;
    QTableWidget*   table;
    QLineEdit*      searchInput;
    QComboBox*      sortCombo;
};

#endif // EMPLOYEEWINDOW_H
