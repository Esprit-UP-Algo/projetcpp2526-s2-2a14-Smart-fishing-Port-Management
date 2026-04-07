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
    explicit PredictMaintenanceDialog(int age, int moisMaintenance, const QString& etat, QWidget *parent = nullptr);
    ~PredictMaintenanceDialog() override;

private slots:
    void calculateScore();

private:
    int m_age;
    int m_moisMaintenance;
    QString m_etat;
    int m_etatVal;

    QSpinBox* spinFrequence;
    QLabel* resultLabel;
};

#endif // PREDICTMAINTENANCEDIALOG_H
