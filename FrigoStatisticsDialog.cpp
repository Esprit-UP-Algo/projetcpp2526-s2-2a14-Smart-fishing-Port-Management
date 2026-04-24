#include "FrigoStatisticsDialog.h"

FrigoStatisticsDialog::FrigoStatisticsDialog(const QMap<QString, double>& typeCount,
                                               const QMap<QString, double>& typeCapacity,
                                               const QMap<QString, double>& statusCount,
                                               double totalOccupation,
                                               QWidget* parent)
    : QDialog(parent), m_typeCount(typeCount), m_typeCapacity(typeCapacity), m_statusCount(statusCount), m_totalOccupation(totalOccupation)
{
    setWindowTitle("Tableau de Bord - Statistiques des Frigos");
    resize(1200, 800);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void FrigoStatisticsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Header Title
    QLabel* headTitle = new QLabel("❄️ Analyse des Unités de Réfrigération");
    headTitle->setStyleSheet("font-size: 26px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // Top Row: Stat Cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    double totalCap = 0;
    for(double c : m_typeCapacity.values()) totalCap += c;

    int totalFrigos = 0;
    for(double c : m_typeCount.values()) totalFrigos += c;
    
    double occPercent = (totalCap > 0) ? (m_totalOccupation / totalCap) * 100.0 : 0.0;

    cardsLayout->addWidget(createStatCard("Total Unités", QString::number(totalFrigos), "#2563EB"));
    cardsLayout->addWidget(createStatCard("Capacité Totale", QString::number(totalCap, 'f', 1) + " Kg", "#059669"));
    cardsLayout->addWidget(createStatCard("Taux d'Occupation", QString::number(occPercent, 'f', 1) + "%", "#7C3AED"));
    cardsLayout->addWidget(createStatCard("Types Actifs", QString::number(m_typeCount.size()), "#D97706"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Pie Charts
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(25);
    chartsLayout->addWidget(createTypePieChart(), 1);
    chartsLayout->addWidget(createStatusPieChart(), 1);
    chartsLayout->addWidget(createOccupancyChart(), 1);
    mainLayout->addLayout(chartsLayout, 1);
}

QFrame* FrigoStatisticsDialog::createStatCard(const QString& title, const QString& value, const QString& color)
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

QChartView* FrigoStatisticsDialog::createTypePieChart()
{
    QPieSeries *series = new QPieSeries();
    double totalLots = 0;
    for(double v : m_typeCount.values()) totalLots += v;

    for (auto it = m_typeCount.begin(); it != m_typeCount.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (totalLots > 0)
            slice->setLabel(QString("%1 ⬆ %2%").arg(it.key()).arg(100.0 * it.value() / totalLots, 0, 'f', 1));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition par Type de Poisson");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* FrigoStatisticsDialog::createStatusPieChart()
{
    QPieSeries *series = new QPieSeries();
    double total = 0;
    for(double v : m_statusCount.values()) total += v;

    for (auto it = m_statusCount.begin(); it != m_statusCount.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (total > 0)
            slice->setLabel(QString("%1 ⬆ %2%").arg(it.key()).arg(100.0 * it.value() / total, 0, 'f', 1));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Disponibilité des Frigos");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* FrigoStatisticsDialog::createOccupancyChart()
{
    double totalCap = 0;
    for(double c : m_typeCapacity.values()) totalCap += c;
    double freeSpace = qMax(0.0, totalCap - m_totalOccupation);

    QPieSeries *series = new QPieSeries();
    QPieSlice *usedSlice = series->append("Occupé", m_totalOccupation);
    QPieSlice *freeSlice = series->append("Libre", freeSpace);

    usedSlice->setBrush(QColor("#7C3AED"));
    freeSlice->setBrush(QColor("#E2E8F0"));
    
    if (totalCap > 0) {
        usedSlice->setLabel(QString("Occupé ⬆ %1%").arg(100.0 * m_totalOccupation / totalCap, 0, 'f', 1));
        freeSlice->setLabel(QString("Libre ⬆ %1%").arg(100.0 * freeSpace / totalCap, 0, 'f', 1));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Utilisation Globale de Capacité");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}
