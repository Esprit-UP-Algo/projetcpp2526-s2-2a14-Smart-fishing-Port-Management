#ifndef PECHESTATISTICSDIALOG_H
#define PECHESTATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMap>

class PecheStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PecheStatisticsDialog(const QMap<QString, double>& speciesCount,
                                   const QMap<QString, double>& weightBySpecies,
                                   const QMap<QString, double>& avgWeightByBoat,
                                   QWidget* parent = nullptr);

private:
    void setupUi();
    QFrame*      createStatCard(const QString& title, const QString& value, const QString& color);
    QChartView*  createSpeciesPieChart();
    QChartView*  createWeightBarChart();
    QChartView*  createWeightPieChart();

    QMap<QString, double> m_speciesCount;
    QMap<QString, double> m_weightBySpecies;
    QMap<QString, double> m_avgWeightByBoat;
};

#endif // PECHESTATISTICSDIALOG_H
