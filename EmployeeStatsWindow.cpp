#include "EmployeeStatsWindow.h"
#include <QSqlQuery>
#include <QDate>
#include <QPainter>
#include <QMap>

EmployeeStatsWindow::EmployeeStatsWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Tableau de Bord - Statistiques Employés");
    resize(1050, 720);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void EmployeeStatsWindow::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // Title
    QLabel* headTitle = new QLabel("📊  Statistiques des Employés");
    headTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // ── Stat Cards ─────────────────────────────────────────────
    QSqlQuery cntQ;
    cntQ.exec("SELECT COUNT(*), SUM(SALAIRE), AVG(SALAIRE) FROM EMPLOYES");
    int    total    = 0;
    double totalSal = 0.0, avgSal = 0.0;
    if (cntQ.next()) {
        total    = cntQ.value(0).toInt();
        totalSal = cntQ.value(1).toDouble();
        avgSal   = cntQ.value(2).toDouble();
    }

    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);
    cardsLayout->addWidget(createStatCard("Total Employés",   QString::number(total),                    "#2563EB"));
    cardsLayout->addWidget(createStatCard("Masse Salariale",  QString::number(totalSal, 'f', 0) + " DT", "#7C3AED"));
    cardsLayout->addWidget(createStatCard("Salaire Moyen",    QString::number(avgSal,   'f', 0) + " DT", "#059669"));
    mainLayout->addLayout(cardsLayout);

    // ── Charts Row ─────────────────────────────────────────────
    QHBoxLayout* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(16);
    chartsRow->addWidget(createPositionBarChart(), 1);
    chartsRow->addWidget(createStatusPieChart(),   1);
    mainLayout->addLayout(chartsRow, 1);

    // ── Salary line ────────────────────────────────────────────
    mainLayout->addWidget(createSalaryLineChart(), 1);
}

QFrame* EmployeeStatsWindow::createStatCard(const QString& title, const QString& value, const QString& color)
{
    QFrame* card = new QFrame();
    card->setStyleSheet(QString(R"(
        QFrame {
            background-color: white;
            border-radius: 12px;
            border-left: 5px solid %1;
        }
    )").arg(color));
    card->setFixedHeight(90);

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 10, 16, 10);

    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #64748b; font-size: 13px; font-weight: 500; border: none;");

    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("color: %1; font-size: 26px; font-weight: bold; border: none;").arg(color));

    lay->addWidget(lblTitle);
    lay->addWidget(lblValue);
    return card;
}

QChartView* EmployeeStatsWindow::createPositionBarChart()
{
    // Count employees by position
    QMap<QString, int> counts;
    QSqlQuery q("SELECT POSITION, COUNT(*) FROM EMPLOYES GROUP BY POSITION");
    while (q.next()) {
        counts[q.value(0).toString()] = q.value(1).toInt();
    }

    QBarSet* set0 = new QBarSet("Employés");
    QStringList categories;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        *set0 << it.value();
        categories << it.key();
    }

    QBarSeries* series = new QBarSeries();
    series->append(set0);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Employés par Position");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    QChartView* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return view;
}

QChartView* EmployeeStatsWindow::createSalaryLineChart()
{
    // Salary mass per hiring year
    QMap<int, double> yearSum;
    QSqlQuery q("SELECT DATEDERECRUTEMENT, SALAIRE FROM EMPLOYES");
    while (q.next()) {
        int year = q.value(0).toDate().year();
        if (year < 2000) year = QDate::currentDate().year();
        yearSum[year] += q.value(1).toDouble();
    }

    QLineSeries* series = new QLineSeries();
    series->setName("Masse Salariale (DT)");
    for (auto it = yearSum.begin(); it != yearSum.end(); ++it)
        series->append(it.key(), it.value());

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Masse Salariale par Année de Recrutement");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis* axisX = new QValueAxis();
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Année");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("DT");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return view;
}

QChartView* EmployeeStatsWindow::createStatusPieChart()
{
    QPieSeries* series = new QPieSeries();

    QSqlQuery q("SELECT STATUT, COUNT(*) FROM EMPLOYES GROUP BY STATUT");
    while (q.next()) {
        QString statut = q.value(0).toString();
        int     cnt    = q.value(1).toInt();
        QPieSlice* slice = series->append(statut + " (" + QString::number(cnt) + ")", cnt);
        slice->setLabelVisible(true);
    }

    // Style the first slice
    if (!series->slices().isEmpty())
        series->slices().first()->setExploded(true);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition par Statut");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);

    QChartView* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return view;
}
