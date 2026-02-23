#ifndef ADDFRIGODIALOG_H
#define ADDFRIGODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include "Frigowindow.h"

class AddFrigoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFrigoDialog(QWidget *parent = nullptr, Frigo* frigoData = nullptr);
    ~AddFrigoDialog();

    Frigo getData() const;

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
    QLineEdit* humEdit;
    QLineEdit* tempEdit;
    QComboBox* statusBox;
    QComboBox* fishBox;

    Frigo* frigoData;
    bool isEdit;
};

#endif // ADDFRIGODIALOG_H
