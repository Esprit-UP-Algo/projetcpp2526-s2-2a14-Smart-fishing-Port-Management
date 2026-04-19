#include "PecheExportDialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PecheExportDialog::PecheExportDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Options d'Export PDF");
    setMinimumWidth(400);
    setStyleSheet("QDialog { background-color: #f8fafc; }");

    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(20);
    mainLay->setContentsMargins(25, 25, 25, 25);

    QLabel* titleL = new QLabel("Configuration du Rapport");
    titleL->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e3a5f;");
    mainLay->addWidget(titleL);

    // Type selection
    QLabel* typeL = new QLabel("Type de rapport :");
    typeL->setStyleSheet("font-weight: 600; color: #475569;");
    typeCombo = new QComboBox();
    typeCombo->addItems({"Rapport Complet (Total)", "Rapport par Bateau", "Rapport par Date"});
    typeCombo->setFixedHeight(35);
    mainLay->addWidget(typeL);
    mainLay->addWidget(typeCombo);

    // Boat Selection (Hidden by default)
    boatWidget = new QWidget();
    QVBoxLayout* boatLay = new QVBoxLayout(boatWidget);
    boatLay->setContentsMargins(0,0,0,0);
    QLabel* boatL = new QLabel("Choisir le bateau :");
    boatL->setStyleSheet("font-weight: 600; color: #475569;");
    boatCombo = new QComboBox();
    boatCombo->setFixedHeight(35);
    boatLay->addWidget(boatL);
    boatLay->addWidget(boatCombo);
    boatWidget->setVisible(false);
    mainLay->addWidget(boatWidget);

    // Date Selection (Hidden by default)
    dateWidget = new QWidget();
    QVBoxLayout* dateLayV = new QVBoxLayout(dateWidget);
    dateLayV->setContentsMargins(0,0,0,0);
    QLabel* dateL = new QLabel("Choisir la période :");
    dateL->setStyleSheet("font-weight: 600; color: #475569;");
    
    QHBoxLayout* dateLayH = new QHBoxLayout();
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1));
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setFixedHeight(35);
    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setFixedHeight(35);
    
    dateLayH->addWidget(new QLabel("Du:"));
    dateLayH->addWidget(startDateEdit);
    dateLayH->addWidget(new QLabel("Au:"));
    dateLayH->addWidget(endDateEdit);
    
    dateLayV->addWidget(dateL);
    dateLayV->addLayout(dateLayH);
    dateWidget->setVisible(false);
    mainLay->addWidget(dateWidget);

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    QPushButton* cancelBtn = new QPushButton("Annuler");
    QPushButton* okBtn = new QPushButton("Générer PDF");
    
    cancelBtn->setFixedHeight(40);
    okBtn->setFixedHeight(40);
    okBtn->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; border-radius: 8px;");
    cancelBtn->setStyleSheet("background-color: white; border: 1px solid #d1d5db; border-radius: 8px;");
    
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(okBtn);
    mainLay->addLayout(btnLay);

    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PecheExportDialog::onTypeChanged);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    loadBoats();
}

void PecheExportDialog::loadBoats()
{
    QSqlQuery q("SELECT DISTINCT IDBATEAU FROM BATEAUX");
    while(q.next()){
        boatCombo->addItem(q.value(0).toString());
    }
}

void PecheExportDialog::onTypeChanged(int index)
{
    boatWidget->setVisible(index == 1);
    dateWidget->setVisible(index == 2);
}

PecheExportDialog::ExportType PecheExportDialog::exportType() const 
{
    return static_cast<ExportType>(typeCombo->currentIndex());
}

QString PecheExportDialog::selectedBoat() const { return boatCombo->currentText(); }
QDate PecheExportDialog::startDate() const { return startDateEdit->date(); }
QDate PecheExportDialog::endDate() const { return endDateEdit->date(); }
