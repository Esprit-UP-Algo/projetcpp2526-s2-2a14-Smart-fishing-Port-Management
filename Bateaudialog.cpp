#include "bateaudialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>

BateauDialog::BateauDialog(QWidget *parent, Bateau* bateauData)
    : QDialog(parent), bateauData(bateauData), isEdit(bateauData != nullptr)
{
    setupUi();

    if (isEdit) {
        populateFields();
    }
}

BateauDialog::~BateauDialog()
{
}

void BateauDialog::setupUi()
{
    setWindowTitle("Ajouter / Modifier Bateau");
    setFixedSize(700, 720);
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

    QLabel* title = new QLabel(isEdit ? "Modifier Bateau" : "Ajouter Bateau");
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: white;");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(40, 40);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
            font-size: 24px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
            border-radius: 20px;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

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

    // Nom du bateau et Immatriculation (côte à côte)
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(20);

    // Nom du bateau
    QVBoxLayout* nomCol = new QVBoxLayout();
    nomCol->setSpacing(8);
    QLabel* nomLabel = new QLabel("Nom du bateau");
    nomLabel->setFont(labelFont);
    nomLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    nomCol->addWidget(nomLabel);

    nomBateauInput = new QLineEdit();
    nomBateauInput->setPlaceholderText("Neptune");
    nomBateauInput->setFont(inputFont);
    nomBateauInput->setFixedHeight(50);
    nomBateauInput->setStyleSheet(getInputStyle());
    nomCol->addWidget(nomBateauInput);
    nameRow->addLayout(nomCol);

    // Immatriculation
    QVBoxLayout* immatCol = new QVBoxLayout();
    immatCol->setSpacing(8);
    QLabel* immatLabel = new QLabel("Immatriculation");
    immatLabel->setFont(labelFont);
    immatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    immatCol->addWidget(immatLabel);

    immatriculationInput = new QLineEdit();
    immatriculationInput->setPlaceholderText("TN-001");
    immatriculationInput->setFont(inputFont);
    immatriculationInput->setFixedHeight(50);
    immatriculationInput->setStyleSheet(getInputStyle());
    immatCol->addWidget(immatriculationInput);
    nameRow->addLayout(immatCol);

    formLayout->addLayout(nameRow);
    formLayout->addSpacing(10);

    // Capacité et Longueur (côte à côte)
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(20);

    QVBoxLayout* col1 = new QVBoxLayout();
    col1->setSpacing(8);
    QLabel* capaciteLabel = new QLabel("Capacité de pêche (tonnes)");
    capaciteLabel->setFont(labelFont);
    capaciteLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    col1->addWidget(capaciteLabel);

    capacitePecheInput = new QLineEdit();
    capacitePecheInput->setPlaceholderText("50");
    capacitePecheInput->setFont(inputFont);
    capacitePecheInput->setFixedHeight(50);
    capacitePecheInput->setStyleSheet(getInputStyle());
    col1->addWidget(capacitePecheInput);
    row1->addLayout(col1);

    QVBoxLayout* col2 = new QVBoxLayout();
    col2->setSpacing(8);
    QLabel* longueurLabel = new QLabel("Longueur (mètres)");
    longueurLabel->setFont(labelFont);
    longueurLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    col2->addWidget(longueurLabel);

    longueurInput = new QLineEdit();
    longueurInput->setPlaceholderText("20");
    longueurInput->setFont(inputFont);
    longueurInput->setFixedHeight(50);
    longueurInput->setStyleSheet(getInputStyle());
    col2->addWidget(longueurInput);
    row1->addLayout(col2);

    formLayout->addLayout(row1);
    formLayout->addSpacing(10);

    // Propriétaire
    QLabel* proprietaireLabel = new QLabel("Propriétaire");
    proprietaireLabel->setFont(labelFont);
    proprietaireLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(proprietaireLabel);

    proprietaireInput = new QLineEdit();
    proprietaireInput->setPlaceholderText("Ahmed Ben Ali");
    proprietaireInput->setFont(inputFont);
    proprietaireInput->setFixedHeight(50);
    proprietaireInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(proprietaireInput);

    formLayout->addSpacing(10);

    // État du bateau
    QLabel* etatLabel = new QLabel("État du bateau");
    etatLabel->setFont(labelFont);
    etatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(etatLabel);

    etatBateauCombo = new QComboBox();
    etatBateauCombo->addItems({"En mer", "Au port", "En maintenance"});
    etatBateauCombo->setFont(inputFont);
    etatBateauCombo->setFixedHeight(50);
    etatBateauCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(etatBateauCombo);

    formLayout->addSpacing(10);

    // Date dernière maintenance
    QLabel* dateLabel = new QLabel("Date dernière maintenance");
    dateLabel->setFont(labelFont);
    dateLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(dateLabel);

    dateMaintenanceInput = new QDateEdit();
    dateMaintenanceInput->setDate(QDate::currentDate());
    dateMaintenanceInput->setCalendarPopup(true);
    dateMaintenanceInput->setFont(inputFont);
    dateMaintenanceInput->setFixedHeight(50);
    dateMaintenanceInput->setStyleSheet(getInputStyle());
    dateMaintenanceInput->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(dateMaintenanceInput);

    formLayout->addSpacing(10);

    // Disponibilité
    QLabel* disponibleLabel = new QLabel("Disponibilité");
    disponibleLabel->setFont(labelFont);
    disponibleLabel->setStyleSheet("color: #2C3E50; margin-bottom: 10px;");
    formLayout->addWidget(disponibleLabel);

    QHBoxLayout* disponibleLayout = new QHBoxLayout();
    disponibleLayout->setSpacing(25);

    disponibleGroup = new QButtonGroup(this);

    disponibleOuiRadio = new QRadioButton("Oui");
    QFont radioFont("Segoe UI", 11);
    disponibleOuiRadio->setFont(radioFont);
    disponibleOuiRadio->setChecked(true);
    disponibleOuiRadio->setStyleSheet(getRadioStyle());
    disponibleOuiRadio->setFixedHeight(40);
    disponibleGroup->addButton(disponibleOuiRadio);
    disponibleLayout->addWidget(disponibleOuiRadio);

    disponibleNonRadio = new QRadioButton("Non");
    disponibleNonRadio->setFont(radioFont);
    disponibleNonRadio->setStyleSheet(getRadioStyle());
    disponibleNonRadio->setFixedHeight(40);
    disponibleGroup->addButton(disponibleNonRadio);
    disponibleLayout->addWidget(disponibleNonRadio);

    disponibleLayout->addStretch();
    formLayout->addLayout(disponibleLayout);

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

QString BateauDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F8F9FA;
            border: 2px solid #E1E8ED;
            border-radius: 10px;
            padding: 12px 15px;
            color: #2C3E50;
            font-size: 12px;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #5D9CEC;
            background-color: white;
        }
        QLineEdit:hover, QComboBox:hover, QDateEdit:hover {
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
        QDateEdit::drop-down {
            border: none;
            width: 30px;
        }
        QDateEdit::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #5A6C7D;
            margin-right: 12px;
        }
    )";
}

