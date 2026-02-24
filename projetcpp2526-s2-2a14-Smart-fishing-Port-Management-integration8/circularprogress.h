#ifndef CIRCULARPROGRESS_H
#define CIRCULARPROGRESS_H

#include <QWidget>
#include <QColor>
#include <QString>

class CircularProgress : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double progress READ progress WRITE setProgress)
public:
    CircularProgress(double value, double maxVal, const QString& label,
                     const QString& unit, const QColor& color, QWidget* parent = nullptr);

    double progress() const;
    void setProgress(double p);
    void animateTo();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    double m_progress, m_target, m_value;
    QString m_label, m_unit;
    QColor m_color;
};

#endif // CIRCULARPROGRESS_H
