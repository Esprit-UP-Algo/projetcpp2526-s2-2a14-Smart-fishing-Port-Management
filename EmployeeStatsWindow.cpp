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
    
    QHBoxLayout* titleLay = new QHBoxLayout();
    titleLay->addWidget(headTitle);
    titleLay->addStretch();
    
    QPushButton* aiInsightBtn = new QPushButton("✨ Insights IA (Google)");
    aiInsightBtn->setFixedSize(180, 40);
    aiInsightBtn->setCursor(Qt::PointingHandCursor);
    aiInsightBtn->setStyleSheet("background-color: #8B5CF6; color: white; font-weight: bold; border-radius: 20px;");
    connect(aiInsightBtn, &QPushButton::clicked, this, &EmployeeStatsWindow::onSmartInsight);
    titleLay->addWidget(aiInsightBtn);
    
    mainLayout->addLayout(titleLay);

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
    lay->setContentsMargins(15, 10, 15, 10);

    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 500; border: none;");
    
    QHBoxLayout* valLay = new QHBoxLayout();
    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: bold; border: none;").arg(color));

    valLay->addWidget(lblValue);
    
    if (value.contains("%")) {
        QLabel* arrow = new QLabel("⬆");
        arrow->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold; border: none;").arg(color));
        valLay->addWidget(arrow);
    }
    valLay->addStretch();

    lay->addWidget(lblTitle);
    lay->addLayout(valLay);
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
    QList<QPair<QString, double>> data = {
        {"Admin", 10}, {"Logistique", 45}, {"Sécurité", 30}, {"Maintenance", 25}
    };
    
    double total = 0;
    for(auto& p : data) total += p.second;

    for(auto& p : data) {
        QPieSlice *slice = series->append(p.first, p.second);
        if (total > 0)
            slice->setLabel(QString("%1 (%2%)").arg(p.first).arg(100.0 * p.second / total, 0, 'f', 1));
    }

    if(series->slices().size() > 1) {
        QPieSlice *slice = series->slices().at(1); // Logistique
        slice->setExploded();
        slice->setLabelVisible(true);
        slice->setPen(QPen(Qt::darkGreen, 2));
        slice->setBrush(Qt::green);
    }

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

#include "aiintegration.h"
#include <QMessageBox>
#include <QCoreApplication>

void EmployeeStatsWindow::onSmartInsight()
{
    // Gather global data
    double totalSalary = 0;
    QMap<QString, int> deptCounts;
    for (const auto& emp : m_employees) {
        QString cleanSal = emp.salary;
        cleanSal.remove(QRegularExpression("[^0-9]"));
        totalSalary += cleanSal.toDouble();
        deptCounts[emp.position]++;
    }

    QMessageBox* loading = new QMessageBox(this);
    loading->setWindowTitle("Consultation IA Google");
    loading->setText("L'IA analyse la structure salariale et la répartition des effectifs...");
    loading->setStandardButtons(QMessageBox::NoButton);
    loading->show();
    QCoreApplication::processEvents();

    // Use a generic prompt for global analysis
    EmployeeAnalysisContext context;
    context.name = "Analyse Globale du Port";
    context.position = QString("Effectif total: %1").arg(m_employees.size());
    context.salary = totalSalary;
    context.totalHours = m_employees.size() * 160; 
    
    EmployeeAnalysisReport report = EmployeeIntelligenceService::generateSmartReport(context);
    
    loading->close();
    delete loading;

    if (report.success) {
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("Insights IA Stratégiques - PortFlow");
        dialog->setMinimumWidth(600);
        
        // Since stats window seems to be light mode by default (from previous setup), use light theme colors
        // or ensure explicit styling for text readability
        QString bgColor = "#F8FAFC";
        QString cardBg = "#FFFFFF";
        QString textColor = "#1E293B";
        QString accentColor = "#8B5CF6";
        
        dialog->setStyleSheet(QString("QDialog { background-color: %1; color: %2; }").arg(bgColor, textColor));
        
        QVBoxLayout* layout = new QVBoxLayout(dialog);
        layout->setSpacing(20);
        layout->setContentsMargins(30, 30, 30, 30);
        
        QLabel* title = new QLabel("✨ Insights Stratégiques (Google AI)");
        title->setStyleSheet(QString("font-size: 22px; font-weight: bold; color: %1;").arg(accentColor));
        layout->addWidget(title);
        
        // Summary
        QFrame* summaryCard = new QFrame();
        summaryCard->setStyleSheet(QString("background-color: %1; border-radius: 12px; padding: 15px; border: 1px solid #E2E8F0;").arg(cardBg));
        QVBoxLayout* summaryLay = new QVBoxLayout(summaryCard);
        
        QLabel* summaryTitle = new QLabel("Résumé Global");
        summaryTitle->setStyleSheet(QString("font-weight: bold; font-size: 14px; color: %1;").arg(textColor));
        summaryLay->addWidget(summaryTitle);
        
        QLabel* summaryText = new QLabel(report.summary);
        summaryText->setStyleSheet(QString("color: %1;").arg(textColor));
        summaryText->setWordWrap(true);
        summaryLay->addWidget(summaryText);
        layout->addWidget(summaryCard);
        
        // Analysis
        QLabel* analysisTitle = new QLabel("📝 Analyse Structurelle");
        analysisTitle->setStyleSheet("font-weight: bold; color: #1E3A8A; margin-top: 10px;");
        layout->addWidget(analysisTitle);
        
        QLabel* analysisText = new QLabel(report.analysis);
        analysisText->setWordWrap(true);
        analysisText->setStyleSheet(QString(R"(
            QLabel { 
                line-height: 1.5; color: %1; background: %2; padding: 12px; 
                border: 1px solid #E5E7EB; border-radius: 10px; 
            }
        )").arg(textColor, cardBg));
        layout->addWidget(analysisText);
        
        // Recommendation
        QLabel* recoTitle = new QLabel("💡 Recommandations");
        recoTitle->setStyleSheet("font-weight: bold; color: #059669; margin-top: 10px;");
        layout->addWidget(recoTitle);
        
        QLabel* recoText = new QLabel(report.recommendation);
        recoText->setWordWrap(true);
        recoText->setStyleSheet(QString(R"(
            QLabel { 
                line-height: 1.5; color: %1; background: #ECFDF5; padding: 12px; 
                border: 1px solid #A7F3D0; border-radius: 10px; 
            }
        )").arg(textColor));
        layout->addWidget(recoText);
        
        QPushButton* closeBtn = new QPushButton("Fermer");
        closeBtn->setFixedHeight(45);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet(QString(R"(
            QPushButton { 
                background-color: %1; color: white; font-weight: bold; 
                border-radius: 10px; margin-top: 10px; 
            }
            QPushButton:hover { background-color: #7C3AED; }
        )").arg(accentColor));
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        layout->addWidget(closeBtn);
        
        dialog->exec();
    } else {
        QMessageBox::critical(this, "Erreur AI", "Impossible de générer l'insight IA pour le moment.");
    }
}
