#include "adddockdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AddDockDialog::AddDockDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Gestion des Quais");
    setFixedSize(600, 650);
    setModal(true);

    setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header premium
    QFrame* header = new QFrame();
    header->setFixedHeight(100);
    header->setStyleSheet("background-color: #5D9CEC;");
    
    QVBoxLayout* headerVLayout = new QVBoxLayout(header);
    headerVLayout->setContentsMargins(30, 10, 30, 10);
    headerVLayout->setSpacing(5);

    QLabel* titleLabel = new QLabel("Ajouter un Quai");
    titleLabel->setFont(QFont("Segoe UI", 18, QFont::Bold));
    titleLabel->setStyleSheet("color: white;");
    headerVLayout->addWidget(titleLabel);

    QLabel* subTitle = new QLabel("⚓  Configuration de l'emplacement");
    subTitle->setFont(QFont("Segoe UI", 11));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerVLayout->addWidget(subTitle);

    mainLayout->addWidget(header);

    // Form area
    QWidget* content = new QWidget();
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setContentsMargins(40, 30, 40, 30);
    formLayout->setSpacing(15);

    auto addField = [&](const QString& label, QWidget* input) {
        QLabel* lbl = new QLabel(label);
        lbl->setFont(QFont("Segoe UI", 11, QFont::Bold));
        lbl->setStyleSheet("color: #2C3E50;");
        formLayout->addWidget(lbl);
        input->setFixedHeight(45);
        input->setStyleSheet(R"(
            QLineEdit, QComboBox {
                background-color: #F8F9FA;
                border: 2px solid #E1E8ED;
                border-radius: 10px;
                padding: 5px 15px;
                font-family: 'Segoe UI';
                font-size: 11pt;
            }
            QLineEdit:focus, QComboBox:focus {
                border: 2px solid #5D9CEC;
                background-color: white;
            }
        )");
        formLayout->addWidget(input);
    };

    nomInput = new QLineEdit;
    capaciteInput = new QLineEdit;
    tailleMaxInput = new QLineEdit;
    tarifInput = new QLineEdit;
    clientInput = new QLineEdit;
    statutInput = new QComboBox;
    statutInput->addItems({"Disponible", "Occupé", "Maintenance"});

    addField("Nom du quai", nomInput);
    addField("Capacité", capaciteInput);
    addField("Taille max", tailleMaxInput);
    addField("Tarif", tarifInput);
    addField("Client", clientInput);
    addField("Statut", statutInput);

    formLayout->addStretch();

    QHBoxLayout* btnLayout = new QHBoxLayout;
    saveBtn = new QPushButton("Enregistrer");
    cancelBtn = new QPushButton("Annuler");

    saveBtn->setFixedSize(140, 45);
    cancelBtn->setFixedSize(120, 45);
    saveBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setCursor(Qt::PointingHandCursor);

    saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #5D9CEC; color: white; border-radius: 10px; font-weight: bold; font-size: 11pt; }
        QPushButton:hover { background-color: #4A89DC; }
    )");
    cancelBtn->setStyleSheet(R"(
        QPushButton { background-color: #E8EEF5; color: #5A6C7D; border-radius: 10px; font-weight: bold; font-size: 11pt; }
        QPushButton:hover { background-color: #D8DEE5; }
    )");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    formLayout->addLayout(btnLayout);

    mainLayout->addWidget(content);

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
