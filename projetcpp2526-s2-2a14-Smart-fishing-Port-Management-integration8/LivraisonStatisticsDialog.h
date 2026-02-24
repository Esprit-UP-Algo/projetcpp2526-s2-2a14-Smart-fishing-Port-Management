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
public:
    explicit HorizontalBarChartWidget(const QList<QPair<QString, double>>& data, QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QList<QPair<QString, double>> m_data;
};

#endif // LIVRAISONSTATISTICSDIALOG_H
