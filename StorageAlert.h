#ifndef STORAGEALERT_H
#define STORAGEALERT_H

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class StorageAlert : public QDialog
{
    Q_OBJECT

public:
    explicit StorageAlert(const QString& fridgeRef, const QString& fishType, int days, QWidget *parent = nullptr);
    ~StorageAlert();

private:
    void setupUi(const QString& ref, const QString& type, int days);

    QLabel* iconLabel;
    QLabel* titleLabel;
    QLabel* messageLabel;
    QLabel* storageInfoLabel;
};

#endif // STORAGEALERT_H
