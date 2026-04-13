#ifndef PECHESTATISTICSDIALOG_H
#define PECHESTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QList>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>

class PechePieChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PechePieChartWidget(const QString& title,
                            const QMap<QString, double>& data,
                            QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_title;
    QMap<QString, double> m_data;
    QList<QColor> m_colors;
};

class PecheStatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PecheStatisticsDialog(const QMap<QString, double>& speciesCount,
                                   const QMap<QString, double>& weightBySpecies,
                                   const QMap<QString, double>& avgWeightByBoat,
                                   QWidget* parent = nullptr);

private:
    void setupUi(const QMap<QString, double>& speciesCount,
                 const QMap<QString, double>& weightBySpecies,
                 const QMap<QString, double>& avgWeightByBoat);
};

#endif // PECHESTATISTICSDIALOG_H
