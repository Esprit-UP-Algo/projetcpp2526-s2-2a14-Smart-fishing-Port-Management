#ifndef TEMPERATUREALERT_H
#define TEMPERATUREALERT_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class TemperatureAlert : public QWidget
{
    Q_OBJECT

public:
    explicit TemperatureAlert(const QString& fridgeRef, double threshold, double current, QWidget *parent = nullptr);
    ~TemperatureAlert();

signals:
    void requestNavigation();

private:
    void setupUi(const QString& ref, double threshold, double current);
    QString getStatusMessage(double threshold, double current);

    QLabel* iconLabel;
    QLabel* titleLabel;
    QLabel* messageLabel;
    QLabel* currentTempLabel;
    QLabel* thresholdLabel;
};

#endif // TEMPERATUREALERT_H
