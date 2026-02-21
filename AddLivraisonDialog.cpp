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
    setStyleSheet("background-color: #F0F4F8;");


    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(100);
    header->setObjectName("dialogHeader");
    header->setStyleSheet(R"(
        #dialogHeader {
            background-color: #5D9CEC;
            border-top-left-radius: 0px;
            border-top-right-radius: 0px;
        }
    )");

    QVBoxLayout* headerVLayout = new QVBoxLayout(header);
    headerVLayout->setContentsMargins(30, 10, 30, 10);
    headerVLayout->setSpacing(5);

    QLabel* title = new QLabel(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setStyleSheet("color: white;");
    headerVLayout->addWidget(title);

    QLabel* subTitle = new QLabel(isEdit ? "📦  Mise à jour des informations" : "📦  Nouvelle expédition");
    subTitle->setFont(QFont("Segoe UI", 11));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerVLayout->addWidget(subTitle);

    mainLayout->addWidget(header);

    // Form
    QWidget* content = new QWidget();
    content->setObjectName("dialogContent");
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);

    formLayout->setSpacing(20);
    formLayout->setContentsMargins(40, 30, 40, 30);

    QFont labelFont("Segoe UI", 11, QFont::Bold);
    QString labelStyle = "color: #2C3E50;";

    // Adresse
    QLabel* adresseLabel = new QLabel("Adresse de livraison");
    adresseLabel->setStyleSheet("color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt; font-weight: bold;");
    formLayout->addWidget(adresseLabel);


    adresseEdit = new QLineEdit();
    adresseEdit->setPlaceholderText("Ex: 123 Rue de la Marine, Tunis");
    adresseEdit->setFixedHeight(45);
    adresseEdit->setStyleSheet("background-color: #F8F9FA; border: 2px solid #E1E8ED; border-radius: 8px; padding: 8px 15px; color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt;");
    formLayout->addWidget(adresseEdit);


    // Transport et Prix
    QHBoxLayout* row2 = new QHBoxLayout();
    
    QVBoxLayout* transCol = new QVBoxLayout();
    QLabel* transLabel = new QLabel("Moyen de Transport");
    transLabel->setStyleSheet("color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt; font-weight: bold;");
    transCol->addWidget(transLabel);

    transportEdit = new QLineEdit();
    transportEdit->setPlaceholderText("Ex: Camion");
    transportEdit->setFixedHeight(45);
    transportEdit->setStyleSheet("background-color: #F8F9FA; border: 2px solid #E1E8ED; border-radius: 8px; padding: 8px 15px; color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt;");
    transCol->addWidget(transportEdit);

    row2->addLayout(transCol);

    QVBoxLayout* prixCol = new QVBoxLayout();
    QLabel* prixLabel = new QLabel("Prix (DT)");
    prixLabel->setStyleSheet("color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt; font-weight: bold;");
    prixCol->addWidget(prixLabel);

    prixEdit = new QLineEdit();
    prixEdit->setPlaceholderText("Ex: 150");
    prixEdit->setFixedHeight(45);
    prixEdit->setStyleSheet("background-color: #F8F9FA; border: 2px solid #E1E8ED; border-radius: 8px; padding: 8px 15px; color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt;");
    prixCol->addWidget(prixEdit);

    row2->addLayout(prixCol);

    formLayout->addLayout(row2);

    // Statut
    QLabel* statusLabel = new QLabel("Statut");
    statusLabel->setStyleSheet("color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt; font-weight: bold;");
    formLayout->addWidget(statusLabel);


    statusBox = new QComboBox();
    statusBox->addItems({"En attente", "En cours", "Livré"});
    statusBox->setFixedHeight(45);
    statusBox->setStyleSheet("background-color: #F8F9FA; border: 2px solid #E1E8ED; border-radius: 8px; padding: 8px 15px; color: #2C3E50; font-family: 'Segoe UI'; font-size: 11pt;");
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
    cancelBtn->setStyleSheet("QPushButton { background-color: #E8EEF5; color: #5A6C7D; border: none; border-radius: 8px; font-weight: 600; }"
                             "QPushButton:hover { background-color: #D8DEE5; }");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton(isEdit ? "Mettre à jour" : "Enregistrer");
    saveBtn->setFixedSize(150, 45);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setObjectName("dialogSaveBtn");
    saveBtn->setStyleSheet("QPushButton { background-color: #5D9CEC; color: white; border: none; border-radius: 8px; font-weight: 700; }"
                           "QPushButton:hover { background-color: #4A89DC; }");

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
