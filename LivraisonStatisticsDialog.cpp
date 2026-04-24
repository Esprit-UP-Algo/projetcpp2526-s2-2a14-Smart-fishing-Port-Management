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
    int deliveredCount = 0;
    int canceledCount = 0;

    for(auto it = m_statusData.begin(); it != m_statusData.end(); ++it) {
        totalLiv += it.value();
        if (it.key() == "Livré" || it.key() == "Arrivé") {
            deliveredCount += it.value();
        } else if (it.key() == "Annulé" || it.key() == "Canceled") {
            canceledCount += it.value();
        }
    }

    double deliveryRate = (totalLiv > 0) ? (static_cast<double>(deliveredCount) / totalLiv * 100.0) : 0.0;

    double overallAvg = 0;
    if(!m_avgTimeData.isEmpty()){
        for(double t : m_avgTimeData.values()) overallAvg += t;
        overallAvg /= m_avgTimeData.size();
    }

    cardsLayout->addWidget(createStatCard("Total Livraisons", QString::number(totalLiv), "#2563EB"));
    cardsLayout->addWidget(createStatCard("Délai Moyen", QString::number(overallAvg, 'f', 1) + "h", "#059669"));
    cardsLayout->addWidget(createStatCard("Taux Livraison", QString::number(deliveryRate, 'f', 1) + "%", "#D97706"));
    cardsLayout->addWidget(createStatCard("Annulées", QString::number(canceledCount), "#EF4444"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Charts
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(25);
    // Give more stretch to the pie chart so labels have room (equal 1:1)
    chartsLayout->addWidget(createStatusPieChart(), 1); 
    chartsLayout->addWidget(createVehicleBarChart(), 1);
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
    
    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold; border: none;").arg(color));

    lay->addWidget(lblTitle);
    lay->addStretch();
    lay->addWidget(lblValue);
    return card;
}

QChartView* LivraisonStatisticsDialog::createStatusPieChart()
{
    QPieSeries *series = new QPieSeries();
    series->setPieSize(0.45); // Make pie smaller to give labels more breathing room
    int total = 0;
    for (int count : m_statusData.values()) total += count;

    for (auto it = m_statusData.begin(); it != m_statusData.end(); ++it) {
        double percentage = (total > 0) ? (static_cast<double>(it.value()) / total * 100.0) : 0.0;
        QString label = QString("%1: %2%").arg(it.key()).arg(QString::number(percentage, 'f', 1));
        QPieSlice *slice = series->append(label, it.value());
        
        // Explicitly set each slice label to be visible and positioned outside
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        
        // Make labels small but visible as requested
        QFont labelFont("Segoe UI", 8); // Reduced to 8pt, non-bold for compactness
        slice->setLabelFont(labelFont);
        slice->setLabelArmLengthFactor(0.12); // Slightly shorter arms

        // Maintain specific styling for Arrivé and Annulé
        if (it.key() == "Arrivé" || it.key() == "Livré") {
            slice->setBrush(QColor("#10B981")); // Success Green
            slice->setExploded(true);
        } else if (it.key() == "Annulé" || it.key() == "Canceled") {
            slice->setBrush(QColor("#EF4444")); // Failure Red
        } else if (it.key() == "En attente") {
            slice->setBrush(QColor("#64748B")); // Gray for waiting
        } else if (it.key() == "En chemin") {
            slice->setBrush(QColor("#3B82F6")); // Blue for transit
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des Statuts de Livraison (%)");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 8));
    
    // Standard margins
    chart->setMargins(QMargins(15, 15, 15, 15));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border: 1px solid #e2e8f0; border-radius: 8px;");
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
