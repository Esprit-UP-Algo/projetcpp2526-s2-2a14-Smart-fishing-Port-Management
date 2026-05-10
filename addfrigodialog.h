#ifndef ADDFRIGODIALOG_H
#define ADDFRIGODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
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
private slots:
    void onSave();

private:
    void setupUi();
    void populateFields();
    bool validateInputs();
    void updateFieldStyle(QWidget* field, bool isValid);
    QString getInputStyle() const;

    QLineEdit* refEdit;
    QLineEdit* capEdit;
    QLineEdit* tempEdit;
    QLineEdit* occEdit;
    QComboBox* statusBox;
    QComboBox* fishBox;
    QComboBox* phoneBox;
    QDateEdit* dateResEdit;

    // Error labels
    QLabel* refError;
    QLabel* capError;
    QLabel* tempError;
    QLabel* occError;

    FrigoModel* frigoData;
    bool isEdit;
};

#endif // ADDFRIGODIALOG_H
