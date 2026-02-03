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

    void setupUi(QDialog *AddFrigoDialog)
    {
        if (AddFrigoDialog->objectName().isEmpty())
            AddFrigoDialog->setObjectName("AddFrigoDialog");
        AddFrigoDialog->resize(450, 750);
        mainLayout = new QVBoxLayout(AddFrigoDialog);
        mainLayout->setSpacing(15);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(20, 20, 20, 20);
        labelId = new QLabel(AddFrigoDialog);
        labelId->setObjectName("labelId");

        mainLayout->addWidget(labelId);

        idEdit = new QLineEdit(AddFrigoDialog);
        idEdit->setObjectName("idEdit");

        mainLayout->addWidget(idEdit);

        labelCap = new QLabel(AddFrigoDialog);
        labelCap->setObjectName("labelCap");

        mainLayout->addWidget(labelCap);

        capEdit = new QLineEdit(AddFrigoDialog);
        capEdit->setObjectName("capEdit");

        mainLayout->addWidget(capEdit);

        labelHum = new QLabel(AddFrigoDialog);
        labelHum->setObjectName("labelHum");

        mainLayout->addWidget(labelHum);

        humEdit = new QLineEdit(AddFrigoDialog);
        humEdit->setObjectName("humEdit");

        mainLayout->addWidget(humEdit);

        labelStatut = new QLabel(AddFrigoDialog);
        labelStatut->setObjectName("labelStatut");

        mainLayout->addWidget(labelStatut);

        statusBox = new QComboBox(AddFrigoDialog);
        statusBox->setObjectName("statusBox");

        mainLayout->addWidget(statusBox);

        labelFish = new QLabel(AddFrigoDialog);
        labelFish->setObjectName("labelFish");

        mainLayout->addWidget(labelFish);

        fishBox = new QComboBox(AddFrigoDialog);
        fishBox->setObjectName("fishBox");

        mainLayout->addWidget(fishBox);

        labelTemp = new QLabel(AddFrigoDialog);
        labelTemp->setObjectName("labelTemp");

        mainLayout->addWidget(labelTemp);

        tempEdit = new QLineEdit(AddFrigoDialog);
        tempEdit->setObjectName("tempEdit");

        mainLayout->addWidget(tempEdit);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainLayout->addItem(verticalSpacer);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        btnCancelDialog = new QPushButton(AddFrigoDialog);
        btnCancelDialog->setObjectName("btnCancelDialog");
        btnCancelDialog->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        buttonLayout->addWidget(btnCancelDialog);

        btnAddDialog = new QPushButton(AddFrigoDialog);
        btnAddDialog->setObjectName("btnAddDialog");
        btnAddDialog->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        buttonLayout->addWidget(btnAddDialog);


        mainLayout->addLayout(buttonLayout);


        retranslateUi(AddFrigoDialog);

        QMetaObject::connectSlotsByName(AddFrigoDialog);
    } // setupUi

    void retranslateUi(QDialog *AddFrigoDialog)
    {
        AddFrigoDialog->setWindowTitle(QCoreApplication::translate("AddFrigoDialog", "Add Delivery", nullptr));
        labelId->setText(QCoreApplication::translate("AddFrigoDialog", "Delivery ID:", nullptr));
        idEdit->setPlaceholderText(QCoreApplication::translate("AddFrigoDialog", "LIV00X", nullptr));
        labelCap->setText(QCoreApplication::translate("AddFrigoDialog", "Date:", nullptr));
        capEdit->setPlaceholderText(QCoreApplication::translate("AddFrigoDialog", "YYYY-MM-DD", nullptr));
        labelHum->setText(QCoreApplication::translate("AddFrigoDialog", "Address:", nullptr));
        humEdit->setPlaceholderText(QCoreApplication::translate("AddFrigoDialog", "Full Address", nullptr));
        labelStatut->setText(QCoreApplication::translate("AddFrigoDialog", "Status:", nullptr));
        labelFish->setText(QCoreApplication::translate("AddFrigoDialog", "Transport Type:", nullptr));
        labelTemp->setText(QCoreApplication::translate("AddFrigoDialog", "Price:", nullptr));
        tempEdit->setPlaceholderText(QCoreApplication::translate("AddFrigoDialog", "Ex: 1500 USD", nullptr));
        btnCancelDialog->setText(QCoreApplication::translate("AddFrigoDialog", "\342\235\214 Cancel", nullptr));
        btnAddDialog->setText(QCoreApplication::translate("AddFrigoDialog", "\342\234\205 Add", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddFrigoDialog: public Ui_AddFrigoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDFRIGODIALOG_H
