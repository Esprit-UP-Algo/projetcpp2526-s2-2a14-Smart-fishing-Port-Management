/********************************************************************************
** Form generated from reading UI file 'adddeliverydialog.ui'
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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AddDeliveryDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *labelId;
    QLineEdit *idEdit;
    QLabel *labelCap;
    QLineEdit *capEdit;
    QLabel *labelHum;
    QLineEdit *humEdit;
    QLabel *labelStatut;
    QComboBox *statusBox;
    QLabel *labelFish;
    QComboBox *fishBox;
    QLabel *labelTemp;
    QLineEdit *tempEdit;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *buttonLayout;
    QPushButton *btnCancelDialog;
    QPushButton *btnAddDialog;

    void setupUi(QDialog *AddDeliveryDialog)
    {
        if (AddDeliveryDialog->objectName().isEmpty())
            AddDeliveryDialog->setObjectName("AddDeliveryDialog");
        AddDeliveryDialog->resize(450, 750);
        mainLayout = new QVBoxLayout(AddDeliveryDialog);
        mainLayout->setSpacing(15);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(20, 20, 20, 20);
        labelId = new QLabel(AddDeliveryDialog);
        labelId->setObjectName("labelId");

        mainLayout->addWidget(labelId);

        idEdit = new QLineEdit(AddDeliveryDialog);
        idEdit->setObjectName("idEdit");

        mainLayout->addWidget(idEdit);

        labelCap = new QLabel(AddDeliveryDialog);
        labelCap->setObjectName("labelCap");

        mainLayout->addWidget(labelCap);

        capEdit = new QLineEdit(AddDeliveryDialog);
        capEdit->setObjectName("capEdit");

        mainLayout->addWidget(capEdit);

        labelHum = new QLabel(AddDeliveryDialog);
        labelHum->setObjectName("labelHum");

        mainLayout->addWidget(labelHum);

        humEdit = new QLineEdit(AddDeliveryDialog);
        humEdit->setObjectName("humEdit");

        mainLayout->addWidget(humEdit);

        labelStatut = new QLabel(AddDeliveryDialog);
        labelStatut->setObjectName("labelStatut");

        mainLayout->addWidget(labelStatut);

        statusBox = new QComboBox(AddDeliveryDialog);
        statusBox->setObjectName("statusBox");

        mainLayout->addWidget(statusBox);

        labelFish = new QLabel(AddDeliveryDialog);
        labelFish->setObjectName("labelFish");

        mainLayout->addWidget(labelFish);

        fishBox = new QComboBox(AddDeliveryDialog);
        fishBox->setObjectName("fishBox");

        mainLayout->addWidget(fishBox);

        labelTemp = new QLabel(AddDeliveryDialog);
        labelTemp->setObjectName("labelTemp");

        mainLayout->addWidget(labelTemp);

        tempEdit = new QLineEdit(AddDeliveryDialog);
        tempEdit->setObjectName("tempEdit");

        mainLayout->addWidget(tempEdit);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainLayout->addItem(verticalSpacer);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        btnCancelDialog = new QPushButton(AddDeliveryDialog);
        btnCancelDialog->setObjectName("btnCancelDialog");
        btnCancelDialog->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        buttonLayout->addWidget(btnCancelDialog);

        btnAddDialog = new QPushButton(AddDeliveryDialog);
        btnAddDialog->setObjectName("btnAddDialog");
        btnAddDialog->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        buttonLayout->addWidget(btnAddDialog);


        mainLayout->addLayout(buttonLayout);


        retranslateUi(AddDeliveryDialog);

        QMetaObject::connectSlotsByName(AddDeliveryDialog);
    } // setupUi

    void retranslateUi(QDialog *AddDeliveryDialog)
    {
        AddDeliveryDialog->setWindowTitle(QCoreApplication::translate("AddDeliveryDialog", "Add Delivery", nullptr));
        labelId->setText(QCoreApplication::translate("AddDeliveryDialog", "Delivery ID:", nullptr));
        idEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "LIV00X", nullptr));
        labelCap->setText(QCoreApplication::translate("AddDeliveryDialog", "Date:", nullptr));
        capEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "YYYY-MM-DD", nullptr));
        labelHum->setText(QCoreApplication::translate("AddDeliveryDialog", "Address:", nullptr));
        humEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "Full Address", nullptr));
        labelStatut->setText(QCoreApplication::translate("AddDeliveryDialog", "Status:", nullptr));
        labelFish->setText(QCoreApplication::translate("AddDeliveryDialog", "Transport Type:", nullptr));
        labelTemp->setText(QCoreApplication::translate("AddDeliveryDialog", "Price:", nullptr));
        tempEdit->setPlaceholderText(QCoreApplication::translate("AddDeliveryDialog", "Ex: 1500 USD", nullptr));
        btnCancelDialog->setText(QCoreApplication::translate("AddDeliveryDialog", "\342\235\214 Cancel", nullptr));
        btnAddDialog->setText(QCoreApplication::translate("AddDeliveryDialog", "\342\234\205 Add", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddDeliveryDialog: public Ui_AddDeliveryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDDELIVERYDIALOG_H
