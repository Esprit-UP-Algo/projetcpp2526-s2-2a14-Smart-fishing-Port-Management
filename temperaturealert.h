#ifndef TEMPERATUREALERT_H
#define TEMPERATUREALERT_H

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class TemperatureAlert : public QDialog
{
    Q_OBJECT

public:
    explicit TemperatureAlert(const QString& fridgeRef, double threshold, double current, QWidget *parent = nullptr);
    ~TemperatureAlert();

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
