#ifndef ADDLIVRAISONDIALOG_H
#define ADDLIVRAISONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include "Livraisonwindow.h"

struct Livraison;

class AddLivraisonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddLivraisonDialog(QWidget *parent = nullptr, Livraison* livraisonData = nullptr);
    ~AddLivraisonDialog();

    Livraison getData() const;

private:
    void setupUi();
    void populateFields();

    QTextEdit* adresseEdit;
    QLineEdit* vehiculeEdit;
    QComboBox* transportEdit;
    QLineEdit* prixEdit;
    QComboBox* statusBox;
    QDateEdit* dateEdit;

    Livraison* livraisonData;
    bool isEdit;
};

#endif // ADDLIVRAISONDIALOG_H
