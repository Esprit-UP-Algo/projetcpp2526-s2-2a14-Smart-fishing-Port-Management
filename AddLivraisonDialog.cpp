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
    this->update();
}


AddLivraisonDialog::~AddLivraisonDialog()
{
}

void AddLivraisonDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
    setFixedSize(650, 680);
    setModal(true);

    setStyleSheet(R"(
        QDialog {
            background-color: #F0F4F8;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(130);
    header->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
        }
    )");
    QVBoxLayout* headerVLayout = new QVBoxLayout(header);
    headerVLayout->setContentsMargins(30, 15, 30, 15);
    headerVLayout->setSpacing(2);

    QLabel* title = new QLabel(isEdit ? "Modifier Livraison" : "Ajouter Livraison");
    title->setFont(QFont("Segoe UI", 20, QFont::Bold));
    title->setStyleSheet("color: white;");
    headerVLayout->addWidget(title);

    QLabel* infoLabel = new QLabel("Veuillez saisir l'adresse, le nom du van, le type de transport, le prix et le statut.");
    infoLabel->setFont(QFont("Segoe UI", 9));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: rgba(255, 255, 255, 0.85);");
    headerVLayout->addWidget(infoLabel);

    QLabel* subTitle = new QLabel(isEdit ? "✏️  Mise à jour des informations" : "📦  Nouvelle expédition");
    subTitle->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.95);");
    headerVLayout->addWidget(subTitle);

    mainLayout->addWidget(header);

    // Scroll Area (matching BateauDialog)
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #ffffff; border: none; }");
    scrollArea->viewport()->setStyleSheet("background-color: #ffffff;");

    QWidget* content = new QWidget();
    content->setObjectName("dialogContent");
    content->setStyleSheet("QWidget#dialogContent { background-color: #ffffff; }");
    
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(20);
    formLayout->setContentsMargins(40, 30, 40, 30);

    auto getInputStyle = []() -> QString {
        return R"(
            QLineEdit, QComboBox, QTextEdit, QDateEdit {
                background-color: #F8F9FA;
                border: 2px solid #E1E8ED;
                border-radius: 10px;
                padding: 10px 15px;
                color: #2C3E50;
                font-size: 11pt;
            }
            QLineEdit:focus, QComboBox:focus, QTextEdit:focus, QDateEdit:focus {
                border: 2px solid #5D9CEC;
                background-color: white;
            }
            /* Fix for Calendar Popup */
            QCalendarWidget QAbstractItemView {
                background-color: white;
                color: #2C3E50;
                selection-background-color: #5D9CEC;
                selection-color: white;
            }
            QCalendarWidget QWidget#qt_calendar_navigationbar { 
                background-color: #5D9CEC; 
            }
            QCalendarWidget QToolButton {
                color: white;
                font-weight: bold;
            }
        )";
    };

    QFont labelFont("Segoe UI", 11, QFont::DemiBold);

    // Adresse
    QLabel* adresseLabel = new QLabel("📍  Adresse de livraison");
    adresseLabel->setFont(labelFont);
    adresseLabel->setStyleSheet("color: #2C3E50; margin-bottom: 2px;");
    formLayout->addWidget(adresseLabel);
    
    adresseEdit = new QTextEdit();
    adresseEdit->setPlaceholderText("Ex: 123 Rue de la Marine,\nTunis, Tunisie");
    adresseEdit->setFixedHeight(80);
    adresseEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(adresseEdit);

    // Date de livraison
    QLabel* dateLabel = new QLabel("📅  Date de livraison");
    dateLabel->setFont(labelFont);
    dateLabel->setStyleSheet("color: #2C3E50; margin-bottom: 2px;");
    formLayout->addWidget(dateLabel);
    
    dateEdit = new QDateEdit();
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setFixedHeight(45);
    dateEdit->setStyleSheet(getInputStyle());
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(dateEdit);

    // Véhicule
    QLabel* vehiculeLabel = new QLabel("🚐  Nom du Véhicule / Van");
    vehiculeLabel->setFont(labelFont);
    vehiculeLabel->setStyleSheet("color: #2C3E50; margin-bottom: 2px;");
    formLayout->addWidget(vehiculeLabel);
    
    vehiculeEdit = new QLineEdit();
    vehiculeEdit->setPlaceholderText("Ex: Van Agile-01");
    vehiculeEdit->setFixedHeight(45);
    vehiculeEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(vehiculeEdit);

    // Row for Transport and Price
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(20);
    
    QVBoxLayout* transCol = new QVBoxLayout();
    QLabel* transLabel = new QLabel("🚛  Transport");
    transLabel->setFont(labelFont);
    transLabel->setStyleSheet("color: #2C3E50;");
    transCol->addWidget(transLabel);
    
    transportEdit = new QComboBox();
    transportEdit->addItems({
        "Camion non frigorifique",
        "Camion frigorifique",
        "Motocyclette / Scooter",
        "Véhicule utilitaire léger",
        "Bateau"
    });
    transportEdit->setFixedHeight(45);
    transportEdit->setStyleSheet(getInputStyle());
    transCol->addWidget(transportEdit);
    row2->addLayout(transCol);

    QVBoxLayout* prixCol = new QVBoxLayout();
    QLabel* prixLabel = new QLabel("💰  Prix (DT)");
    prixLabel->setFont(labelFont);
    prixLabel->setStyleSheet("color: #2C3E50;");
    prixCol->addWidget(prixLabel);
    prixEdit = new QLineEdit();
    prixEdit->setPlaceholderText("Ex: 150");
    prixEdit->setFixedHeight(45);
    prixEdit->setStyleSheet(getInputStyle());
    prixCol->addWidget(prixEdit);
    row2->addLayout(prixCol);

    formLayout->addLayout(row2);

    // Statut
    QLabel* statusLabel = new QLabel("🏷️  Statut");
    statusLabel->setFont(labelFont);
    statusLabel->setStyleSheet("color: #2C3E50; margin-bottom: 2px;");
    formLayout->addWidget(statusLabel);
    
    statusBox = new QComboBox();
    statusBox->addItems({"En attente", "En cours", "Livré", "Annulé"});
    statusBox->setFixedHeight(45);
    statusBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(statusBox);

    formLayout->addStretch();

    // Buttons
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(15);
    btnRow->addStretch();

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedSize(120, 50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background-color: #E8EEF5; color: #5A6C7D; border: none; border-radius: 10px; font-weight: 600; }
        QPushButton:hover { background-color: #D8DEE5; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton(isEdit ? "Mettre à jour" : "Enregistrer");
    saveBtn->setFixedSize(160, 50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #5D9CEC; color: white; border: none; border-radius: 10px; font-weight: 700; }
        QPushButton:hover { background-color: #4A89DC; }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(saveBtn);

    formLayout->addLayout(btnRow);

    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);
}

void AddLivraisonDialog::populateFields()
{
    if (!livraisonData) return;
    
    // Set Date
    QDate d = QDate::fromString(livraisonData->getDate(), "dd/MM/yyyy");
    if (d.isValid()) dateEdit->setDate(d);
    else dateEdit->setDate(QDate::currentDate());

    adresseEdit->setPlainText(livraisonData->getAdresse());
    vehiculeEdit->setText(livraisonData->getVehicule());
    transportEdit->setCurrentText(livraisonData->getTransport());
    prixEdit->setText(livraisonData->getPrix());
    statusBox->setCurrentText(livraisonData->getStatut());
}

Livraison AddLivraisonDialog::getData() const
{
    Livraison data;
    data.setDate(dateEdit->date().toString("dd/MM/yyyy"));
    data.setAdresse(adresseEdit->toPlainText());
    data.setVehicule(vehiculeEdit->text());
    data.setTransport(transportEdit->currentText());
    
    QString prix = prixEdit->text();
    data.setPrix(prix.replace("DT", "").trimmed());
    
    data.setStatut(statusBox->currentText());
    
    // Automatic duration calculation (randomized for demo/completeness)
    if (isEdit && livraisonData) {
        data.setDuree(livraisonData->getDuree());
    } else {
        data.setDuree(15 + (rand() % 45)); // 15 to 60 mins
    }
    
    return data;
}
