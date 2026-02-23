#include "LivraisonStatisticsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QPainter>
#include <QLinearGradient>
#include <QPaintEvent>

LivraisonStatisticsDialog::LivraisonStatisticsDialog(
    const QMap<QString, int>& statusData, 
    const QMap<QString, int>& vehicleData,
    const QMap<QString, double>& avgTimeData,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques de Livraison");
    setMinimumSize(900, 700);
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

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(80);
    header->setStyleSheet("background-color: #5D9CEC;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(30, 0, 30, 0);

    QLabel* title = new QLabel("Tableau de Bord Logistique");
    title->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
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
        QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }
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
        card->setStyleSheet(QString(R"(
            QFrame { 
                background: %1; 
                border-radius: 18px; 
                border: none;
            }
            QFrame:hover { 
                background: %2; 
            }
        )").arg("#5D9CEC", "#4A89DC"));
        
        card->setFixedHeight(110);
        QVBoxLayout* l = new QVBoxLayout(card);
        l->setContentsMargins(25, 15, 25, 15);
        l->setSpacing(2);

        QLabel* lbl = new QLabel(label);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
        lbl->setStyleSheet("color: rgba(255, 255, 255, 0.8); text-transform: uppercase; letter-spacing: 1px;");
        
        QLabel* valLbl = new QLabel(QString::number(val));
        valLbl->setFont(QFont("Segoe UI", 24, QFont::Bold));
        valLbl->setStyleSheet("color: white;");

        l->addWidget(lbl);
        l->addWidget(valLbl);
        metricsRow->addWidget(card, 1);
    };

    makeMetric("Total Livraisons", total, "#5D9CEC");
    makeMetric("Terminées", completed, "#5D9CEC");
    makeMetric("En cours", pending, "#5D9CEC");
    makeMetric("Annulé", canceled, "#5D9CEC");

    cl->addLayout(metricsRow);

    // Charts Row
    QHBoxLayout* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(25);

    // Top Vans (Horizontal Bar Chart)
    QFrame* vanCard = new QFrame();
    vanCard->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    QVBoxLayout* vl = new QVBoxLayout(vanCard);
    vl->setContentsMargins(25, 25, 25, 25);
    
    QLabel* vanTitle = new QLabel("Livraisons par Véhicule");
    vanTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b; border: none;");
    vl->addWidget(vanTitle);
    
    QMap<QString, double> chartData;
    for(auto it = vehicleData.begin(); it != vehicleData.end(); ++it)
        chartData[it.key()] = it.value();
    
    vl->addWidget(new HorizontalBarChartWidget(chartData));
    chartsRow->addWidget(vanCard, 3);

    // Avg Time (List)
    QFrame* timeCard = new QFrame();
    timeCard->setStyleSheet("background: white; border-radius: 16px; border: 1px solid #e2e8f0;");
    QVBoxLayout* tl = new QVBoxLayout(timeCard);
    tl->setContentsMargins(25, 25, 25, 25);
    
    QLabel* timeTitle = new QLabel("Temps Moyen par Van");
    timeTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b; border: none;");
    tl->addWidget(timeTitle);

    QVBoxLayout* avgLayout = new QVBoxLayout();
    if (avgTimeData.isEmpty()) {
        avgLayout->addWidget(new QLabel("Aucune donnée"), 0, Qt::AlignCenter);
    } else {
        for (auto it = avgTimeData.begin(); it != avgTimeData.end(); ++it) {
            QFrame* itemFrame = new QFrame();
            itemFrame->setStyleSheet(R"(
                QFrame {
                    background-color: #F0F7FF;
                    border: 1px solid #C4E0FF;
                    border-radius: 10px;
                }
                QFrame:hover {
                    background-color: #E1EFFF;
                    border: 1px solid #5D9CEC;
                }
            )");
            
            QHBoxLayout* il = new QHBoxLayout(itemFrame);
            il->setContentsMargins(15, 8, 15, 8);
            
            QLabel* name = new QLabel(it.key());
            name->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
            name->setStyleSheet("color: #2C3E50;");
            
            QLabel* time = new QLabel(QString::number(it.value(), 'f', 1) + " min");
            time->setFont(QFont("Segoe UI", 10, QFont::Bold));
            time->setStyleSheet("color: #5D9CEC;");
            
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

HorizontalBarChartWidget::HorizontalBarChartWidget(const QMap<QString, double>& data, QWidget* parent)
    : QFrame(parent), m_data(data)
{
    setMinimumHeight(350);
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
    int barHeight = 25;
    int spacing = (chartHeight - (m_data.count() * barHeight)) / (m_data.count() + 1);

    double maxVal = 0;
    for (double val : m_data.values()) if (val > maxVal) maxVal = val;
    if (maxVal == 0) maxVal = 1;

    int i = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        int y = padding + spacing + i * (barHeight + spacing);
        double barWidth = (it.value() / maxVal) * (chartWidth - 150);

        // Bar
        QRectF rect(padding + 120, y, barWidth, barHeight);
        QLinearGradient grad(rect.topLeft(), rect.topRight());
        grad.setColorAt(0, QColor("#5D9CEC"));
        grad.setColorAt(1, QColor("#4A89DC"));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawRoundedRect(rect, 6, 6);

        // Name
        painter.setPen(QColor("#475569"));
        painter.setFont(QFont("Segoe UI", 9, QFont::Medium));
        painter.drawText(QRect(padding, y, 110, barHeight), Qt::AlignRight | Qt::AlignVCenter, it.key());

        // Value
        painter.setPen(QColor("#1e293b"));
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.drawText(QRect(padding + 130 + barWidth, y, 50, barHeight), Qt::AlignLeft | Qt::AlignVCenter, QString::number(it.value()));

        i++;
    }
}
