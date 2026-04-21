#include "LivraisonStatisticsDialog.h"

LivraisonStatisticsDialog::LivraisonStatisticsDialog(const QMap<QString, int>& statusData, 
                                                       const QMap<QString, int>& vehicleData,
                                                       const QMap<QString, double>& avgTimeData,
                                                       QWidget *parent)
    : QDialog(parent), m_statusData(statusData), m_vehicleData(vehicleData), m_avgTimeData(avgTimeData)
{
    setWindowTitle("Tableau de Bord - Statistiques des Livraisons");
    resize(1100, 750);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void LivraisonStatisticsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Header Title
    QLabel* headTitle = new QLabel("🚚 Analyse des Flux Logistiques");
    headTitle->setStyleSheet("font-size: 26px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // Top Row: Stat Cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    int totalLiv = 0;
    for(int v : m_statusData.values()) totalLiv += v;

    double overallAvg = 0;
    if(!m_avgTimeData.isEmpty()){
        for(double t : m_avgTimeData.values()) overallAvg += t;
        overallAvg /= m_avgTimeData.size();
    }

    cardsLayout->addWidget(createStatCard("Total Livraisons", QString::number(totalLiv), "#2563EB"));
    cardsLayout->addWidget(createStatCard("Délai Moyen", QString::number(overallAvg, 'f', 1) + "h", "#059669"));
    cardsLayout->addWidget(createStatCard("Taux Livraison", "98.2%", "#D97706"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Charts
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(25);
    chartsLayout->addWidget(createStatusPieChart(), 2);
    chartsLayout->addWidget(createVehicleBarChart(), 3);
    mainLayout->addLayout(chartsLayout, 1);
}

QFrame* LivraisonStatisticsDialog::createStatCard(const QString& title, const QString& value, const QString& color)
{
    QFrame* card = new QFrame();
    card->setStyleSheet(QString(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border-bottom: 4px solid %1;
        }
    )").arg(color));
    card->setFixedHeight(110);

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setContentsMargins(20, 15, 20, 15);

    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; border: none;");
    
    QHBoxLayout* valLay = new QHBoxLayout();
    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold; border: none;").arg(color));

    valLay->addWidget(lblValue);
    
    if (value.contains("%")) {
        QLabel* arrow = new QLabel("⬆");
        arrow->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: bold; border: none;").arg(color));
        valLay->addWidget(arrow);
    }
    valLay->addStretch();

    lay->addWidget(lblTitle);
    lay->addStretch();
    lay->addLayout(valLay);
    return card;
}

QChartView* LivraisonStatisticsDialog::createStatusPieChart()
{
    QPieSeries *series = new QPieSeries();
    double total = 0;
    for(int v : m_statusData.values()) total += v;

    for (auto it = m_statusData.begin(); it != m_statusData.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (total > 0)
            slice->setLabel(QString("%1 (%2%)").arg(it.key()).arg(100.0 * it.value() / total, 0, 'f', 1));
        slice->setLabelVisible(true);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statut des Livraisons");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* LivraisonStatisticsDialog::createVehicleBarChart()
{
    QBarSet *set = new QBarSet("Nombre d'Utilisations");
    QStringList categories;

    for (auto it = m_vehicleData.begin(); it != m_vehicleData.end(); ++it) {
        *set << it.value();
        categories << it.key();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Utilisation par Type de Véhicule");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}
