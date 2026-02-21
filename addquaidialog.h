#ifndef ADDQUAIDIALOG_H
#define ADDQUAIDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "quai.h"

class AddQuaiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddQuaiDialog(QWidget *parent = nullptr);
    Quai getData() const;

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

#endif // ADDQUAIDIALOG_H
