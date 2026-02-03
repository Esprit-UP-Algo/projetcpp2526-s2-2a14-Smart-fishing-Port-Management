#ifndef ADDFRIGODIALOG_H
#define ADDFRIGODIALOG_H

#include <QDialog>

namespace Ui {
class AddFrigoDialog;
}

class AddFrigoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFrigoDialog(QWidget *parent = nullptr);
    ~AddFrigoDialog();

    QString getId() const;
    QString getCap() const;
    QString getHum() const;
    QString getTemp() const;
    QString getStatus() const;
    QString getFish() const;

    void setData(const QString &id, const QString &date, const QString &addr, 
                 const QString &price, const QString &status, const QString &type);

private:
    Ui::AddFrigoDialog *ui;
};

#endif // ADDFRIGODIALOG_H
