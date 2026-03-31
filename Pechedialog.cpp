#include "pechedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QRegularExpression>

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
    setWindowTitle(isEdit ? "Modifier un Lot de Pêche" : "Ajouter un Lot de Pêche");
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
    header->setFixedHeight(100);
    header->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            padding: 20px;
        }
    )");
    QVBoxLayout* headerVLayout = new QVBoxLayout(header);
    headerVLayout->setContentsMargins(30, 10, 30, 10);
    headerVLayout->setSpacing(5);

    QLabel* title = new QLabel(isEdit ? "Modifier Lot de Pêche" : "Ajouter Lot de Pêche");
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: white;");
    headerVLayout->addWidget(title);

    QLabel* subTitle = new QLabel(isEdit ? "✏️  Modifier les informations du lot" : "📋  Informations du lot de pêche");
    subTitle->setFont(QFont("Segoe UI", 11));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerVLayout->addWidget(subTitle);

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

    formLayout->addSpacing(5);

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

    refErrorLabel = new QLabel("⚠️ Format invalide (lettres, chiffres, tirets uniquement).");
    refErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    refErrorLabel->hide();
    formLayout->addWidget(refErrorLabel);

    connect(referenceInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            refErrorLabel->setText("⚠️ Champ obligatoire.");
            refErrorLabel->show(); 
            referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;"); 
            return; 
        }
        QRegularExpression rx("^[a-zA-Z0-9-]*$");
        if(!rx.match(text).hasMatch()) {
            refErrorLabel->setText("⚠️ Format invalide (lettres, chiffres, tirets uniquement).");
            refErrorLabel->show();
            referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            refErrorLabel->hide();
            referenceInput->setStyleSheet(getInputStyle());
        }
    });

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

    qteErrorLabel = new QLabel("⚠️ Veuillez saisir un nombre valide (> 0).");
    qteErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    qteErrorLabel->hide();
    formLayout->addWidget(qteErrorLabel);

    connect(quantiteInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.isEmpty()) { 
            qteErrorLabel->setText("⚠️ Champ obligatoire.");
            qteErrorLabel->show(); 
            quantiteInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;"); 
            return; 
        }
        QRegularExpression rx("^[0-9]*[.,]?[0-9]*$");
        if(!rx.match(text).hasMatch()) {
            qteErrorLabel->setText("⚠️ Veuillez saisir un nombre valide (> 0).");
            qteErrorLabel->show();
            quantiteInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;");
        } else {
            qteErrorLabel->hide();
            quantiteInput->setStyleSheet(getInputStyle());
        }
    });

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

    formLayout->addSpacing(10);

    // Bateau
    QLabel* boatLabel = new QLabel("Bateau");
    boatLabel->setFont(labelFont);
    boatLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(boatLabel);

    boatCombo = new QComboBox();
    boatCombo->setFont(inputFont);
    boatCombo->setFixedHeight(50);
    boatCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(boatCombo);

    formLayout->addSpacing(10);

    // Frigo
    QLabel* fridgeLabel = new QLabel("Frigo (Destination)");
    fridgeLabel->setFont(labelFont);
    fridgeLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(fridgeLabel);

    QHBoxLayout* fridgeLayout = new QHBoxLayout();
    fridgeCombo = new QComboBox();
    fridgeCombo->setFont(inputFont);
    fridgeCombo->setFixedHeight(50);
    fridgeCombo->setStyleSheet(getInputStyle());
    fridgeLayout->addWidget(fridgeCombo, 1);

    QPushButton* suggestBtn = new QPushButton("🔧 Suggérer");
    suggestBtn->setFixedSize(140, 50);
    suggestBtn->setCursor(Qt::PointingHandCursor);
    suggestBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #34C988;
            color: white;
            border: none;
            border-radius: 10px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #2EB177; }
    )");
    connect(suggestBtn, &QPushButton::clicked, this, &PecheDialog::onSuggestStorage);
    fridgeLayout->addWidget(suggestBtn);

    formLayout->addLayout(fridgeLayout);

    // Initialisation des combos
    QSqlQuery bQuery("SELECT IdBateau, NomBateau FROM BATEAUX");
    while (bQuery.next()) boatCombo->addItem(bQuery.value(1).toString(), bQuery.value(0));
    
    QSqlQuery fQuery("SELECT IDFRIGO, REFERENCE FROM FRIGOS");
    while (fQuery.next()) fridgeCombo->addItem(fQuery.value(1).toString(), fQuery.value(0));

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
    connect(saveBtn, &QPushButton::clicked, this, &PecheDialog::onSave);
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

    referenceInput->setText(pecheData->getReference());

    int especeIndex = especeCombo->findText(pecheData->getEspece());
    if (especeIndex >= 0) {
        especeCombo->setCurrentIndex(especeIndex);
    }

    quantiteInput->setText(pecheData->getQuantiteKg());

    // Parse date
    QStringList dateParts = pecheData->getDateCapture().split("/");
    if (dateParts.size() == 3) {
        dateInput->setDate(QDate(dateParts[2].toInt(), dateParts[1].toInt(), dateParts[0].toInt()));
    } else {
        QDate dt = QDate::fromString(pecheData->getDateCapture(), Qt::ISODate);
        if (dt.isValid()) dateInput->setDate(dt);
    }

    int bIdx = boatCombo->findData(pecheData->getIdBateau());
    if (bIdx >= 0) boatCombo->setCurrentIndex(bIdx);
    
    int fIdx = fridgeCombo->findData(pecheData->getIdFrigo());
    if (fIdx >= 0) fridgeCombo->setCurrentIndex(fIdx);
}

