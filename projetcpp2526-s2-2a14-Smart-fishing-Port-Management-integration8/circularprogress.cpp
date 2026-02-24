#include "circularprogress.h"
#include <QPainter>
#include <QFont>
#include <QRect>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QPropertyAnimation>

CircularProgress::CircularProgress(double value, double maxVal, const QString& label,
                                   const QString& unit, const QColor& color, QWidget* parent)
    : QWidget(parent), m_progress(0), m_target(maxVal > 0 ? value / maxVal : 0),
    m_value(value), m_label(label), m_unit(unit), m_color(color)
{
    setFixedSize(160, 180);
}

double CircularProgress::progress() const { return m_progress; }
void CircularProgress::setProgress(double p) { m_progress = p; update(); }

void CircularProgress::animateTo() {
    QPropertyAnimation* anim = new QPropertyAnimation(this, "progress");
    anim->setDuration(1200);
    anim->setStartValue(0.0);
    anim->setEndValue(m_target);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CircularProgress::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int cx = width() / 2;
    int cy = height() / 2 - 15;
    int radius = 58;
    int thickness = 12;

    // Background track
    p.setPen(QPen(QColor(230, 235, 245), thickness, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(cx - radius, cy - radius, radius * 2, radius * 2,
              90 * 16, -360 * 16);

    // Progress arc
    if (m_progress > 0) {
        QConicalGradient grad(cx, cy, 90);
        grad.setColorAt(0, m_color.lighter(130));
        grad.setColorAt(1, m_color);
        p.setPen(QPen(QBrush(grad), thickness, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(cx - radius, cy - radius, radius * 2, radius * 2,
                  90 * 16, -static_cast<int>(m_progress * 360 * 16));
    }

    // Center glow circle
    QRadialGradient glow(cx, cy, radius - thickness - 4);
    glow.setColorAt(0, QColor(255, 255, 255, 200));
    glow.setColorAt(1, m_color.lighter(180));
    p.setBrush(glow);
    p.setPen(Qt::NoPen);
    p.drawEllipse(cx - (radius - thickness - 4),
                  cy - (radius - thickness - 4),
                  (radius - thickness - 4) * 2,
                  (radius - thickness - 4) * 2);

    // Value text
    p.setPen(m_color.darker(130));
    QString valStr;
    if (m_unit == "%")
        valStr = QString::number(static_cast<int>(m_progress * m_value / (m_target > 0 ? m_target : 1))) + "%";
    else
        valStr = QString::number(static_cast<int>(m_value));

    // Recompute display value based on animation progress
    double displayVal = (m_target > 0) ? m_progress / m_target * m_value : 0;
    if (m_unit == "%")
        valStr = QString::number(static_cast<int>(m_progress * 100)) + "%";
    else if (m_unit == "DT")
        valStr = QString::number(static_cast<int>(displayVal));
    else
        valStr = QString::number(static_cast<int>(displayVal));

    QFont valFont("Segoe UI", 16, QFont::Bold);
    p.setFont(valFont);
    QRect valRect(cx - radius + thickness, cy - 14, (radius - thickness) * 2, 28);
    p.drawText(valRect, Qt::AlignCenter, valStr);

    if (!m_unit.isEmpty() && m_unit != "%") {
        QFont unitFont("Segoe UI", 8);
        p.setFont(unitFont);
        p.setPen(QColor(150, 160, 180));
        QRect unitRect(cx - radius + thickness, cy + 10, (radius - thickness) * 2, 16);
        p.drawText(unitRect, Qt::AlignCenter, m_unit);
    }

    // Label below
    QFont labelFont("Segoe UI", 9, QFont::Medium);
    p.setFont(labelFont);
    p.setPen(QColor(80, 90, 110));
    QRect labelRect(0, height() - 36, width(), 36);
    p.drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap, m_label);
}
