#include "pechedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QTimer>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDoubleValidator>

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

    QLabel* subTitle = new QLabel(isEdit ? "✏️  Modifier les informations du lot" : "🎣  Informations du lot de pêche");
    subTitle->setFont(QFont("Segoe UI", 11));
    subTitle->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerVLayout->addWidget(subTitle);

    mainLayout->addWidget(header);

    // Scroll Area pour le formulaire
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: white; border: none; }"
        "QScrollBar:vertical { background: #F0F4F8; width: 10px; border-radius: 5px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #5D9CEC; border-radius: 5px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #3b82f6; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

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
    
    // VALIDATION DE FORMAT (REF-YYYY-NOMBRE)
    QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
    QRegularExpressionValidator* refValidator = new QRegularExpressionValidator(refRegex, this);
    referenceInput->setValidator(refValidator);
    
    formLayout->addWidget(referenceInput);

    refErrorLabel = new QLabel("⚠️ La référence ne peut pas être vide.");
    refErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    refErrorLabel->hide();
    formLayout->addWidget(refErrorLabel);

    connect(referenceInput, &QLineEdit::textChanged, this, [=](const QString &text){
        QRegularExpression fullRegex("^REF-\\d{4}-\\d+$");
        if(text.trimmed().isEmpty()) {
            refErrorLabel->setText("⚠️ La référence ne peut pas être vide.");
            refErrorLabel->show();
            referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
        } else if (!fullRegex.match(text).hasMatch()) {
            refErrorLabel->setText("⚠️ Format invalide! Utilisez : REF-YYYY-NOMBRE (ex: REF-2025-1)");
            refErrorLabel->show();
            referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
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

    // Bloquer la saisie des lettres - autoriser uniquement les nombres positifs
    QDoubleValidator* qteValidator = new QDoubleValidator(0.01, 999999.99, 2, this);
    qteValidator->setNotation(QDoubleValidator::StandardNotation);
    quantiteInput->setValidator(qteValidator);

    formLayout->addWidget(quantiteInput);

    qteErrorLabel = new QLabel("⚠️ La quantité doit être un nombre positif.");
    qteErrorLabel->setStyleSheet("color: #E74C3C; font-size: 13px; font-weight: bold; margin-top: -5px;");
    qteErrorLabel->hide();
    formLayout->addWidget(qteErrorLabel);

    connect(quantiteInput, &QLineEdit::textChanged, this, [=](const QString &text){
        bool ok;
        double val = text.trimmed().toDouble(&ok);
        if(text.trimmed().isEmpty() || !ok || val <= 0) {
            qteErrorLabel->show();
            quantiteInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
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
    fridgeCombo->setStyleSheet(getInputStyle());
    fridgeLayout->addWidget(fridgeCombo, 4);

    autoSelectBtn = new QPushButton("✨ Auto");
    autoSelectBtn->setToolTip("Choisir automatiquement le meilleur frigo");
    autoSelectBtn->setFixedWidth(100);
    autoSelectBtn->setFixedHeight(50);
    autoSelectBtn->setCursor(Qt::PointingHandCursor);
    autoSelectBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3498DB;
            color: white;
            border-radius: 10px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #2980B9; }
    )");
    fridgeLayout->addWidget(autoSelectBtn, 1);
    formLayout->addLayout(fridgeLayout);

    autoMsgLabel = new QLabel("");
    autoMsgLabel->setStyleSheet("color: #27AE60; font-size: 12px; font-weight: 500; margin-top: -5px;");
    autoMsgLabel->hide();
    formLayout->addWidget(autoMsgLabel);

    connect(autoSelectBtn, &QPushButton::clicked, this, &PecheDialog::autoSelectFridge);

    formLayout->addSpacing(10);

    // Pêcheur
    QLabel* fishermanLabel = new QLabel("Pêcheur (Responsable)");
    fishermanLabel->setFont(labelFont);
    fishermanLabel->setStyleSheet("color: #2C3E50; margin-bottom: 5px;");
    formLayout->addWidget(fishermanLabel);

    fishermanCombo = new QComboBox();
    fishermanCombo->setFont(inputFont);
    fishermanCombo->setFixedHeight(50);
    fishermanCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(fishermanCombo);

    // Initialisation des combos
    QSqlQuery bQuery("SELECT IdBateau, NomBateau FROM BATEAUX");
    while (bQuery.next()) boatCombo->addItem(bQuery.value(1).toString(), bQuery.value(0));

    // Peuple les pêcheurs
    QSqlQuery eQuery("SELECT ID_EMPLOYE, PRENOM || ' ' || NOM FROM EMPLOYEES WHERE \"POSITION\" = 'Pêcheur'");
    while (eQuery.next()) fishermanCombo->addItem(eQuery.value(1).toString(), eQuery.value(0));

    // Peuple les frigos filtrés par type de poisson
    refreshFridgeCombo(especeCombo->currentText());

    // Quand l'espèce change, re-filtrer les frigos
    connect(especeCombo, &QComboBox::currentTextChanged, this, [=](const QString &e){
        refreshFridgeCombo(e);
        autoMsgLabel->hide();
    });

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
    else if (fridgeCombo->count() > 0) fridgeCombo->setCurrentIndex(0);

    int eIdx = fishermanCombo->findData(pecheData->getIdPecheur());
    if (eIdx >= 0) fishermanCombo->setCurrentIndex(eIdx);
}

void PecheDialog::onSave()
{
    if (validateInputs()) {
        accept();
    }
}

bool PecheDialog::validateInputs()
{
    QString ref = referenceInput->text().trimmed();
    QString qte = quantiteInput->text().trimmed();
    bool isValid = true;

    // Validation Référence (Vérification et Format final)
    QRegularExpression fullRegex("^REF-\\d{4}-\\d+$");
    if (ref.isEmpty()) {
        refErrorLabel->setText("⚠️ La référence ne peut pas être vide.");
        refErrorLabel->show();
        referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
        isValid = false;
    } else if (!fullRegex.match(ref).hasMatch()) {
        refErrorLabel->setText("⚠️ Format invalide! Attendu : REF-annee-nombre (ex: REF-2026-001)");
        refErrorLabel->show();
        referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
        isValid = false;
    } else {
        // [NOUVEAU] Contrôle unicité via le modèle
        int currentId = -1;
        if (isEdit && pecheData) {
            QString idStr = pecheData->getIdLot();
            currentId = idStr.startsWith("LOT") ? idStr.mid(3).toInt() : idStr.toInt();
        }
        
        if (Peche::referenceExiste(ref, currentId)) {
            refErrorLabel->setText("⚠️ Cette référence est déjà utilisée.");
            refErrorLabel->show();
            referenceInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
            isValid = false;
        } else {
            refErrorLabel->hide();
            referenceInput->setStyleSheet(getInputStyle());
        }
    }

    // Validation Quantité
    bool ok;
    double val = qte.toDouble(&ok);
    if (qte.isEmpty() || !ok || val <= 0) {
        qteErrorLabel->show();
        quantiteInput->setStyleSheet("border: 2px solid #E74C3C; background-color: #FDEDEC; border-radius: 10px; padding: 12px 15px; color: #E74C3C;");
        isValid = false;
    } else {
        qteErrorLabel->hide();
        quantiteInput->setStyleSheet(getInputStyle());
    }

    if (boatCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un bateau.");
        isValid = false;
    }

    if (fridgeCombo->currentIndex() == -1 || fridgeCombo->currentData().toString().isEmpty()) {
        showError(QString("Aucun frigo compatible pour l'espèce '%1'. Veuillez d'abord créer un frigo de type '%1'.")
                      .arg(especeCombo->currentText()));
        isValid = false;
    }

    if (fishermanCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un pêcheur.");
        isValid = false;
    }

    if (!isValid && (ref.isEmpty() || qte.isEmpty() || !ok || val <= 0)) {
        showError("Veuillez corriger les erreurs dans le formulaire.");
    }

    return isValid;
}

void PecheDialog::showError(const QString& msg)
{
    QMessageBox::warning(this, "Validation", msg);
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
    peche.setIdPecheur(fishermanCombo->currentData().toString());

    return peche;
}

void PecheDialog::refreshFridgeCombo(const QString &espece)
{
    // Sauvegarder l'ID frigo actuellement sélectionné
    QString currentFrigoId = fridgeCombo->currentData().toString();

    fridgeCombo->clear();

    // Filtrer les frigos compatibles avec le type de poisson sélectionné
    QSqlQuery fQuery;
    fQuery.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, OCCUPATION "
                   "FROM FRIGOS "
                   "WHERE UPPER(TYPE_POISSON) = UPPER(:espece) "
                   "AND STATUT = 'Disponible' "
                   "ORDER BY REFERENCE");
    fQuery.bindValue(":espece", espece);

    if (fQuery.exec()) {
        while (fQuery.next()) {
            QString frigoId  = fQuery.value(0).toString();
            QString ref      = fQuery.value(1).toString();
            double  cap      = fQuery.value(2).toDouble();
            double  occ      = fQuery.value(3).toDouble();
            double  libre    = cap - occ;
            QString label    = QString("%1  (libre : %2 kg)").arg(ref).arg(libre, 0, 'f', 0);
            fridgeCombo->addItem(label, frigoId);
        }
    }

    if (fridgeCombo->count() == 0) {
        // Aucun frigo compatible — avertir l'utilisateur
        fridgeCombo->addItem(QString("⚠️ Aucun frigo disponible pour '%1'").arg(espece), "");
        fridgeCombo->setStyleSheet(
            fridgeCombo->styleSheet() +
            " QComboBox { border: 2px solid #E74C3C; background-color: #FDEDEC; color: #E74C3C; }"
        );
    } else {
        // Restaurer la sélection précédente si possible
        fridgeCombo->setStyleSheet(getInputStyle());
        int idx = fridgeCombo->findData(currentFrigoId);
        if (idx >= 0) fridgeCombo->setCurrentIndex(idx);
    }
}

void PecheDialog::autoSelectFridge()
{
    QString espece = especeCombo->currentText();
    bool ok;
    double qteNeeded = quantiteInput->text().toDouble(&ok);

    if (!ok || qteNeeded <= 0) {
        showError("Veuillez saisir une quantité valide avant l'auto-sélection.");
        return;
    }

    // Définition des températures idéales par espèce (Fraîcheur optimale)
    QMap<QString, double> idealTemps;
    idealTemps["Sardine"] = -2.0;
    idealTemps["Thon"]    = -18.0;
    idealTemps["Crevette"]= -20.0;
    idealTemps["Merlan"]  = 0.0;
    idealTemps["Saumon"]  = -4.0;
    
    double targetTemp = idealTemps.value(espece, -5.0); // -5 par défaut

    QSqlQuery query;
    query.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, OCCUPATION, TEMPERATURE "
                  "FROM FRIGOS "
                  "WHERE UPPER(TYPE_POISSON) = UPPER(:type) "
                  "AND STATUT = 'Disponible'");
    query.bindValue(":type", espece);

    if (!query.exec()) {
        showError("Erreur lors de la recherche de stockage.");
        return;
    }

    struct FrigoScore {
        QString id;
        QString ref;
        double tempDiff;
        double freeSpace;
    };

    QList<FrigoScore> candidates;
    while (query.next()) {
        double cap = query.value(2).toDouble();
        double occ = query.value(3).toDouble();
        double temp = query.value(4).toDouble();
        
        if ((cap - occ) >= qteNeeded) {
            candidates.append({
                query.value(0).toString(),
                query.value(1).toString(),
                qAbs(temp - targetTemp),
                cap - occ
            });
        }
    }

    if (candidates.isEmpty()) {
        showError(QString("Aucun frigo disponible avec assez d'espace (%1 kg) pour '%2'.").arg(qteNeeded).arg(espece));
        return;
    }

    // Trier par différence de température (Fraîcheur) puis par espace libre (Capacité)
    std::sort(candidates.begin(), candidates.end(), [](const FrigoScore& a, const FrigoScore& b) {
        if (a.tempDiff != b.tempDiff) return a.tempDiff < b.tempDiff;
        return a.freeSpace > b.freeSpace;
    });

    FrigoScore best = candidates.first();

    // Sélectionner dans le combo
    int idx = fridgeCombo->findData(best.id);
    if (idx >= 0) {
        fridgeCombo->setCurrentIndex(idx);
        autoMsgLabel->setText(QString("✨ Auto-sélection : %1 (Optimisé pour %2°C)").arg(best.ref).arg(targetTemp));
        autoMsgLabel->show();
        
        // Petit effet visuel sur le combo
        fridgeCombo->setStyleSheet(
            fridgeCombo->styleSheet() + 
            " QComboBox { border: 2px solid #27AE60; background-color: #EBF5FB; }"
        );
        QTimer::singleShot(2000, this, [=](){ 
            fridgeCombo->setStyleSheet(getInputStyle()); 
            autoMsgLabel->hide();
        });
    }
}
