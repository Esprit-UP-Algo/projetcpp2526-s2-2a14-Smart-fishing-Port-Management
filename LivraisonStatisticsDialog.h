#ifndef LIVRAISONSTATISTICSDIALOG_H
#define LIVRAISONSTATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMap>

class LivraisonStatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LivraisonStatisticsDialog(const QMap<QString, int>& statusData, 
        const QMap<QString, int>& vehicleData,
        const QMap<QString, double>& avgTimeData,
        QWidget *parent = nullptr);

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createStatusPieChart();
    QChartView*  createVehicleBarChart();

    QMap<QString, int> m_statusData;
    QMap<QString, int> m_vehicleData;
    QMap<QString, double> m_avgTimeData;
};

#endif // LIVRAISONSTATISTICSDIALOG_H
