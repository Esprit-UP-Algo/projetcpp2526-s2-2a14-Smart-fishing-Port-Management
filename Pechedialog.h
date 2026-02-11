#ifndef PECHEDIALOG_H
#define PECHEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include "pechewindow.h"

class PecheDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PecheDialog(QWidget *parent = nullptr, Peche* pecheData = nullptr);
    ~PecheDialog();

    Peche getData() const;

private:
    void setupUi();
    void populateFields();
    QString getInputStyle() const;

    QLineEdit* referenceInput;
    QComboBox* especeCombo;
    QLineEdit* quantiteInput;
    QDateEdit* dateInput;

    Peche* pecheData;
    bool isEdit;
};

#endif // PECHEDIALOG_H
