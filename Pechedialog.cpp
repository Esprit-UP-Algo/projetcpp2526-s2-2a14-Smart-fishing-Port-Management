#include "pechedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>

PecheDialog::PecheDialog(QWidget *parent, Peche* pecheData)
    : QDialog(parent), pecheData(pecheData), isEdit(pecheData != nullptr)
{
    setupUi();

    if (isEdit) {
        populateFields();
    }
}

PecheDialog::~PecheDialog()
{
}

void PecheDialog::setupUi()
{
    setWindowTitle("Ajouter / Modifier Lot de Pêche");
    setFixedSize(700, 600);
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

    QLabel* title = new QLabel(isEdit ? "Modifier Lot de Pêche" : "Ajouter Lot de Pêche");
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

    // Référence
    QLabel* refLabel = new QLabel("Référence");
    refLabel->setFont(labelFont);
    refLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(refLabel);

    referenceInput = new QLineEdit();
    referenceInput->setPlaceholderText("REF-2025-001");
    referenceInput->setFont(inputFont);
    referenceInput->setFixedHeight(50);
    referenceInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(referenceInput);

    formLayout->addSpacing(10);

    // Espèce
    QLabel* especeLabel = new QLabel("Espèce");
    especeLabel->setFont(labelFont);
    especeLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(especeLabel);

    especeCombo = new QComboBox();
    especeCombo->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon"});
    especeCombo->setFont(inputFont);
    especeCombo->setFixedHeight(50);
    especeCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(especeCombo);

    formLayout->addSpacing(10);

    // Quantité (Kg)
    QLabel* quantiteLabel = new QLabel("Quantité (Kg)");
    quantiteLabel->setFont(labelFont);
    quantiteLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(quantiteLabel);

    quantiteInput = new QLineEdit();
    quantiteInput->setPlaceholderText("450");
    quantiteInput->setFont(inputFont);
    quantiteInput->setFixedHeight(50);
    quantiteInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(quantiteInput);

    formLayout->addSpacing(10);

    // Date de capture
    QLabel* dateLabel = new QLabel("Date de Capture");
    dateLabel->setFont(labelFont);
    dateLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(dateLabel);

    dateInput = new QDateEdit();
    dateInput->setDate(QDate::currentDate());
    dateInput->setCalendarPopup(true);
    dateInput->setFont(inputFont);
    dateInput->setFixedHeight(50);
    dateInput->setStyleSheet(getInputStyle());
    dateInput->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(dateInput);

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

QString PecheDialog::getInputStyle() const
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

void PecheDialog::populateFields()
{
    if (!pecheData) return;

    referenceInput->setText(pecheData->reference);

    int especeIndex = especeCombo->findText(pecheData->espece);
    if (especeIndex >= 0) {
        especeCombo->setCurrentIndex(especeIndex);
    }

    quantiteInput->setText(pecheData->quantiteKg);

    // Parse date
    QStringList dateParts = pecheData->dateCapture.split("/");
    if (dateParts.size() == 3) {
        dateInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    }
}

Peche PecheDialog::getData() const
{
    Peche peche;
    peche.reference = referenceInput->text();
    peche.espece = especeCombo->currentText();
    peche.quantiteKg = quantiteInput->text();
    peche.dateCapture = dateInput->date().toString("dd/MM/yyyy");

    return peche;
}
