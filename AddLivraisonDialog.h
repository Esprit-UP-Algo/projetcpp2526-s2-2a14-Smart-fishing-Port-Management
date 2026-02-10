#ifndef ADDLIVRAISONDIALOG_H
#define ADDLIVRAISONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include "Livraisonwindow.h"

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
    QString getInputStyle() const;

    QLineEdit* adresseEdit;
    QLineEdit* transportEdit;
    QLineEdit* prixEdit;
    QComboBox* statusBox;

    Livraison* livraisonData;
    bool isEdit;
};

#endif // ADDLIVRAISONDIALOG_H
