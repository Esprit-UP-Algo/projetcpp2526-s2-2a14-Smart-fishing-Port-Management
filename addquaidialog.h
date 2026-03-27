#ifndef ADDQUAIDIALOG_H
#define ADDQUAIDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;
class Quai;

class AddQuaiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddQuaiDialog(QWidget *parent = nullptr);
    Quai getData() const;
    void saveQuai();

private:
    QLineEdit* numeroInput;
    QLineEdit* locationInput;
    QLineEdit* capaciteInput;
    QLineEdit* tarifInput;
    QLineEdit* dureeInput;
    QComboBox* etatInput;

    QPushButton* saveBtn;
    QPushButton* cancelBtn;
};

#endif // ADDQUAIDIALOG_H
