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
#include <QMap>

struct Employee {
    QString id;
    QString firstName;  // Added
    QString lastName;   // Added
    QString position;
    QString salary;
    QString date;
    QString status;
};

class EmployeeWindow : public QWidget
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

private:
    void setupUi();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createTableCard();
    QWidget* createActionButtons(int row);
    QWidget* createDeleteButton(int row);
    QWidget* createStatusBadge(const QString& status);
    void setupTable();
    void populateTable(const QString& filterText = "");
    QString generateEmployeeId();

    QVector<Employee> employees;
    QTableWidget* table;
    QLineEdit* searchInput;
};

#endif // EMPLOYEEWINDOW_H
