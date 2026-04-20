#include "PecheStatisticsDialog.h"

PecheStatisticsDialog::PecheStatisticsDialog(const QMap<QString, double>& speciesCount,
                                               const QMap<QString, double>& weightBySpecies,
                                               const QMap<QString, double>& avgWeightByBoat,
                                               QWidget* parent)
    : QDialog(parent), m_speciesCount(speciesCount), m_weightBySpecies(weightBySpecies), m_avgWeightByBoat(avgWeightByBoat)
{
    setWindowTitle("Tableau de Bord - Statistiques des Pêches");
    resize(1100, 750);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void PecheStatisticsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Header Title
    QLabel* headTitle = new QLabel("📈 Analyse de la Production Halieutique");
    headTitle->setStyleSheet("font-size: 26px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // Top Row: Stat Cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    double totalWeight = 0;
    for(double w : m_weightBySpecies.values()) totalWeight += w;

    int totalLots = 0;
    for(double c : m_speciesCount.values()) totalLots += c;

    cardsLayout->addWidget(createStatCard("Volume Total", QString::number(totalWeight, 'f', 1) + " Kg", "#2563EB"));
    cardsLayout->addWidget(createStatCard("Nombre de Lots", QString::number(totalLots), "#059669"));
    cardsLayout->addWidget(createStatCard("Espèces Actives", QString::number(m_speciesCount.size()), "#D97706"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Pie Chart & Bar Chart
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(25);
    chartsLayout->addWidget(createSpeciesPieChart(), 2);
    chartsLayout->addWidget(createWeightBarChart(), 3);
    mainLayout->addLayout(chartsLayout, 1);
}

QFrame* PecheStatisticsDialog::createStatCard(const QString& title, const QString& value, const QString& color)
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

QChartView* PecheStatisticsDialog::createSpeciesPieChart()
{
    QPieSeries *series = new QPieSeries();
    for (auto it = m_speciesCount.begin(); it != m_speciesCount.end(); ++it) {
        series->append(it.key(), it.value());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des Lots par Espèce");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 9));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* PecheStatisticsDialog::createWeightBarChart()
{
    QBarSet *set = new QBarSet("Poids (Kg)");
    QStringList categories;

    for (auto it = m_weightBySpecies.begin(); it != m_weightBySpecies.end(); ++it) {
        *set << it.value();
        categories << it.key();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Volume Total par Espèce");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f Kg");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}
