#include "EmployeeStatsWindow.h"

EmployeeStatsWindow::EmployeeStatsWindow(const QVector<Employee>& employees, QWidget *parent)
    : QDialog(parent), m_employees(employees)
{
    setWindowTitle("Tableau de Bord - Statistiques Employés");
    resize(1000, 700);
    setStyleSheet("QDialog { background-color: #f8fafc; }");
    setupUi();
}

void EmployeeStatsWindow::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // Title
    QLabel* headTitle = new QLabel("📊 Statistiques Générales");
    headTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(headTitle);

    // Top Row: Cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);
    
    int totalEmp = m_employees.size();
    // Simulate some logic for absenteeism (e.g., random but stable based on employee count)
    double absRate = (totalEmp > 0) ? 4.2 : 0.0; 

    cardsLayout->addWidget(createStatCard("Total Employés", QString::number(totalEmp), "#2563EB"));
    cardsLayout->addWidget(createStatCard("Taux Absentéisme", QString::number(absRate, 'f', 1) + "%", "#EF4444"));
    mainLayout->addLayout(cardsLayout);

    // Middle Row: Bar & Line Charts
    QHBoxLayout* middleLayout = new QHBoxLayout();
    middleLayout->setSpacing(20);
    middleLayout->addWidget(createDepartmentBarChart(), 1);
    middleLayout->addWidget(createSalaryLineChart(), 1);
    mainLayout->addLayout(middleLayout);

    // Bottom Row: Pie Chart
    mainLayout->addWidget(createOvertimePieChart(), 1);
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
    card->setFixedHeight(100);

    QVBoxLayout* lay = new QVBoxLayout(card);
    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 500; border: none;");
    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: bold; border: none;").arg(color));
    
    lay->addWidget(lblTitle);
    lay->addWidget(lblValue);
    return card;
}

QChartView* EmployeeStatsWindow::createDepartmentBarChart()
{
    QBarSet *set0 = new QBarSet("Employés");
    
    // Count by position (acting as departments)
    QMap<QString, int> counts;
    for (const auto& emp : m_employees) {
        counts[emp.position]++;
    }

    QStringList categories;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        *set0 << it.value();
        categories << it.key();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set0);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Employés par Département");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* EmployeeStatsWindow::createSalaryLineChart()
{
    QLineSeries *series = new QLineSeries();
    
    // Simulate salary trend based on hiring years
    QMap<int, double> yearSum;
    for (const auto& emp : m_employees) {
        int year = QDate::fromString(emp.date, "dd/MM/yyyy").year();
        if (year < 2000) year = 2024; // Fallback
        
        QString cleanSal = emp.salary;
        cleanSal.remove("DT").remove(" ").remove(",");
        yearSum[year] += cleanSal.toDouble();
    }

    for (auto it = yearSum.begin(); it != yearSum.end(); ++it) {
        series->append(it.key(), it.value());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Masse Salariale par Année");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis *axisX = new QValueAxis();
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Année");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("DT");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return chartView;
}

QChartView* EmployeeStatsWindow::createOvertimePieChart()
{
    QPieSeries *series = new QPieSeries();
    
    // Simulating overtime categories
    series->append("Admin (10h)", 10);
    series->append("Logistique (45h)", 45);
    series->append("Sécurité (30h)", 30);
    series->append("Maintenance (25h)", 25);

    QPieSlice *slice = series->slices().at(1);
    slice->setExploded();
    slice->setLabelVisible();
    slice->setPen(QPen(Qt::darkGreen, 2));
    slice->setBrush(Qt::green);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition Heures Supplémentaires");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #e2e8f0;");
    return chartView;
}
