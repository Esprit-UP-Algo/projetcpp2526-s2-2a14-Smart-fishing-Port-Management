#ifndef ADDDOCKDIALOG_H
#define ADDDOCKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "dock.h"   // <-- this already defines struct Dock

class AddDockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDockDialog(QWidget *parent = nullptr);
    Dock getData() const;

private:
    QLineEdit* nomInput;
    QLineEdit* capaciteInput;
    QLineEdit* tailleMaxInput;
    QLineEdit* tarifInput;
    QLineEdit* clientInput;
    QComboBox* statutInput;

    QPushButton* saveBtn;
    QPushButton* cancelBtn;
};

#endif // ADDDOCKDIALOG_H
