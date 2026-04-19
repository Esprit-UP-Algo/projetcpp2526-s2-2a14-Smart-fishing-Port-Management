#ifndef LIVRAISONSTATISTICSDIALOG_H
#define LIVRAISONSTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QFrame>

class LivraisonStatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LivraisonStatisticsDialog(const QMap<QString, int>& statusData, 
        const QMap<QString, int>& vehicleData,
        const QMap<QString, double>& avgTimeData,
        QWidget *parent = nullptr);

private:
    void setupUi(const QMap<QString, int>& statusData, 
                 const QMap<QString, int>& vehicleData, 
                 const QMap<QString, double>& avgTimeData);
};

class HorizontalBarChartWidget : public QFrame {
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
public:
    explicit HorizontalBarChartWidget(const QList<QPair<QString, double>>& data, QWidget* parent = nullptr);
    void animateTo();

    qreal progress() const { return m_progress; }
    void setProgress(qreal p) { m_progress = p; update(); }
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QList<QPair<QString, double>> m_data;
    qreal m_progress = 0.0;
};

#endif // LIVRAISONSTATISTICSDIALOG_H
