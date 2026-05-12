#ifndef PECHEDIALOG_H
#define PECHEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include "peche.h"

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
    bool validateInputs();
    void showError(const QString& msg);
    void updateFieldStyle(QWidget* field, bool isValid);

private slots:
    void onSave();
    void refreshFridgeCombo(const QString &espece);
    void autoSelectFridge();
    void updateAvailableSpaceDisplay();

private:
    QLineEdit* referenceInput;
    QLabel* refErrorLabel;
    QComboBox* especeCombo;
    QLineEdit* quantiteInput;
    QLabel* qteErrorLabel;
    QDateEdit* dateInput;
    QComboBox* boatCombo;
    QComboBox* fridgeCombo;
    QComboBox* fishermanCombo;
    QPushButton* autoSelectBtn;
    QLabel* autoMsgLabel;
    QLabel* availableSpaceLabel;

    Peche* pecheData;
    bool isEdit;
};

#endif // PECHEDIALOG_H
