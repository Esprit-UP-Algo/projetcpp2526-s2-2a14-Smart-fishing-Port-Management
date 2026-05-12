#include "BateauStatisticsDialog.h"

BateauStatisticsDialog::BateauStatisticsDialog(const QMap<QString, double>& availabilityData,
                                               int totalBateaux,
                                               double totalCapacite,
                                               double avgAge,
                                               QWidget* parent)
    : QDialog(parent), m_availabilityData(availabilityData), m_totalBateaux(totalBateaux), 
      m_totalCapacite(totalCapacite), m_avgAge(avgAge)
{
    setWindowTitle("Tableau de Bord - Statistiques de la Flotte");
    resize(1000, 700);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void BateauStatisticsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Header Title
    QLabel* headTitle = new QLabel("⚓ Analyse de la Flotte");
    headTitle->setStyleSheet("font-size: 26px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // Top Row: Stat Cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    double availableCount = m_availabilityData.value("Au port", 0) + m_availabilityData.value("", 0);
    double availPercent = (m_totalBateaux > 0) ? (availableCount / m_totalBateaux) * 100.0 : 0.0;

    cardsLayout->addWidget(createStatCard("Total Bateaux", QString::number(m_totalBateaux), "#2563EB"));
    cardsLayout->addWidget(createStatCard("Capacité Totale", QString::number(m_totalCapacite, 'f', 1) + " T", "#059669"));
    cardsLayout->addWidget(createStatCard("Disponibilité", QString::number(availPercent, 'f', 1) + "%", "#7C3AED"));
    cardsLayout->addWidget(createStatCard("Âge Moyen", QString::number(m_avgAge, 'f', 1) + " ans", "#D97706"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Pie Chart
    mainLayout->addWidget(createAvailabilityPieChart(), 1);
}

QFrame* BateauStatisticsDialog::createStatCard(const QString& title, const QString& value, const QString& color)
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

QChartView* BateauStatisticsDialog::createAvailabilityPieChart()
{
    QPieSeries *series = new QPieSeries();
    double total = 0;
    for(double v : m_availabilityData.values()) total += v;

    for (auto it = m_availabilityData.begin(); it != m_availabilityData.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (total > 0)
            slice->setLabel(QString("%1 ⬆ %2%").arg(it.key()).arg(100.0 * it.value() / total, 0, 'f', 1));
        slice->setLabelVisible(true);
    }

    if (series->slices().size() > 0) {
        series->slices().at(0)->setExploded();
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Disponibilité des Bateaux");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->setFont(QFont("Segoe UI", 10));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}
