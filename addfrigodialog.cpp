#include "addfrigodialog.h"

AddFrigoDialog::AddFrigoDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Frigo");
    setFixedSize(350,400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    idEdit   = new QLineEdit();
    capEdit  = new QLineEdit();
    humEdit  = new QLineEdit();
    tempEdit = new QLineEdit();

    statusBox = new QComboBox();
    fishBox   = new QComboBox();

    statusBox->addItems({"Disponible","Occupé"});
    fishBox->addItems({"Sardine","Thon","Merlan","Crevette","Saumon"});

    layout->addWidget(new QLabel("ID Frigo"));
    layout->addWidget(idEdit);

    layout->addWidget(new QLabel("Capacité (Kg)"));
    layout->addWidget(capEdit);

    layout->addWidget(new QLabel("Humidité (%)"));
    layout->addWidget(humEdit);

    layout->addWidget(new QLabel("Température (°C)"));
    layout->addWidget(tempEdit);

    layout->addWidget(new QLabel("Statut"));
    layout->addWidget(statusBox);

    layout->addWidget(new QLabel("Poisson"));
    layout->addWidget(fishBox);

    QPushButton *btnAdd = new QPushButton("Valider");
    QPushButton *btnCancel = new QPushButton("Annuler");

    btnAdd->setStyleSheet("background:#22c55e;color:white;height:35px;border-radius:6px;");
    btnCancel->setStyleSheet("background:#ef4444;color:white;height:35px;border-radius:6px;");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnCancel);

    layout->addLayout(btnLayout);

    connect(btnAdd,&QPushButton::clicked,this,&QDialog::accept);
    connect(btnCancel,&QPushButton::clicked,this,&QDialog::reject);
}

void AddFrigoDialog::setData(QString id, QString cap, QString hum,
             QString temp, QString stat, QString fish)
{
    idEdit->setText(id);
    capEdit->setText(cap);
    humEdit->setText(hum);
    tempEdit->setText(temp);

    statusBox->setCurrentText(stat);
    fishBox->setCurrentText(fish);
}

QString AddFrigoDialog::getId(){ return idEdit->text(); }
QString AddFrigoDialog::getCap(){ return capEdit->text(); }
QString AddFrigoDialog::getHum(){ return humEdit->text(); }
QString AddFrigoDialog::getTemp(){ return tempEdit->text(); }
QString AddFrigoDialog::getStatus(){ return statusBox->currentText(); }
QString AddFrigoDialog::getFish(){ return fishBox->currentText(); }
