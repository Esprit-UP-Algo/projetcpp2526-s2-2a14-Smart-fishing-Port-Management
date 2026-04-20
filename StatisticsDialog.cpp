#include "StatisticsDialog.h"
#include <QPainter>
#include <QFont>
#include <QPushButton>
#include <QScrollArea>

// ─────────────────────────────────────────
// BarChartWidget
// ─────────────────────────────────────────
BarChartWidget::BarChartWidget(const QString& title,
                               const QMap<QString, int>& data,
                               const QList<QColor>& colors,
                               QWidget* parent)
    : QFrame(parent), chartTitle(title), chartData(data), barColors(colors)
{
    setMinimumSize(400, 320);
    setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 2px solid #e5e7eb;
        }
    )");
}

void BarChartWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int marginLeft   = 60;
    const int marginRight  = 30;
    const int marginTop    = 55;
    const int marginBottom = 80;

    int chartW = width()  - marginLeft - marginRight;
    int chartH = height() - marginTop  - marginBottom;

    // ── Title ──
    painter.setPen(QColor("#1f2937"));
    QFont titleFont("Segoe UI", 13, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 12, width(), 30), Qt::AlignHCenter, chartTitle);

    if (chartData.isEmpty()) return;

    QList<QString> keys   = chartData.keys();
    QList<int>     values = chartData.values();

    int maxVal = *std::max_element(values.begin(), values.end());
    if (maxVal == 0) maxVal = 1;

    int n         = keys.size();
    int barSpacing = chartW / n;
    int barW      = qMax(30, barSpacing - 24);

    // ── Y-axis grid ──
    painter.setPen(QPen(QColor("#f3f4f6"), 1));
    QFont gridFont("Segoe UI", 8);
    painter.setFont(gridFont);
    for (int i = 0; i <= 4; ++i) {
        int y    = marginTop + chartH - (chartH * i / 4);
        int yVal = maxVal * i / 4;
        painter.drawLine(marginLeft, y, marginLeft + chartW, y);
        painter.setPen(QColor("#9ca3af"));
        painter.drawText(QRect(0, y - 8, marginLeft - 6, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(yVal));
        painter.setPen(QPen(QColor("#f3f4f6"), 1));
    }

    // ── Bars ──
    for (int i = 0; i < n; ++i) {
        int barH  = (int)((double)values[i] / maxVal * chartH);
        int x     = marginLeft + i * barSpacing + (barSpacing - barW) / 2;
        int y     = marginTop  + chartH - barH;

        QColor col = (i < barColors.size()) ? barColors[i] : QColor("#5D9CEC");

        // Bar shadow
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 20));
        painter.drawRoundedRect(x + 4, y + 4, barW, barH, 8, 8);

        // Bar body
        QLinearGradient grad(x, y, x, y + barH);
        grad.setColorAt(0, col.lighter(120));
        grad.setColorAt(1, col);
        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(x, y, barW, barH, 8, 8);

        // Value label on top
        painter.setPen(QColor("#1f2937"));
        QFont valFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(valFont);
        painter.drawText(QRect(x, y - 22, barW, 18), Qt::AlignHCenter,
                         QString::number(values[i]));

        // X-axis label
        painter.setPen(QColor("#374151"));
        QFont labelFont("Segoe UI", 9);
        painter.setFont(labelFont);
        QRect labelRect(x - 16, marginTop + chartH + 10, barW + 32, 40);
        painter.drawText(labelRect, Qt::AlignHCenter | Qt::TextWordWrap, keys[i]);
    }

    // ── Axis lines ──
    painter.setPen(QPen(QColor("#d1d5db"), 2));
    painter.drawLine(marginLeft, marginTop, marginLeft, marginTop + chartH);
    painter.drawLine(marginLeft, marginTop + chartH,
                     marginLeft + chartW, marginTop + chartH);
}

// ─────────────────────────────────────────
// StatisticsDialog
// ─────────────────────────────────────────
StatisticsDialog::StatisticsDialog(const QMap<QString, int>& stateData,
                                   const QMap<QString, int>& disponibiliteData,
                                   QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques des Bateaux");
    setMinimumSize(960, 500);
    setStyleSheet("background-color: #F0F4F8;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // ── Header ──
    QLabel* header = new QLabel("📊  Statistiques des Bateaux");
    QFont hFont("Segoe UI", 20, QFont::Bold);
    header->setFont(hFont);
    header->setStyleSheet("color: #2C3E50;");
    mainLayout->addWidget(header);

    // ── Charts row ──
    QHBoxLayout* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(24);

    QList<QColor> stateColors = {
        QColor("#5D9CEC"),   // En mer     – bleu
        QColor("#34C988"),   // Au port    – vert
        QColor("#F6C244")    // En maint.  – jaune
    };
    QList<QColor> dispColors = {
        QColor("#34C988"),   // Oui  – vert
        QColor("#F47B7B")    // Non  – rouge
    };

    BarChartWidget* stateChart =
        new BarChartWidget("Répartition par État", stateData, stateColors);
    BarChartWidget* dispChart  =
        new BarChartWidget("Répartition par Disponibilité", disponibiliteData, dispColors);

    chartsRow->addWidget(stateChart);
    chartsRow->addWidget(dispChart);
    mainLayout->addLayout(chartsRow, 1);

    // ── Summary cards ──
    QHBoxLayout* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(16);

    auto makeCard = [&](const QString& icon, const QString& label,
                        int value, const QString& bgColor, const QString& textColor) {
        QFrame* card = new QFrame();
        card->setStyleSheet(QString(R"(
            QFrame {
                background-color: %1;
                border-radius: 14px;
                padding: 6px;
            }
        )").arg(bgColor));
        card->setFixedHeight(80);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setAlignment(Qt::AlignCenter);
        QLabel* lbl = new QLabel(icon + "  " + label + ": " + QString::number(value));
        QFont lf("Segoe UI", 12, QFont::Bold);
        lbl->setFont(lf);
        lbl->setStyleSheet(QString("color: %1;").arg(textColor));
        lbl->setAlignment(Qt::AlignCenter);
        cl->addWidget(lbl);
        cardsRow->addWidget(card);
    };

    int total = 0;
    for (int v : stateData.values()) total += v;
    makeCard("🚢", "Total bateaux",  total,                     "#DBEAFE", "#1E40AF");
    makeCard("🌊", "En mer",         stateData.value("En mer",0), "#DBEAFE", "#1E40AF");
    makeCard("⚓", "Au port",        stateData.value("Au port",0),"#D1FAE5","#065F46");
    makeCard("🔧", "En maintenance", stateData.value("En maintenance",0),"#FEF3C7","#92400E");
    makeCard("✅", "Disponibles",    disponibiliteData.value("Oui",0),    "#D1FAE5","#065F46");
    makeCard("❌", "Non disponibles",disponibiliteData.value("Non",0),    "#FEE2E2","#991B1B");

    mainLayout->addLayout(cardsRow);

    // ── Close button ──
    QPushButton* closeBtn = new QPushButton("Fermer");
    QFont btnFont("Segoe UI", 11, QFont::Bold);
    closeBtn->setFont(btnFont);
    closeBtn->setFixedHeight(44);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 8px 40px;
        }
        QPushButton:hover { background-color: #4A89DC; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    mainLayout->addLayout(btnRow);
}
