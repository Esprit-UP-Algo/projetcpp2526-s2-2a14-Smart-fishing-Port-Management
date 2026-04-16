#ifndef PREDICTMAINTENANCEDIALOG_H
#define PREDICTMAINTENANCEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

class PredictMaintenanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PredictMaintenanceDialog(int age, int moisMaintenance, int frequence, const QString& etat, QWidget *parent = nullptr);
    ~PredictMaintenanceDialog() override;

private slots:
    void calculateScore();

private:
    int m_age;
    int m_moisMaintenance;
    int m_frequence;
    QString m_etat;
    int m_etatVal;

    QLabel* resultLabel;
    class QSystemTrayIcon* trayIcon;
};

#endif // PREDICTMAINTENANCEDIALOG_H
