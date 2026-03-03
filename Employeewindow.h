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
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

struct Employee {
    QString id;
    QString cin;
    QString firstName;
    QString lastName;
    QString position;
    QString salary;
    QString date;
    QString status;  // Actif / Congé / Inactif
};

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
    void loadEmployeesFromDb();
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QString generateEmployeeId();

    QVector<Employee> employees;
    QTableWidget* table;
    QLineEdit* searchInput;
    QComboBox* sortCombo;
};

#endif // EMPLOYEEWINDOW_H
