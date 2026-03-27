#ifndef ADDFRIGODIALOG_H
#define ADDFRIGODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include "frigo.h"

class AddFrigoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFrigoDialog(QWidget *parent = nullptr, FrigoModel* frigoData = nullptr);
    ~AddFrigoDialog();

    FrigoModel getData() const;

    // Méthodes conservées pour compatibilité
    void setData(QString id, QString cap, QString hum,
                 QString temp, QString stat, QString fish);
    QString getId();
    QString getCap();
    QString getHum();
    QString getTemp();
    QString getStatus();
    QString getFish();

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;

    QLineEdit* refEdit;
    QLineEdit* capEdit;
    QLineEdit* tempEdit;
    QLineEdit* occEdit;
    QComboBox* statusBox;
    QComboBox* fishBox;
    QDateEdit* dateResEdit;

    FrigoModel* frigoData;
    bool isEdit;
};

#endif // ADDFRIGODIALOG_H
