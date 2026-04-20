#ifndef PREDICTMAINTENANCEDIALOG_H
#define PREDICTMAINTENANCEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

#include <QDateEdit>

class PredictMaintenanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PredictMaintenanceDialog(int age, int moisMaintenance, int frequence, const QString& etat, QWidget *parent = nullptr);
    ~PredictMaintenanceDialog() override;

    QDate getSelectedDate() const { return m_finalDate; }

private slots:
    void calculateScore();
    void onAcceptBtn();
    void onCustomBtn();
    void onConfirmCustomBtn();

private:
    int m_age;
    int m_moisMaintenance;
    int m_frequence;
    QString m_etat;
    int m_etatVal;
    QDate m_finalDate;
    QDate m_systemPredictedDate;

    QLabel* resultLabel;
    QPushButton* btnAccept;
    QPushButton* btnCustom;
    QWidget* customDateWidget;
    QDateEdit* dateEdit;
    QPushButton* btnConfirmCustom;

    class QSystemTrayIcon* trayIcon;
};

#endif // PREDICTMAINTENANCEDIALOG_H
