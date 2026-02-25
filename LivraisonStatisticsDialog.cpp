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
#include <QMap>
#include <QFrame>
#include <QList>
#include <QPair>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QEasingCurve>
#include <QColor>
#include <algorithm>

LivraisonStatisticsDialog::LivraisonStatisticsDialog(
    const QMap<QString, int>& statusData, 
    const QMap<QString, int>& vehicleData,
    const QMap<QString, double>& avgTimeData,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques de Livraison");
    setMinimumSize(1000, 750);
    setStyleSheet("background-color: #f8fafc;");
    setupUi(statusData, vehicleData, avgTimeData);
}

void LivraisonStatisticsDialog::setupUi(const QMap<QString, int>& statusData, 
    const QMap<QString, int>& vehicleData, 
    const QMap<QString, double>& avgTimeData)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header with Gradient
    QFrame* headerFrame = new QFrame();
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1E3A8A, stop:1 #3B82F6);
        }
    )");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(35, 0, 35, 0);

    QLabel* titleLbl = new QLabel("📊  Tableau de Bord Logistique");
    titleLbl->setStyleSheet("color: white; font-size: 26px; font-weight: bold; font-family: 'Segoe UI';");
    headerLayout->addWidget(titleLbl);
    headerLayout->addStretch();

    QPushButton* closeBtn = new QPushButton("✕  Fermer");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(130, 44);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.15);
            color: white;
            border: 1px solid rgba(255, 255, 255, 0.3);
            border-radius: 12px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: rgba(255, 255, 255, 0.25); border: 1px solid white; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(headerFrame);

    // Scroll Area for content
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    QWidget* contentWidget = new QWidget();
    contentWidget->setObjectName("contentWidget");
    contentWidget->setStyleSheet("#contentWidget { background-color: #f8fafc; }");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(35, 35, 35, 35);
    contentLayout->setSpacing(35);

    // Metrics Row
    QHBoxLayout* metricsRow = new QHBoxLayout();
    metricsRow->setSpacing(24);

    int totalDeliv = 0;
    for (int v : statusData.values()) totalDeliv += v;
    int completedNum = statusData.value("Livré", 0);
    int pendingNum   = statusData.value("En attente", 0) + statusData.value("En cours", 0);
    int canceledNum  = statusData.value("Annulé", 0);

    QList<QFrame*> animatedCards;
    auto makeMetric = [&](const QString& icon, const QString& label, int val, const QString& color1, const QString& color2) {
        QFrame* cardItem = new QFrame();
        cardItem->setMinimumHeight(125);
        cardItem->setStyleSheet(QString(R"(
            QFrame { 
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);
                border-radius: 20px; 
            }
        )").arg(color1, color2));
        
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
        shadowEffect->setBlurRadius(20);
        shadowEffect->setOffset(0, 8);
        shadowEffect->setColor(QColor(0, 0, 0, 40));
        cardItem->setGraphicsEffect(shadowEffect);

        QVBoxLayout* cardLayout = new QVBoxLayout(cardItem);
        cardLayout->setContentsMargins(25, 20, 25, 20);
        cardLayout->setSpacing(4);

        QHBoxLayout* iconHeaderLayout = new QHBoxLayout();
        QLabel* iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 24px; background: transparent;");
        iconHeaderLayout->addWidget(iconLabel);
        iconHeaderLayout->addStretch();
        
        QLabel* labelLabel = new QLabel(label);
        labelLabel->setFont(QFont("Segoe UI", 9, QFont::Bold));
        labelLabel->setStyleSheet("color: rgba(255, 255, 255, 0.9); text-transform: uppercase; letter-spacing: 1px;");
        iconHeaderLayout->addWidget(labelLabel);
        cardLayout->addLayout(iconHeaderLayout);
        
        QLabel* valueLabel = new QLabel(QString::number(val));
        valueLabel->setFont(QFont("Segoe UI", 28, QFont::Bold));
        valueLabel->setStyleSheet("color: white;");
        cardLayout->addWidget(valueLabel);
        
        metricsRow->addWidget(cardItem, 1);
        animatedCards.append(cardItem);
        
        // Initial state for animation
        cardItem->setWindowOpacity(0);
    };

    makeMetric("📦", "Total Livraisons", totalDeliv, "#1E3A8A", "#3B82F6");
    makeMetric("✅", "Terminées", completedNum, "#2563EB", "#60A5FA");
    makeMetric("⏳", "En attente", pendingNum, "#3B82F6", "#93C5FD");
    makeMetric("❌", "Annulées", canceledNum, "#64748B", "#94A3B8");

    contentLayout->addLayout(metricsRow);

    // Charts Row
    QHBoxLayout* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(30);

    // Top Vans Card
    QFrame* vanCard = new QFrame();
    vanCard->setObjectName("vanCard");
    vanCard->setStyleSheet(R"(
        #vanCard { 
            background: white; 
            border-radius: 24px; 
            border: 1px solid #e2e8f0;
        }
    )");
    
    QGraphicsDropShadowEffect* vanShadow = new QGraphicsDropShadowEffect();
    vanShadow->setBlurRadius(25);
    vanShadow->setOffset(0, 10);
    vanShadow->setColor(QColor(0, 0, 0, 30));
    vanCard->setGraphicsEffect(vanShadow);
    
    QVBoxLayout* vanLayout = new QVBoxLayout(vanCard);
    vanLayout->setContentsMargins(30, 30, 30, 30);
    
    QLabel* vanTitle = new QLabel("Performance par Véhicule");
    vanTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; margin-bottom: 20px;");
    vanLayout->addWidget(vanTitle);
    
    QList<QPair<QString, double>> sortedData;
    for(auto it = vehicleData.begin(); it != vehicleData.end(); ++it)
        sortedData.append({it.key(), (double)it.value()});
    
    std::sort(sortedData.begin(), sortedData.end(), [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
        return a.second > b.second;
    });
    
    HorizontalBarChartWidget* barChart = new HorizontalBarChartWidget(sortedData);
    vanLayout->addWidget(barChart);
    chartsRow->addWidget(vanCard, 3);

    // Avg Time Card
    QFrame* timeCard = new QFrame();
    timeCard->setObjectName("timeCard");
    timeCard->setStyleSheet(R"(
        #timeCard { 
            background: white; 
            border-radius: 24px; 
            border: 1px solid #e2e8f0;
        }
    )");
    
    QGraphicsDropShadowEffect* timeShadow = new QGraphicsDropShadowEffect();
    timeShadow->setBlurRadius(25);
    timeShadow->setOffset(0, 10);
    timeShadow->setColor(QColor(0, 0, 0, 30));
    timeCard->setGraphicsEffect(timeShadow);
    
    QVBoxLayout* timeLayout = new QVBoxLayout(timeCard);
    timeLayout->setContentsMargins(30, 30, 30, 30);
    
    QLabel* timeTitle = new QLabel("Temps Moyen (min)");
    timeTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; margin-bottom: 20px;");
    timeLayout->addWidget(timeTitle);

    QVBoxLayout* avgLayout = new QVBoxLayout();
    avgLayout->setSpacing(12);
    
    if (avgTimeData.isEmpty()) {
        avgLayout->addWidget(new QLabel("Aucune donnée disponible"), 0, Qt::AlignCenter);
    } else {
        for (auto it = avgTimeData.begin(); it != avgTimeData.end(); ++it) {
            QFrame* itemFrame = new QFrame();
            itemFrame->setStyleSheet(R"(
                QFrame {
                    background-color: #f8fafc;
                    border: 1px solid #e2e8f0;
                    border-radius: 12px;
                }
                QFrame:hover {
                    background-color: #f1f5f9;
                    border: 1px solid #cbd5e1;
                }
            )");
            
            QHBoxLayout* itemLayout = new QHBoxLayout(itemFrame);
            itemLayout->setContentsMargins(18, 12, 18, 12);
            
            QLabel* nameLbl = new QLabel(it.key());
            nameLbl->setStyleSheet("color: #475569; font-weight: 600; font-size: 14px;");
            
            QLabel* timeValLbl = new QLabel(QString::number(it.value(), 'f', 1));
            timeValLbl->setStyleSheet("color: #2563EB; font-weight: 800; font-size: 16px;");
            
            itemLayout->addWidget(nameLbl);
            itemLayout->addStretch();
            itemLayout->addWidget(timeValLbl);
            avgLayout->addWidget(itemFrame);
        }
    }
    avgLayout->addStretch();
    timeLayout->addLayout(avgLayout);
    chartsRow->addWidget(timeCard, 2);

    contentLayout->addLayout(chartsRow);
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // --- STAGGERED ENTRANCE ANIMATIONS ---
    for (int idx = 0; idx < animatedCards.size(); ++idx) {
        QFrame* card = animatedCards[idx];
        QTimer::singleShot(150 * idx, [card]() {
            QPropertyAnimation* anim = new QPropertyAnimation(card, "pos");
            anim->setDuration(500);
            QPoint originalPos = card->pos();
            anim->setStartValue(originalPos + QPoint(0, 30));
            anim->setEndValue(originalPos);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            
            QPropertyAnimation* fade = new QPropertyAnimation(card, "windowOpacity");
            fade->setDuration(500);
            fade->setStartValue(0.0);
            fade->setEndValue(1.0);
            
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            fade->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    // Trigger bar chart animation
    QTimer::singleShot(600, [barChart]() { barChart->animate(); });
}

HorizontalBarChartWidget::HorizontalBarChartWidget(const QList<QPair<QString, double>>& data, QWidget* parent)
    : QFrame(parent), m_data(data), m_barProgress(0.0)
{
    setMinimumHeight(400);
    setStyleSheet("background: transparent; border: none;");
}

void HorizontalBarChartWidget::animate()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "barProgress");
    anim->setDuration(1500);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutExpo);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void HorizontalBarChartWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_data.isEmpty()) return;

    int paddingLeft = 140;
    int paddingRight = 60;
    int paddingTop = 10;
    int chartWidth = width() - paddingLeft - paddingRight;
    int chartHeight = height() - paddingTop * 2;
    
    int barHeight = 32; 
    int barSpacing = (chartHeight - (m_data.count() * barHeight)) / (m_data.count() + 1);
    if (barSpacing < 10) barSpacing = 10;

    double maxValue = 0;
    for (const auto& pair : m_data) if (pair.second > maxValue) maxValue = pair.second;
    if (maxValue == 0) maxValue = 1;

    for (int idx = 0; idx < m_data.count(); ++idx) {
        int yPos = paddingTop + barSpacing + idx * (barHeight + barSpacing);
        
        // Track Background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#f1f5f9"));
        painter.drawRoundedRect(paddingLeft, yPos, chartWidth, barHeight, 10, 10);

        // Animated Bar Fill
        double actualWidth = (m_data[idx].second / maxValue) * chartWidth;
        double animatedWidth = actualWidth * m_barProgress;
        
        QRectF barRect(paddingLeft, yPos, animatedWidth, barHeight);
        
        QLinearGradient barGrad(barRect.topLeft(), barRect.topRight());
        if (idx == 0) {
            barGrad.setColorAt(0, QColor("#1E40AF")); // Sapphire Blue for #1
            barGrad.setColorAt(1, QColor("#3B82F6"));
        } else if (idx == 1) {
            barGrad.setColorAt(0, QColor("#3B82F6")); // Medium Blue for #2
            barGrad.setColorAt(1, QColor("#60A5FA"));
        } else {
            barGrad.setColorAt(0, QColor("#60A5FA")); // Light Sky for others
            barGrad.setColorAt(1, QColor("#93C5FD"));
        }
        
        painter.setBrush(barGrad);
        painter.drawRoundedRect(barRect, 10, 10);

        // Name
        painter.setPen(QColor("#475569"));
        painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        painter.drawText(QRect(0, yPos, paddingLeft - 15, barHeight), Qt::AlignRight | Qt::AlignVCenter, m_data[idx].first);

        // Count (fade in with progress)
        if (m_barProgress > 0.5) {
            painter.setOpacity((m_barProgress - 0.5) * 2);
            painter.setPen(QColor("#1e293b"));
            painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
            painter.drawText(QRect(paddingLeft + animatedWidth + 10, yPos, 50, barHeight), Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_data[idx].second));
            painter.setOpacity(1.0);
        }
    }
}
