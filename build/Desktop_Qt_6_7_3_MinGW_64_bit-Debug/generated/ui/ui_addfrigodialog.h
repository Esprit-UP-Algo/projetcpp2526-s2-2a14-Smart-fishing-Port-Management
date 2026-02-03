/********************************************************************************
** Form generated from reading UI file 'addfrigodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDFRIGODIALOG_H
#define UI_ADDFRIGODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AddFrigoDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *idEdit;
    QLabel *label_2;
    QLineEdit *capEdit;
    QLabel *label_3;
    QLineEdit *humEdit;
    QLabel *label_4;
    QLineEdit *tempEdit;
    QLabel *label_5;
    QComboBox *statusBox;
    QLabel *label_6;
    QComboBox *fishBox;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *buttonLayout;
    QPushButton *btnAddDialog;
    QPushButton *btnCancelDialog;

    void setupUi(QDialog *AddFrigoDialog)
    {
        if (AddFrigoDialog->objectName().isEmpty())
            AddFrigoDialog->setObjectName("AddFrigoDialog");
        AddFrigoDialog->resize(350, 400);
        AddFrigoDialog->setModal(true);
        verticalLayout = new QVBoxLayout(AddFrigoDialog);
        verticalLayout->setSpacing(10);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(20, 20, 20, 20);
        label = new QLabel(AddFrigoDialog);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        idEdit = new QLineEdit(AddFrigoDialog);
        idEdit->setObjectName("idEdit");

        verticalLayout->addWidget(idEdit);

        label_2 = new QLabel(AddFrigoDialog);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        capEdit = new QLineEdit(AddFrigoDialog);
        capEdit->setObjectName("capEdit");

        verticalLayout->addWidget(capEdit);

        label_3 = new QLabel(AddFrigoDialog);
        label_3->setObjectName("label_3");

        verticalLayout->addWidget(label_3);

        humEdit = new QLineEdit(AddFrigoDialog);
        humEdit->setObjectName("humEdit");

        verticalLayout->addWidget(humEdit);

        label_4 = new QLabel(AddFrigoDialog);
        label_4->setObjectName("label_4");

        verticalLayout->addWidget(label_4);

        tempEdit = new QLineEdit(AddFrigoDialog);
        tempEdit->setObjectName("tempEdit");

        verticalLayout->addWidget(tempEdit);

        label_5 = new QLabel(AddFrigoDialog);
        label_5->setObjectName("label_5");

        verticalLayout->addWidget(label_5);

        statusBox = new QComboBox(AddFrigoDialog);
        statusBox->setObjectName("statusBox");

        verticalLayout->addWidget(statusBox);

        label_6 = new QLabel(AddFrigoDialog);
        label_6->setObjectName("label_6");

        verticalLayout->addWidget(label_6);

        fishBox = new QComboBox(AddFrigoDialog);
        fishBox->setObjectName("fishBox");

        verticalLayout->addWidget(fishBox);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(15);
        buttonLayout->setObjectName("buttonLayout");
        btnAddDialog = new QPushButton(AddFrigoDialog);
        btnAddDialog->setObjectName("btnAddDialog");

        buttonLayout->addWidget(btnAddDialog);

        btnCancelDialog = new QPushButton(AddFrigoDialog);
        btnCancelDialog->setObjectName("btnCancelDialog");

        buttonLayout->addWidget(btnCancelDialog);


        verticalLayout->addLayout(buttonLayout);


        retranslateUi(AddFrigoDialog);

        QMetaObject::connectSlotsByName(AddFrigoDialog);
    } // setupUi

    void retranslateUi(QDialog *AddFrigoDialog)
    {
        AddFrigoDialog->setWindowTitle(QCoreApplication::translate("AddFrigoDialog", "Ajouter un Frigo", nullptr));
        label->setText(QCoreApplication::translate("AddFrigoDialog", "ID Frigo", nullptr));
        label_2->setText(QCoreApplication::translate("AddFrigoDialog", "Capacit\303\251 (Kg)", nullptr));
        label_3->setText(QCoreApplication::translate("AddFrigoDialog", "Humidit\303\251 (%)", nullptr));
        label_4->setText(QCoreApplication::translate("AddFrigoDialog", "Temp\303\251rature (\302\260C)", nullptr));
        label_5->setText(QCoreApplication::translate("AddFrigoDialog", "Statut", nullptr));
        label_6->setText(QCoreApplication::translate("AddFrigoDialog", "Type de Poisson", nullptr));
        btnAddDialog->setText(QCoreApplication::translate("AddFrigoDialog", "Ajouter", nullptr));
        btnCancelDialog->setText(QCoreApplication::translate("AddFrigoDialog", "Annuler", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddFrigoDialog: public Ui_AddFrigoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDFRIGODIALOG_H
