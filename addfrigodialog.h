#ifndef ADDFRIGODIALOG_H
#define ADDFRIGODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class AddFrigoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFrigoDialog(QWidget *parent = nullptr);

    void setData(QString id, QString cap, QString hum,
                 QString temp, QString stat, QString fish);

    QString getId();
    QString getCap();
    QString getHum();
    QString getTemp();
    QString getStatus();
    QString getFish();

private:
    QLineEdit *idEdit;
    QLineEdit *capEdit;
    QLineEdit *humEdit;
    QLineEdit *tempEdit;
    QComboBox *statusBox;
    QComboBox *fishBox;
};

#endif // ADDFRIGODIALOG_H
