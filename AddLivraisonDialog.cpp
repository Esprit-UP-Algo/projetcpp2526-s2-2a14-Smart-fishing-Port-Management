#include "AddLivraisonDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>
#include <QFile>
#include <QTextStream>
#include <QStyle>


AddLivraisonDialog::AddLivraisonDialog(QWidget *parent, Livraison* livraisonData)
    : QDialog(parent), livraisonData(livraisonData), isEdit(livraisonData != nullptr)
{
    setupUi();
    if (isEdit) {
        populateFields();
    }
    loadStyleSheet();
}

void AddLivraisonDialog::loadStyleSheet()
{
    QFile file(":/style/livraison.css");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        this->setStyleSheet(stream.readAll());
    }
}



AddLivraisonDialog::~AddLivraisonDialog()
{
}

void AddLivraisonDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
    setFixedSize(600, 550);
    setModal(true);

    setObjectName("addLivraisonDialog");


    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(80);
    header->setObjectName("dialogHeader");

    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel* title = new QLabel(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setObjectName("dialogTitle");
    headerLayout->addWidget(title);


    headerLayout->addStretch();

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(40, 40);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setObjectName("dialogCloseBtn");

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(header);

    // Form
    QWidget* content = new QWidget();
    content->setObjectName("dialogContent");
    QVBoxLayout* formLayout = new QVBoxLayout(content);

    formLayout->setSpacing(20);
    formLayout->setContentsMargins(40, 30, 40, 30);

    QFont labelFont("Segoe UI", 11, QFont::Bold);
    QString labelStyle = "color: #2C3E50;";

    // Adresse
    QLabel* adresseLabel = new QLabel("Adresse de livraison");
    adresseLabel->setProperty("class", "form-label");
    formLayout->addWidget(adresseLabel);


    adresseEdit = new QLineEdit();
    adresseEdit->setPlaceholderText("Ex: 123 Rue de la Marine, Tunis");
    adresseEdit->setFixedHeight(45);
    adresseEdit->setProperty("class", "form-input");
    formLayout->addWidget(adresseEdit);


    // Transport et Prix
    QHBoxLayout* row2 = new QHBoxLayout();
    
    QVBoxLayout* transCol = new QVBoxLayout();
    QLabel* transLabel = new QLabel("Moyen de Transport");
    transLabel->setProperty("class", "form-label");
    transCol->addWidget(transLabel);

    transportEdit = new QLineEdit();
    transportEdit->setPlaceholderText("Ex: Camion");
    transportEdit->setFixedHeight(45);
    transportEdit->setProperty("class", "form-input");
    transCol->addWidget(transportEdit);

    row2->addLayout(transCol);

    QVBoxLayout* prixCol = new QVBoxLayout();
    QLabel* prixLabel = new QLabel("Prix (DT)");
    prixLabel->setProperty("class", "form-label");
    prixCol->addWidget(prixLabel);

    prixEdit = new QLineEdit();
    prixEdit->setPlaceholderText("Ex: 150");
    prixEdit->setFixedHeight(45);
    prixEdit->setProperty("class", "form-input");
    prixCol->addWidget(prixEdit);

    row2->addLayout(prixCol);

    formLayout->addLayout(row2);

    // Statut
    QLabel* statusLabel = new QLabel("Statut");
    statusLabel->setProperty("class", "form-label");
    formLayout->addWidget(statusLabel);


    statusBox = new QComboBox();
    statusBox->addItems({"En attente", "En cours", "Livré"});
    statusBox->setFixedHeight(45);
    statusBox->setProperty("class", "form-input");
    formLayout->addWidget(statusBox);


    formLayout->addStretch();

    // Buttons
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(15);
    btnRow->addStretch();

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedSize(120, 45);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setObjectName("dialogCancelBtn");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton(isEdit ? "Mettre à jour" : "Enregistrer");
    saveBtn->setFixedSize(150, 45);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setObjectName("dialogSaveBtn");

    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(saveBtn);

    formLayout->addLayout(btnRow);

    mainLayout->addWidget(content);
}



void AddLivraisonDialog::populateFields()
{
    if (!livraisonData) return;
    adresseEdit->setText(livraisonData->adresse);
    transportEdit->setText(livraisonData->transport);
    prixEdit->setText(livraisonData->prix);
    statusBox->setCurrentText(livraisonData->statut);
}

Livraison AddLivraisonDialog::getData() const
{
    Livraison data;
    data.adresse = adresseEdit->text();
    data.transport = transportEdit->text();
    data.prix = prixEdit->text();
    if (!data.prix.contains("DT")) data.prix += " DT";
    data.statut = statusBox->currentText();
    return data;
}
