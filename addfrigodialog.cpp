#include "addfrigodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>

AddFrigoDialog::AddFrigoDialog(QWidget *parent, Frigo* frigoData)
    : QDialog(parent), frigoData(frigoData), isEdit(frigoData != nullptr)
{
    setupUi();

    if (isEdit) {
        populateFields();
    }
}

AddFrigoDialog::~AddFrigoDialog()
{
}

void AddFrigoDialog::setupUi()
{
    setWindowTitle("Ajouter / Modifier Frigo");
    setFixedSize(700, 650);
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
    header->setFixedHeight(80);
    header->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            padding: 20px;
        }
    )");
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel* title = new QLabel(isEdit ? "Modifier Frigo" : "Ajouter Frigo");
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: white;");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

    mainLayout->addWidget(header);

    // Scroll Area pour le formulaire
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");

    // Form content
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: white;");
    QVBoxLayout* formLayout = new QVBoxLayout(content);
    formLayout->setSpacing(25);
    formLayout->setContentsMargins(40, 40, 40, 40);

    QFont labelFont("Segoe UI", 12, QFont::Medium);
    QFont inputFont("Segoe UI", 12);

    // Référence
    QLabel* refLabel = new QLabel("Référence");
    refLabel->setFont(labelFont);
    refLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(refLabel);

    refEdit = new QLineEdit();
    refEdit->setPlaceholderText("Ex: REF-123");
    refEdit->setFont(inputFont);
    refEdit->setFixedHeight(50);
    refEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(refEdit);

    formLayout->addSpacing(10);

    // Capacité et Humidité (côte à côte)
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(20);

    // Capacité
    QVBoxLayout* capaciteCol = new QVBoxLayout();
    capaciteCol->setSpacing(8);
    QLabel* capaciteLabel = new QLabel("Capacité (Kg)");
    capaciteLabel->setFont(labelFont);
    capaciteLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    capaciteCol->addWidget(capaciteLabel);

    capEdit = new QLineEdit();
    capEdit->setPlaceholderText("800");
    capEdit->setFont(inputFont);
    capEdit->setFixedHeight(50);
    capEdit->setStyleSheet(getInputStyle());
    capaciteCol->addWidget(capEdit);
    row1->addLayout(capaciteCol);

    // Humidité
    QVBoxLayout* humiditeCol = new QVBoxLayout();
    humiditeCol->setSpacing(8);
    QLabel* humiditeLabel = new QLabel("Humidité (%)");
    humiditeLabel->setFont(labelFont);
    humiditeLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    humiditeCol->addWidget(humiditeLabel);

    humEdit = new QLineEdit();
    humEdit->setPlaceholderText("65");
    humEdit->setFont(inputFont);
    humEdit->setFixedHeight(50);
    humEdit->setStyleSheet(getInputStyle());
    humiditeCol->addWidget(humEdit);
    row1->addLayout(humiditeCol);

    formLayout->addLayout(row1);
    formLayout->addSpacing(10);

    // Température
    QLabel* tempLabel = new QLabel("Température (°C)");
    tempLabel->setFont(labelFont);
    tempLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(tempLabel);

    tempEdit = new QLineEdit();
    tempEdit->setPlaceholderText("-4");
    tempEdit->setFont(inputFont);
    tempEdit->setFixedHeight(50);
    tempEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(tempEdit);

    formLayout->addSpacing(10);

    // Statut
    QLabel* statusLabel = new QLabel("Statut");
    statusLabel->setFont(labelFont);
    statusLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(statusLabel);

    statusBox = new QComboBox();
    statusBox->addItems({"Disponible", "Occupé", "Maintenance"});
    statusBox->setFont(inputFont);
    statusBox->setFixedHeight(50);
    statusBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(statusBox);

    formLayout->addSpacing(10);

    // Poisson
    QLabel* fishLabel = new QLabel("Poisson");
    fishLabel->setFont(labelFont);
    fishLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(fishLabel);

    fishBox = new QComboBox();
    fishBox->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon"});
    fishBox->setFont(inputFont);
    fishBox->setFixedHeight(50);
    fishBox->setStyleSheet(getInputStyle());
    formLayout->addWidget(fishBox);

    formLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    buttonLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Annuler");
    QFont btnFont("Segoe UI", 13, QFont::Medium);
    cancelBtn->setFont(btnFont);
    cancelBtn->setFixedSize(140, 50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E8EEF5;
            color: #5A6C7D;
            border: none;
            border-radius: 10px;
            padding: 12px 30px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #D8DEE5;
        }
        QPushButton:pressed {
            background-color: #C8CED5;
        }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton("Enregistrer");
    QFont saveBtnFont("Segoe UI", 13, QFont::Bold);
    saveBtn->setFont(saveBtnFont);
    saveBtn->setFixedSize(160, 50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 35px;
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #4A89DC;
        }
        QPushButton:pressed {
            background-color: #3B77C4;
        }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(saveBtn);

    formLayout->addLayout(buttonLayout);

    // Ajouter le contenu au scroll area
    scrollArea->setWidget(content);

    // Ajouter le scroll area au layout principal
    mainLayout->addWidget(scrollArea);
}

QString AddFrigoDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox {
            background-color: #F8F9FA;
            border: 2px solid #E1E8ED;
            border-radius: 10px;
            padding: 12px 15px;
            color: #2C3E50;
            font-size: 12px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #5D9CEC;
            background-color: white;
        }
        QLineEdit:hover, QComboBox:hover {
            border: 2px solid #B8D4F1;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #5A6C7D;
            margin-right: 12px;
        }
    )";
}

void AddFrigoDialog::populateFields()
{
    if (!frigoData) return;

    refEdit->setText(frigoData->reference);
    capEdit->setText(frigoData->capacite);
    humEdit->setText(frigoData->humidite);
    tempEdit->setText(frigoData->temperature);

    int statusIndex = statusBox->findText(frigoData->statut);
    if (statusIndex >= 0) {
        statusBox->setCurrentIndex(statusIndex);
    }

    int fishIndex = fishBox->findText(frigoData->poisson);
    if (fishIndex >= 0) {
        fishBox->setCurrentIndex(fishIndex);
    }
}

Frigo AddFrigoDialog::getData() const
{
    Frigo frigo;
    frigo.reference = refEdit->text();
    frigo.capacite = capEdit->text();
    frigo.humidite = humEdit->text();
    frigo.temperature = tempEdit->text();
    frigo.statut = statusBox->currentText();
    frigo.poisson = fishBox->currentText();

    return frigo;
}

// Ces méthodes sont conservées pour compatibilité avec l'ancien code
void AddFrigoDialog::setData(QString id, QString cap, QString hum,
                             QString temp, QString stat, QString fish)
{
    capEdit->setText(cap);
    humEdit->setText(hum);
    tempEdit->setText(temp);
    statusBox->setCurrentText(stat);
    fishBox->setCurrentText(fish);
}

QString AddFrigoDialog::getId(){ return ""; } // L'ID est maintenant géré ailleurs
QString AddFrigoDialog::getCap(){ return capEdit->text(); }
QString AddFrigoDialog::getHum(){ return humEdit->text(); }
QString AddFrigoDialog::getTemp(){ return tempEdit->text(); }
QString AddFrigoDialog::getStatus(){ return statusBox->currentText(); }
QString AddFrigoDialog::getFish(){ return fishBox->currentText(); }
