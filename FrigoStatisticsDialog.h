#ifndef FRIGOSTATISTICSDIALOG_H
#define FRIGOSTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QList>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>

class PieChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PieChartWidget(const QString& title,
                            const QMap<QString, double>& data,
                            QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_title;
    QMap<QString, double> m_data;
    QList<QColor> m_colors;
};

class FrigoStatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FrigoStatisticsDialog(const QMap<QString, double>& typeCount,
                                   const QMap<QString, double>& typeCapacity,
                                   const QMap<QString, double>& statusCount,
                                   QWidget* parent = nullptr);

private:
    void setupUi(const QMap<QString, double>& typeCount,
                 const QMap<QString, double>& typeCapacity,
                 const QMap<QString, double>& statusCount);
};

#endif // FRIGOSTATISTICSDIALOG_H
