#ifndef ADDQUAIDIALOG_H
#define ADDQUAIDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class Quai;

class AddQuaiDialog : public QDialog   // ✅ MUST be QDialog
{
    Q_OBJECT

public:
    explicit AddQuaiDialog(QWidget *parent = nullptr);
    Quai getData() const;

private slots:
    void saveQuai();

private:
    void updateGeneratedReference();

    QLineEdit* numeroInput;
    QLineEdit* locationInput;
    QLineEdit* capaciteInput;
    QLineEdit* tarifInput;
    QLineEdit* dureeInput;
    QComboBox* etatInput;
    QLabel* referenceValueLabel;

    QPushButton* saveBtn;
    QPushButton* cancelBtn;
};

#endif