void PecheDialog::onSave()
{
    if (validateInputs()) {
        accept();
    }
}

bool PecheDialog::validateInputs()
{
    // Contrôle de saisie obligatoire dans le code C++ (Consigne cours)
    QString ref = referenceInput->text().trimmed();
    QString qte = quantiteInput->text().trimmed();
    bool isValid = true;
    QString errorStyle = "border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C; font-size: 12px;";

    if (ref.isEmpty()) {
        refErrorLabel->setText("⚠️ Champ obligatoire.");
        refErrorLabel->show();
        referenceInput->setStyleSheet(errorStyle);
        isValid = false;
    } else {
        QRegularExpression rxRef("^[a-zA-Z0-9-]*$");
        if(!rxRef.match(ref).hasMatch()) {
            refErrorLabel->setText("⚠️ Format invalide (lettres, chiffres, tirets uniquement).");
            refErrorLabel->show();
            referenceInput->setStyleSheet(errorStyle);
            isValid = false;
        }
    }

    if (qte.isEmpty()) {
        qteErrorLabel->setText("⚠️ Champ obligatoire.");
        qteErrorLabel->show();
        quantiteInput->setStyleSheet(errorStyle);
        isValid = false;
    } else {
        bool ok;
        double val = qte.toDouble(&ok);
        if (!ok || val <= 0) {
            qteErrorLabel->setText("⚠️ Veuillez saisir un nombre valide (> 0).");
            qteErrorLabel->show();
            quantiteInput->setStyleSheet(errorStyle);
            isValid = false;
        }
    }

    if (!isValid) {
        showError("Veuillez corriger les champs en rouge avant d'enregistrer.");
        return false;
    }

    if (boatCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un bateau.");
        return false;
    }

    if (fridgeCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un frigo.");
        return false;
    }

    return true;
}

void PecheDialog::showError(const QString& msg)
{
    QMessageBox::warning(this, "Validation", msg);
}

void PecheDialog::onSuggestStorage()
{
    QString esp = especeCombo->currentText();
    double qte = quantiteInput->text().toDouble();
    
    if (qte <= 0) {
        showError("Veuillez saisir une quantité valide pour suggérer un stockage.");
        return;
    }

    // [LOGIQUE INNOVANTE] Algorithme de sélection automatique du stockage optimal
    // On cherche un frigo Disponible, du même type, avec assez de place et la meilleure température.
    QSqlQuery query;
    query.prepare("SELECT IDFRIGO, REFERENCE, TEMPERATURE, (CAPACITE - OCCUPATION) as SPACE "
                  "FROM FRIGOS "
                  "WHERE STATUT = 'Disponible' AND TYPE_POISSON = :esp AND (CAPACITE - OCCUPATION) >= :qte "
                  "ORDER BY TEMPERATURE ASC, SPACE DESC");
    query.bindValue(":esp", esp);
    query.bindValue(":qte", qte);

    if (query.exec() && query.next()) {
        QString id = query.value("IDFRIGO").toString();
        int idx = fridgeCombo->findData(id);
        if (idx >= 0) {
            fridgeCombo->setCurrentIndex(idx);
            QMessageBox::information(this, "Automatisation", 
                QString("Stockage optimal trouvé : %1\nTempérature : %2°C\nEspace restant : %3 Kg")
                .arg(query.value("REFERENCE").toString())
                .arg(query.value("TEMPERATURE").toString())
                .arg(query.value("SPACE").toString()));
            return;
        }
    }

    // Sinon, on cherche n'importe quel frigo Disponible avec assez de place
    QSqlQuery backup;
    backup.prepare("SELECT IDFRIGO, REFERENCE FROM FRIGOS "
                   "WHERE STATUT = 'Disponible' AND (CAPACITE - OCCUPATION) >= :qte "
                   "ORDER BY (CAPACITE - OCCUPATION) DESC");
    backup.bindValue(":qte", qte);
    
    if (backup.exec() && backup.next()) {
        QString id = backup.value("IDFRIGO").toString();
        int idx = fridgeCombo->findData(id);
        if (idx >= 0) {
            fridgeCombo->setCurrentIndex(idx);
            QMessageBox::information(this, "Automatisation", 
                "Aucun frigo spécifique trouvé. Stockage par défaut suggéré : " + backup.value("REFERENCE").toString());
        }
    } else {
        QMessageBox::warning(this, "Stockage", "Aucun stockage adapté trouvé. Veuillez libérer un frigo.");
    }
}

Peche PecheDialog::getData() const
{
    Peche peche;
    peche.setReference(referenceInput->text().trimmed());
    peche.setEspece(especeCombo->currentText());
    peche.setQuantiteKg(quantiteInput->text().trimmed());
    peche.setDateCapture(dateInput->date().toString("dd/MM/yyyy"));
    peche.setIdBateau(boatCombo->currentData().toString());
    peche.setIdFrigo(fridgeCombo->currentData().toString());

    return peche;
}
