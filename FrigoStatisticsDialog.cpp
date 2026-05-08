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

    // Palette de couleurs élégante pour les types de poissons
    QMap<QString, QColor> colorMap;
    colorMap["Sardine"] = QColor("#3B82F6"); // Bleu
    colorMap["Thon"]    = QColor("#EF4444"); // Rouge
    colorMap["Merlan"]  = QColor("#10B981"); // Vert
    colorMap["Crevette"]= QColor("#F59E0B"); // Orange
    colorMap["Saumon"]  = QColor("#8B5CF6"); // Violet
    colorMap["Sans"]    = QColor("#94A3B8"); // Gris

    int i = 0;
    QList<QColor> defaultColors = {QColor("#EC4899"), QColor("#06B6D4"), QColor("#F43F5E"), QColor("#14B8A6")};

    for (auto it = m_typeCount.begin(); it != m_typeCount.end(); ++it) {
        double percentage = (totalLots > 0) ? (100.0 * it.value() / totalLots) : 0.0;
        QPieSlice *slice = series->append(it.key(), it.value());

        if (colorMap.contains(it.key())) {
            slice->setBrush(colorMap[it.key()]);
        } else {
            slice->setBrush(defaultColors[i % defaultColors.size()]);
            i++;
        }

        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        slice->setLabel(QString("%1%").arg(percentage, 0, 'f', 1));
        slice->setLabelColor(Qt::white);
        slice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition par Type de Poisson");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Style de la légende
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 10, QFont::Medium));
    chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    // Personnaliser la légende pour afficher les Noms au lieu des Pourcentages
    const auto markers = chart->legend()->markers(series);
    auto itMap = m_typeCount.begin();
    for (int j = 0; j < markers.count() && itMap != m_typeCount.end(); ++j, ++itMap) {
        markers[j]->setLabel(itMap.key());
    }

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
        double percentage = (total > 0) ? (100.0 * it.value() / total) : 0.0;
        QPieSlice *slice = series->append(it.key(), it.value());

        if (it.key() == "Disponible") slice->setBrush(QColor("#10B981"));
        else if (it.key() == "Occupé") slice->setBrush(QColor("#F59E0B"));
        else slice->setBrush(QColor("#EF4444"));

        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        slice->setLabel(QString("%1%").arg(percentage, 0, 'f', 1));
        slice->setLabelColor(Qt::white);
        slice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Disponibilité des Frigos");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 10, QFont::Medium));
    chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    // Personnaliser la légende pour afficher les Statuts
    const auto markers = chart->legend()->markers(series);
    auto itStat = m_statusCount.begin();
    for (int j = 0; j < markers.count() && itStat != m_statusCount.end(); ++j, ++itStat) {
        markers[j]->setLabel(itStat.key());
    }

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
        usedSlice->setLabelVisible(true);
        usedSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        usedSlice->setLabel(QString("%1%").arg(100.0 * m_totalOccupation / totalCap, 0, 'f', 1));
        usedSlice->setLabelColor(Qt::white);
        usedSlice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));

        freeSlice->setLabelVisible(true);
        freeSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        freeSlice->setLabel(QString("%1%").arg(100.0 * freeSpace / totalCap, 0, 'f', 1));
        freeSlice->setLabelColor(Qt::black);
        freeSlice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Utilisation Globale de Capacité");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 10, QFont::Medium));
    chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    // Personnaliser la légende pour afficher "Occupé" et "Libre" au lieu des pourcentages
    const auto markers = chart->legend()->markers(series);
    if(markers.count() >= 2) {
        markers[0]->setLabel("Occupé");
        markers[1]->setLabel("Libre");
    }

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    return chartView;
}
