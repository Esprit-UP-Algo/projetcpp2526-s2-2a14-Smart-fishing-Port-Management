#ifndef EMPLOYEESTATSWINDOW_H
#define EMPLOYEESTATSWINDOW_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>

class EmployeeStatsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeStatsWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createPositionBarChart();
    QChartView*  createSalaryLineChart();
    QChartView*  createStatusPieChart();
};

#endif // EMPLOYEESTATSWINDOW_H
