#ifndef EMPLOYEESTATSWINDOW_H
#define EMPLOYEESTATSWINDOW_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include "Employeewindow.h"

class EmployeeStatsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeStatsWindow(const QVector<Employee>& employees, QWidget *parent = nullptr);

private slots:
    void onSmartInsight();

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createDepartmentBarChart();
    QChartView*  createSalaryLineChart();
    QChartView*  createOvertimePieChart();

    QVector<Employee> m_employees;
};

#endif // EMPLOYEESTATSWINDOW_H
