#include "BateauStatisticsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

// ─────────────────────────────────────────
// BateauPieChartWidget
// ─────────────────────────────────────────
BateauPieChartWidget::BateauPieChartWidget(const QString& title, const QMap<QString, double>& data, QWidget* parent)
    : QWidget(parent), m_title(title), m_data(data)
{
    setMinimumSize(400, 350);
    m_colors = {
        QColor("#34C988"),   // Green (Disponible)
        QColor("#F47B7B"),   // Red (Occupé)
        QColor("#5D9CEC"),   // Blue
        QColor("#F6C244"),   // Yellow
        QColor("#9966FF"),   // Purple
    };
}

void BateauPieChartWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Title
    painter.setPen(QColor("#1f2937"));
    QFont titleFont("Segoe UI", 12, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 15, width(), 30), Qt::AlignHCenter, m_title);

    if (m_data.isEmpty()) {
        painter.setPen(QColor("#6b7280"));
        painter.setFont(QFont("Segoe UI", 10));
        painter.drawText(rect(), Qt::AlignCenter, "Aucune donnée");
        return;
    }

    double total = 0;
    for (double v : m_data.values()) total += v;
    if (total == 0) return;

    // Dimensions
    int pieSize = qMin(width() - 40, height() - 120);
    int pieX = (width() - pieSize) / 2;
    int pieY = 55;
    QRect pieRect(pieX, pieY, pieSize, pieSize);

    // Sectors
    int startAngle = 0;
    int i = 0;
    QList<QString> keys = m_data.keys();

    for (const QString& key : keys) {
        double value = m_data[key];
        int spanAngle = static_cast<int>((value / total) * 360 * 16.0);

        QColor color = m_colors[i % m_colors.size()];
        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 2));

        painter.drawPie(pieRect, startAngle, spanAngle);
        startAngle += spanAngle;
        i++;
    }

    // Legend
    int legendY = pieRect.bottom() + 25;
    int currentX = pieX;
    int currentY = legendY;
    i = 0;

    for (const QString& key : keys) {
        double value = m_data[key];
        double percent = (value / total) * 100.0;
        QString text = QString("%1 (%2%)").arg(key).arg(percent, 0, 'f', 1);

        painter.setBrush(m_colors[i % m_colors.size()]);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(currentX, currentY, 14, 14, 3, 3);

        painter.setPen(QColor("#475569"));
        painter.setFont(QFont("Segoe UI", 9, QFont::Medium));
        int textW = painter.fontMetrics().horizontalAdvance(text) + 20;
        painter.drawText(currentX + 22, currentY + 12, text);

        currentX += textW + 20;
        if (currentX > width() - 80) {
            currentX = pieX;
            currentY += 22;
        }
        i++;
    }
}

// ─────────────────────────────────────────
// BateauStatisticsDialog
// ─────────────────────────────────────────
BateauStatisticsDialog::BateauStatisticsDialog(const QMap<QString, double>& availabilityData,
                                               int totalBateaux,
                                               double totalCapacite,
                                               double avgAge,
                                               QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques de la Flotte");
    setMinimumSize(950, 600);
    setStyleSheet("QDialog { background-color: #F0F4F8; }");
    setupUi(availabilityData, totalBateaux, totalCapacite, avgAge);
}

void BateauStatisticsDialog::setupUi(const QMap<QString, double>& availabilityData,
                                    int totalBateaux,
                                    double totalCapacite,
                                    double avgAge)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(80);
    header->setStyleSheet("background-color: #1e4fa3;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(30, 0, 30, 0);

    QLabel* title = new QLabel("📊 Statistiques de la Flotte");
    title->setStyleSheet("color: white; font-size: 24px; font-weight: bold; font-family: 'Segoe UI';");
    hl->addWidget(title);
    hl->addStretch();

    QPushButton* closeBtn = new QPushButton("Fermer");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.2);
            color: white;
            border: 1px solid white;
            border-radius: 10px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    hl->addWidget(closeBtn);

    mainLayout->addWidget(header);

    // Scroll Area
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border: none; background: transparent;");

    QWidget* content = new QWidget();
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(35, 30, 35, 30);
    cl->setSpacing(30);

    // Metrics Row
    QHBoxLayout* metricsRow = new QHBoxLayout();
    metricsRow->setSpacing(20);

    auto makeMetric = [&](const QString& icon, const QString& label, const QString& val, const QString& bgColor, const QString& txtColor) {
        QFrame* card = new QFrame();
        card->setStyleSheet(QString(R"(
            QFrame { 
                background: %1; 
                border-radius: 16px; 
            }
        )").arg(bgColor));
        card->setFixedHeight(110);
        
        QVBoxLayout* l = new QVBoxLayout(card);
        l->setContentsMargins(20, 15, 20, 15);
        
        QLabel* labelW = new QLabel(QString("%1  %2").arg(icon, label));
        labelW->setFont(QFont("Segoe UI", 10, QFont::Bold));
        labelW->setStyleSheet(QString("color: %1; opacity: 0.9; text-transform: uppercase;").arg(txtColor));
        
        QLabel* valW = new QLabel(val);
        valW->setFont(QFont("Segoe UI", 24, QFont::Bold));
        valW->setStyleSheet(QString("color: %1;").arg(txtColor));

        l->addWidget(labelW);
        l->addWidget(valW);
        metricsRow->addWidget(card, 1);
    };

    makeMetric("⛵", "Total Bateaux", QString::number(totalBateaux), "white", "#1e3a5f");
    makeMetric("⚖️", "Capacité Totale", QString::number(totalCapacite, 'f', 1) + " T", "white", "#1e3a5f");
    makeMetric("📅", "Âge Moyen", QString::number(avgAge, 'f', 1) + " ans", "#DBEAFE", "#1E40AF");

    cl->addLayout(metricsRow);

    // Charts Container
    QFrame* chartFrame = new QFrame();
    chartFrame->setStyleSheet("background: white; border-radius: 16px;");
    QVBoxLayout* fl = new QVBoxLayout(chartFrame);
    fl->setContentsMargins(20, 20, 20, 20);
    
    BateauPieChartWidget* chartAvail = new BateauPieChartWidget("Disponibilité de la Flotte", availabilityData);
    fl->addWidget(chartAvail);

    cl->addWidget(chartFrame);
    cl->addStretch();

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}
