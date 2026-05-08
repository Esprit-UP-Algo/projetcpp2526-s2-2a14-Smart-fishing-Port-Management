#ifndef TEMPERATUREALERT_H
#define TEMPERATUREALERT_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMap>
#include <QStringList>

class TemperatureAlert : public QWidget
{
    Q_OBJECT

public:
    explicit TemperatureAlert(QWidget *parent = nullptr);
    ~TemperatureAlert();

    void addOrUpdateFridge(const QString& ref, double threshold, double current);
    void removeFridge(const QString& ref);
    QStringList activeFridges() const { return fridgeRows.keys(); }

signals:
    void requestNavigation();
    void fridgesDismissed(const QStringList& refs);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();
    QWidget* createFridgeRow(const QString& ref, double threshold, double current);

    QVBoxLayout* mainLayout;
    QVBoxLayout* contentLayout;
    QLabel* messageLabel;
    
    struct FridgeWidgets {
        QWidget* container;
        QLabel* currentLabel;
        QLabel* thresholdLabel;
    };
    
    QMap<QString, FridgeWidgets> fridgeRows;
};

#endif // TEMPERATUREALERT_H
