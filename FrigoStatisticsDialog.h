#ifndef FRIGOSTATISTICSDIALOG_H
#define FRIGOSTATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMap>

class FrigoStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrigoStatisticsDialog(const QMap<QString, double>& typeCount,
                                   const QMap<QString, double>& typeCapacity,
                                   const QMap<QString, double>& statusCount,
                                   double totalOccupation,
                                   QWidget* parent = nullptr);

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createTypePieChart();
    QChartView*  createStatusPieChart();
    QChartView*  createOccupancyChart();

    QMap<QString, double> m_typeCount;
    QMap<QString, double> m_typeCapacity;
    QMap<QString, double> m_statusCount;
    double m_totalOccupation;
};

#endif // FRIGOSTATISTICSDIALOG_H
