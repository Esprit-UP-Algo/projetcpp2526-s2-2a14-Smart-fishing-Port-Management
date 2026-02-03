#include "addfrigodialog.h"
#include "ui_addfrigodialog.h"
#include <QPushButton>
#include <QDebug>

AddFrigoDialog::AddFrigoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFrigoDialog)
{
    ui->setupUi(this);

    // Populate data (Logic)
    ui->statusBox->clear();
    ui->statusBox->addItems({"Pending", "In Progress", "Delivered", "Cancelled"});
    
    ui->fishBox->clear();
    ui->fishBox->addItems({"Standard Truck", "Refrig. Truck", "Boat", "Plane", "Train"});

    // Connect existing UI buttons
    connect(ui->btnAddDialog, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->btnCancelDialog, &QPushButton::clicked, this, &QDialog::reject);

    // Set focus
    ui->idEdit->setFocus();
}

AddFrigoDialog::~AddFrigoDialog()
{
    delete ui;
}

// Getters adapted for Delivery Data
QString AddFrigoDialog::getId() const {
    return ui->idEdit->text().trimmed();
}

QString AddFrigoDialog::getCap() const { // Date
    return ui->capEdit->text().trimmed();
}

QString AddFrigoDialog::getHum() const { // Address
    return ui->humEdit->text().trimmed();
}

QString AddFrigoDialog::getTemp() const { // Price
    return ui->tempEdit->text().trimmed();
}

QString AddFrigoDialog::getStatus() const {
    return ui->statusBox->currentText();
}

QString AddFrigoDialog::getFish() const { // Type Transport
    return ui->fishBox->currentText();
}

void AddFrigoDialog::setData(const QString &id, const QString &date, const QString &addr,
                             const QString &price, const QString &status, const QString &type)
{
    ui->idEdit->setText(id);
    ui->capEdit->setText(date);
    ui->humEdit->setText(addr);
    ui->tempEdit->setText(price);
    
    // Set combo boxes
    int statusIndex = ui->statusBox->findText(status);
    if (statusIndex >= 0) ui->statusBox->setCurrentIndex(statusIndex);
    
    int typeIndex = ui->fishBox->findText(type);
    if (typeIndex >= 0) ui->fishBox->setCurrentIndex(typeIndex);
}
