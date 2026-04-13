#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

class BarChartWidget : public QFrame
{
    Q_OBJECT
public:
    explicit BarChartWidget(const QString& title,
                            const QMap<QString, int>& data,
                            const QList<QColor>& colors,
                            QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString chartTitle;
    QMap<QString, int> chartData;
    QList<QColor> barColors;
};

class StatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StatisticsDialog(const QMap<QString, int>& stateData,
                              const QMap<QString, int>& disponibiliteData,
                              QWidget* parent = nullptr);
};

#endif // STATISTICSDIALOG_H
