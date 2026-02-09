#include "adddockdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AddDockDialog::AddDockDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Ajouter un quai");
    setFixedSize(400, 350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    nomInput = new QLineEdit;
    capaciteInput = new QLineEdit;
    tailleMaxInput = new QLineEdit;
    tarifInput = new QLineEdit;
    clientInput = new QLineEdit;

    statutInput = new QComboBox;
    statutInput->addItems({"Disponible", "Occupé", "Maintenance"});

    mainLayout->addWidget(new QLabel("Nom du quai"));
    mainLayout->addWidget(nomInput);

    mainLayout->addWidget(new QLabel("Capacité"));
    mainLayout->addWidget(capaciteInput);

    mainLayout->addWidget(new QLabel("Taille max"));
    mainLayout->addWidget(tailleMaxInput);

    mainLayout->addWidget(new QLabel("Tarif"));
    mainLayout->addWidget(tarifInput);

    mainLayout->addWidget(new QLabel("Client"));
    mainLayout->addWidget(clientInput);

    mainLayout->addWidget(new QLabel("Statut"));
    mainLayout->addWidget(statutInput);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    saveBtn = new QPushButton("Enregistrer");
    cancelBtn = new QPushButton("Annuler");

    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);

    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

Dock AddDockDialog::getData() const
{
    Dock d;
    d.nom = nomInput->text();
    d.capacite = capaciteInput->text();
    d.tailleMax = tailleMaxInput->text();
    d.tarif = tarifInput->text();
    d.client = clientInput->text();
    d.statut = statutInput->currentText();
    return d;
}
