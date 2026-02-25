#ifndef LIVRAISONSTATISTICSDIALOG_H
#define LIVRAISONSTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QFrame>
#include <QList>
#include <QPair>
#include <QWidget>

class HorizontalBarChartWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(double barProgress READ barProgress WRITE setBarProgress)

public:
    explicit HorizontalBarChartWidget(const QList<QPair<QString, double>>& data, QWidget* parent = nullptr);
    
    double barProgress() const { return m_barProgress; }
    void setBarProgress(double progress) { m_barProgress = progress; update(); }
    void animate();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<QPair<QString, double>> m_data;
    double m_barProgress;
};

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

#endif // LIVRAISONSTATISTICSDIALOG_H
