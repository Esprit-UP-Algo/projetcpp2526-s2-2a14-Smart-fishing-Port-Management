#ifndef BATEAUSTATISTICSDIALOG_H
#define BATEAUSTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QList>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>

class BateauPieChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BateauPieChartWidget(const QString& title,
                            const QMap<QString, double>& data,
                            QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_title;
    QMap<QString, double> m_data;
    QList<QColor> m_colors;
};

class BateauStatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BateauStatisticsDialog(const QMap<QString, double>& availabilityData,
                                   int totalBateaux,
                                   double totalCapacite,
                                   double avgAge,
                                   QWidget* parent = nullptr);

private:
    void setupUi(const QMap<QString, double>& availabilityData,
                 int totalBateaux,
                 double totalCapacite,
                 double avgAge);
};

#endif // BATEAUSTATISTICSDIALOG_H
