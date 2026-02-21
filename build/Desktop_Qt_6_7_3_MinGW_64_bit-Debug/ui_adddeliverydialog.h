/********************************************************************************
** Form generated from reading UI file 'AddDeliveryDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDDELIVERYDIALOG_H
#define UI_ADDDELIVERYDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AddDeliveryDialog
{
public:
    QVBoxLayout *mainLayout;
    QFrame *dialogHeader;
    QHBoxLayout *headerLayout;
    QLabel *dialogTitle;
    QSpacerItem *horizontalSpacer;
    QPushButton *dialogCloseBtn;
    QWidget *dialogContent;
    QVBoxLayout *formLayout;
    QLabel *label_adresse;
    QLineEdit *adresseEdit;
    QHBoxLayout *row2;
    QVBoxLayout *transCol;
    QLabel *label_transport;
    QLineEdit *transportEdit;
    QVBoxLayout *prixCol;
    QLabel *label_prix;
    QLineEdit *prixEdit;
    QLabel *label_statut;
    QComboBox *statusBox;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *btnRow;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *dialogCancelBtn;
    QPushButton *dialogSaveBtn;

    void setupUi(QDialog *AddDeliveryDialog)
    {
        if (AddDeliveryDialog->objectName().isEmpty())
            AddDeliveryDialog->setObjectName("AddDeliveryDialog");
        AddDeliveryDialog->resize(600, 550);
        AddDeliveryDialog->setStyleSheet(QString::fromUtf8("#AddDeliveryDialog {\n"
"    background-color: #F0F4F8;\n"
"}"));
        mainLayout = new QVBoxLayout(AddDeliveryDialog);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        dialogHeader = new QFrame(AddDeliveryDialog);
        dialogHeader->setObjectName("dialogHeader");
        dialogHeader->setMinimumSize(QSize(0, 80));
        dialogHeader->setMaximumSize(QSize(16777215, 80));
        headerLayout = new QHBoxLayout(dialogHeader);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(30, 20, 30, 20);
        dialogTitle = new QLabel(dialogHeader);
        dialogTitle->setObjectName("dialogTitle");

        headerLayout->addWidget(dialogTitle);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        headerLayout->addItem(horizontalSpacer);

        dialogCloseBtn = new QPushButton(dialogHeader);
        dialogCloseBtn->setObjectName("dialogCloseBtn");
        dialogCloseBtn->setMinimumSize(QSize(40, 40));
        dialogCloseBtn->setMaximumSize(QSize(40, 40));

        headerLayout->addWidget(dialogCloseBtn);


        mainLayout->addWidget(dialogHeader);

        dialogContent = new QWidget(AddDeliveryDialog);
        dialogContent->setObjectName("dialogContent");
        formLayout = new QVBoxLayout(dialogContent);
        formLayout->setSpacing(20);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(40, 30, 40, 30);
        label_adresse = new QLabel(dialogContent);
        label_adresse->setObjectName("label_adresse");

        formLayout->addWidget(label_adresse);

        adresseEdit = new QLineEdit(dialogContent);
        adresseEdit->setObjectName("adresseEdit");
        adresseEdit->setMinimumSize(QSize(0, 45));

        formLayout->addWidget(adresseEdit);

        row2 = new QHBoxLayout();
        row2->setObjectName("row2");
        transCol = new QVBoxLayout();
        transCol->setObjectName("transCol");
        label_transport = new QLabel(dialogContent);
        label_transport->setObjectName("label_transport");

        transCol->addWidget(label_transport);

        transportEdit = new QLineEdit(dialogContent);
        transportEdit->setObjectName("transportEdit");
        transportEdit->setMinimumSize(QSize(0, 45));

        transCol->addWidget(transportEdit);


        row2->addLayout(transCol);

        prixCol = new QVBoxLayout();
        prixCol->setObjectName("prixCol");
        label_prix = new QLabel(dialogContent);
        label_prix->setObjectName("label_prix");

        prixCol->addWidget(label_prix);

        prixEdit = new QLineEdit(dialogContent);
        prixEdit->setObjectName("prixEdit");
        prixEdit->setMinimumSize(QSize(0, 45));

        prixCol->addWidget(prixEdit);


        row2->addLayout(prixCol);


        formLayout->addLayout(row2);

        label_statut = new QLabel(dialogContent);
        label_statut->setObjectName("label_statut");

        formLayout->addWidget(label_statut);

        statusBox = new QComboBox(dialogContent);
        statusBox->addItem(QString());
        statusBox->addItem(QString());
        statusBox->addItem(QString());
        statusBox->setObjectName("statusBox");
        statusBox->setMinimumSize(QSize(0, 45));

        formLayout->addWidget(statusBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout->addItem(verticalSpacer);

        btnRow = new QHBoxLayout();
        btnRow->setSpacing(15);
        btnRow->setObjectName("btnRow");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        btnRow->addItem(horizontalSpacer_2);

        dialogCancelBtn = new QPushButton(dialogContent);
        dialogCancelBtn->setObjectName("dialogCancelBtn");
        dialogCancelBtn->setMinimumSize(QSize(120, 45));

        btnRow->addWidget(dialogCancelBtn);

        dialogSaveBtn = new QPushButton(dialogContent);
        dialogSaveBtn->setObjectName("dialogSaveBtn");
        dialogSaveBtn->setMinimumSize(QSize(150, 45));

        btnRow->addWidget(dialogSaveBtn);


        formLayout->addLayout(btnRow);


        mainLayout->addWidget(dialogContent);


        retranslateUi(AddDeliveryDialog);

        QMetaObject::connectSlotsByName(AddDeliveryDialog);
    } // setupUi

    void retranslateUi(QDialog *AddDeliveryDialog)
    {
        AddDeliveryDialog->setWindowTitle(QCoreApplication::translate("AddDeliveryDialog", "Ajouter Livraison", nullptr));
        dialogTitle->setText(QCoreApplication::translate("AddDeliveryDialog", "Ajouter Livraison", nullptr));
        dialogCloseBtn->setText(QCoreApplication::translate("AddDeliveryDialog", "\342\234\225", nullptr));
        label_adresse->setText(QCoreApplication::translate("AddDeliveryDialog", "Adresse de livraison", nullptr));
        adresseEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "Ex: 123 Rue de la Marine, Tunis", nullptr));
        label_transport->setText(QCoreApplication::translate("AddDeliveryDialog", "Moyen de Transport", nullptr));
        transportEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "Ex: Camion", nullptr));
        label_prix->setText(QCoreApplication::translate("AddDeliveryDialog", "Prix (DT)", nullptr));
        prixEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "Ex: 150", nullptr));
        label_statut->setText(QCoreApplication::translate("AddDeliveryDialog", "Statut", nullptr));
        statusBox->setItemText(0, QCoreApplication::translate("AddDeliveryDialog", "En attente", nullptr));
        statusBox->setItemText(1, QCoreApplication::translate("AddDeliveryDialog", "En cours", nullptr));
        statusBox->setItemText(2, QCoreApplication::translate("AddDeliveryDialog", "Livr\303\251", nullptr));

        dialogCancelBtn->setText(QCoreApplication::translate("AddDeliveryDialog", "Annuler", nullptr));
        dialogSaveBtn->setText(QCoreApplication::translate("AddDeliveryDialog", "Enregistrer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddDeliveryDialog: public Ui_AddDeliveryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDDELIVERYDIALOG_H
