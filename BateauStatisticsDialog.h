#ifndef BATEAUSTATISTICSDIALOG_H
#define BATEAUSTATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMap>

class BateauStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BateauStatisticsDialog(const QMap<QString, double>& availabilityData,
                                   int totalBateaux,
                                   double totalCapacite,
                                   double avgAge,
                                   QWidget* parent = nullptr);

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createAvailabilityPieChart();

    QMap<QString, double> m_availabilityData;
    int m_totalBateaux;
    double m_totalCapacite;
    double m_avgAge;
};

#endif // BATEAUSTAISTICSDIALOG_H