QString BateauDialog::getRadioStyle() const
{
    return R"(
        QRadioButton {
            color: #2C3E50;
            spacing: 10px;
            padding: 8px;
        }
        QRadioButton::indicator {
            width: 22px;
            height: 22px;
        }
        QRadioButton::indicator:unchecked {
            border: 3px solid #BDC3C7;
            border-radius: 11px;
            background-color: white;
        }
        QRadioButton::indicator:unchecked:hover {
            border: 3px solid #5D9CEC;
        }
        QRadioButton::indicator:checked {
            border: 3px solid #5D9CEC;
            border-radius: 11px;
            background-color: white;
        }
        QRadioButton::indicator:checked::after {
            width: 12px;
            height: 12px;
            border-radius: 6px;
            background-color: #5D9CEC;
        }
    )";
}

void BateauDialog::populateFields()
{
    if (!bateauData) return;

    nomBateauInput->setText(bateauData->nomBateau);
    immatriculationInput->setText(bateauData->immatriculation);
    capacitePecheInput->setText(bateauData->capacitePeche);
    longueurInput->setText(bateauData->longueur);
    proprietaireInput->setText(bateauData->proprietaire);

    int index = etatBateauCombo->findText(bateauData->etatBateau);
    if (index >= 0) {
        etatBateauCombo->setCurrentIndex(index);
    }

    // Parse date
    QStringList dateParts = bateauData->dateDerniereMaintenance.split("/");
    if (dateParts.size() == 3) {
        dateMaintenanceInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    }

    // Set disponibilité
    if (bateauData->disponible == "Oui") {
        disponibleOuiRadio->setChecked(true);
    } else {
        disponibleNonRadio->setChecked(true);
    }
}

Bateau BateauDialog::getData() const
{
    Bateau bateau;
    // ID sera généré automatiquement dans BateauWindow
    bateau.nomBateau = nomBateauInput->text();
    bateau.immatriculation = immatriculationInput->text();
    bateau.capacitePeche = capacitePecheInput->text();
    bateau.longueur = longueurInput->text();
    bateau.proprietaire = proprietaireInput->text();
    bateau.etatBateau = etatBateauCombo->currentText();
    bateau.dateDerniereMaintenance = dateMaintenanceInput->date().toString("dd/MM/yyyy");

    if (disponibleOuiRadio->isChecked()) {
        bateau.disponible = "Oui";
    } else {
        bateau.disponible = "Non";
    }

    return bateau;
}
