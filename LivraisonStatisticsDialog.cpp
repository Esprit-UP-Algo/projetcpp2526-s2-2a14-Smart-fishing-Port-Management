#include "LivraisonStatisticsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QPainter>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QTimer>

LivraisonStatisticsDialog::LivraisonStatisticsDialog(
    const QMap<QString, int>& statusData, 
    const QMap<QString, int>& vehicleData,
    const QMap<QString, double>& avgTimeData,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques de Livraison");
    setMinimumSize(900, 700);
    setStyleSheet("background-color: #F0F4F8;");
    setupUi(statusData, vehicleData, avgTimeData);
}

void LivraisonStatisticsDialog::setupUi(const QMap<QString, int>& statusData, 
    const QMap<QString, int>& vehicleData, 
    const QMap<QString, double>& avgTimeData)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(80);
    // Vibrant blue gradient for header
    header->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E4ED8, stop:1 #3B82F6);");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(30, 0, 30, 0);

    QLabel* title = new QLabel("Tableau de Bord Logistique");
    title->setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent;");
    hl->addWidget(title);
    hl->addStretch();

    QPushButton* closeBtn = new QPushButton("Fermer");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.2);
            color: white;
            border: 1px solid white;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: rgba(255, 255, 255, 0.4); border: 2px solid white; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    hl->addWidget(closeBtn);

    mainLayout->addWidget(header);

    // Scroll Area for content
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border: none; background: transparent;");
    
    QWidget* content = new QWidget();
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(30, 30, 30, 30);
    cl->setSpacing(30);

    // Metrics Row
    QHBoxLayout* metricsRow = new QHBoxLayout();
    metricsRow->setSpacing(20);

    int total = 0;
    for (int v : statusData.values()) total += v;
    int completed = statusData.value("Livré", 0);
    int pending   = statusData.value("En attente", 0) + statusData.value("En cours", 0);
    int canceled  = statusData.value("Annulé", 0);

    auto makeMetric = [&](const QString& label, int val, const QString& color) {
        QFrame* card = new QFrame();
        // Updated to use blue & white gradients and dynamic hover scale illusion
        card->setStyleSheet(QString(R"(
            QFrame { 
                background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #3B82F6, stop: 1 #1E40AF); 
                border-radius: 18px; 
                border: 1px solid rgba(255, 255, 255, 0.3);
            }
            QFrame:hover { 
                background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #60A5FA, stop: 1 #1D4ED8); 
                border: 1px solid white;
            }
        )"));
        
        card->setFixedHeight(110);
        QVBoxLayout* l = new QVBoxLayout(card);
        l->setContentsMargins(25, 15, 25, 15);
        l->setSpacing(2);

        QLabel* lbl = new QLabel(label);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
        lbl->setStyleSheet("color: rgba(255, 255, 255, 0.8); text-transform: uppercase; letter-spacing: 1px;");
        
        QLabel* valLbl = new QLabel(QString::number(val));
        valLbl->setFont(QFont("Segoe UI", 24, QFont::Bold));
        valLbl->setStyleSheet("color: white; background: transparent;");

        l->addWidget(lbl);
        l->addWidget(valLbl);
        metricsRow->addWidget(card, 1);
    };

    makeMetric("Total Livraisons", total, "#3B82F6");
    makeMetric("Terminées", completed, "#3B82F6");
    makeMetric("En cours", pending, "#3B82F6");
    makeMetric("Annulé", canceled, "#3B82F6");

    cl->addLayout(metricsRow);

    // Charts Row
    QHBoxLayout* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(25);

    // Top Vans (Horizontal Bar Chart)
    QFrame* vanCard = new QFrame();
    vanCard->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0; border-top: 5px solid #2563EB;");
    QVBoxLayout* vl = new QVBoxLayout(vanCard);
    vl->setContentsMargins(25, 25, 25, 25);
    
    QLabel* vanTitle = new QLabel("Livraisons par Véhicule");
    vanTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b; border: none; background: transparent;");
    vl->addWidget(vanTitle);
    
    QList<QPair<QString, double>> sortedData;
    for(auto it = vehicleData.begin(); it != vehicleData.end(); ++it)
        sortedData.append({it.key(), (double)it.value()});
    
    // Sort descending by value (Best vehicle first)
    std::sort(sortedData.begin(), sortedData.end(), [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
        return a.second > b.second;
    });
    
    HorizontalBarChartWidget* chart = new HorizontalBarChartWidget(sortedData);
    vl->addWidget(chart);
    chartsRow->addWidget(vanCard, 3);

    // Trigger animation slightly after the dialog shows
    QTimer::singleShot(200, chart, &HorizontalBarChartWidget::animateTo);

    // Avg Time (List)
    QFrame* timeCard = new QFrame();
    timeCard->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0; border-top: 5px solid #3B82F6;");
    QVBoxLayout* tl = new QVBoxLayout(timeCard);
    tl->setContentsMargins(25, 25, 25, 25);
    
    QLabel* timeTitle = new QLabel("Temps Moyen par Van");
    timeTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b; border: none; background: transparent;");
    tl->addWidget(timeTitle);

    QVBoxLayout* avgLayout = new QVBoxLayout();
    if (avgTimeData.isEmpty()) {
        avgLayout->addWidget(new QLabel("Aucune donnée"), 0, Qt::AlignCenter);
    } else {
        for (auto it = avgTimeData.begin(); it != avgTimeData.end(); ++it) {
            QFrame* itemFrame = new QFrame();
            itemFrame->setStyleSheet(R"(
                QFrame {
                    background-color: #F8FAFC;
                    border: 1px solid #E2E8F0;
                    border-radius: 10px;
                }
                QFrame:hover {
                    background-color: #EFF6FF;
                    border: 1px solid #60A5FA;
                }
            )");
            
            QHBoxLayout* il = new QHBoxLayout(itemFrame);
            il->setContentsMargins(15, 8, 15, 8);
            
            QLabel* name = new QLabel(it.key());
            name->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
            name->setStyleSheet("color: #1E293B; background: transparent; border: none;");
            
            QLabel* time = new QLabel(QString::number(it.value(), 'f', 1) + " min");
            time->setFont(QFont("Segoe UI", 10, QFont::Bold));
            time->setStyleSheet("color: #2563EB; background: transparent; border: none;");
            
            il->addWidget(name);
            il->addStretch();
            il->addWidget(time);
            avgLayout->addWidget(itemFrame);
        }
    }
    avgLayout->addStretch();
    tl->addLayout(avgLayout);
    chartsRow->addWidget(timeCard, 2);

    cl->addLayout(chartsRow);
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

HorizontalBarChartWidget::HorizontalBarChartWidget(const QList<QPair<QString, double>>& data, QWidget* parent)
    : QFrame(parent), m_data(data), m_progress(0.0)
{
    setMinimumHeight(350);
}

void HorizontalBarChartWidget::animateTo()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "progress", this);
    anim->setDuration(1200);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void HorizontalBarChartWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_data.isEmpty()) return;

    int padding = 40;
    int chartWidth = width() - padding * 2;
    int chartHeight = height() - padding * 2;
    
    // Premium proportions
    int barHeight = 30; 
    int spacing = (chartHeight - (m_data.count() * barHeight)) / (m_data.count() + 1);
    if (spacing < 15) spacing = 15;

    double maxVal = 0;
    for (const auto& pair : m_data) if (pair.second > maxVal) maxVal = pair.second;
    if (maxVal == 0) maxVal = 1;

    for (int i = 0; i < m_data.count(); ++i) {
        int y = padding + spacing + i * (barHeight + spacing);
        
        // Track Background (Glassmorphism effect)
        int trackX = padding + 120;
        int trackWidth = chartWidth - 170;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#f1f5f9"));
        painter.drawRoundedRect(trackX, y, trackWidth, barHeight, 15, 15);

        // Bar Fill with Vibrant Gradient
        double targetWidth = (m_data[i].second / maxVal) * trackWidth;
        double barWidth = targetWidth * m_progress;
        
        // Ensure rect has at least a small width so it doesn't break rendering
        if (barWidth < 1) barWidth = 1;
        
        QRectF rect(trackX, y, barWidth, barHeight);
        
        QLinearGradient grad(rect.topLeft(), rect.topRight());
        if (i == 0) {
            grad.setColorAt(0, QColor("#3B82F6")); // Top vehicle - Brighter Blue
            grad.setColorAt(1, QColor("#1D4ED8")); // Dark Royal Blue
        } else {
            grad.setColorAt(0, QColor("#93C5FD")); // Premium Light Blue
            grad.setColorAt(1, QColor("#3B82F6")); // Sky Blue
        }
        
        painter.setBrush(grad);
        painter.drawRoundedRect(rect, 15, 15);

        // Vehicle Name
        painter.setPen(QColor("#475569"));
        painter.setFont(QFont("Segoe UI", 10, QFont::Medium));
        painter.drawText(QRect(padding, y, 110, barHeight), Qt::AlignRight | Qt::AlignVCenter, m_data[i].first);

        // Delivery Count (animating up)
        int currentVal = qRound(m_data[i].second * m_progress);
        painter.setPen(QColor("#1e293b"));
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter.drawText(QRect(trackX + barWidth + 12, y, 50, barHeight), Qt::AlignLeft | Qt::AlignVCenter, QString::number(currentVal));
        
        // Highlight Label for Top Vehicle
        if (i == 0) {
             painter.setPen(QColor("#1E40AF"));
             painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
             painter.drawText(QRect(trackX + trackWidth - 70, y, 65, barHeight), Qt::AlignRight | Qt::AlignVCenter, "TOP ★");
        }
    }
}
