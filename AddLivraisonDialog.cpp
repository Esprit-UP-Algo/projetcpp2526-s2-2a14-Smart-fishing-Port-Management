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
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QTimer>
#include <cstdlib>


AddLivraisonDialog::AddLivraisonDialog(QWidget *parent, Livraison* livraisonData)
    : QDialog(parent), livraisonData(livraisonData), isEdit(livraisonData != nullptr), 
      addressValidating(false), addressFound(true), waitingForSave(false)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &AddLivraisonDialog::onAddressValidationFinished);

    addressDebounceTimer = new QTimer(this);
    addressDebounceTimer->setSingleShot(true);
    connect(addressDebounceTimer, &QTimer::timeout, this, &AddLivraisonDialog::onAddressDebounceTimeout);

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

    QLabel* infoLabel = new QLabel("Veuillez saisir l'adresse, le nom du van, le type de transport et le prix.");
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
            QLineEdit:hover, QComboBox:hover, QTextEdit:hover, QDateEdit:hover {
                border: 2px solid #CBD5E1;
            }
            /* Hover States for Validation */
            QLineEdit:hover[state="error"], QTextEdit:hover[state="error"] {
                border: 2px solid #EF4444;
            }
            QLineEdit:hover[state="success"], QTextEdit:hover[state="success"] {
                border: 2px solid #10B981;
            }
            QLineEdit:focus, QComboBox:focus, QTextEdit:focus, QDateEdit:focus {
                border: 2px solid #5D9CEC;
                background-color: white;
            }
            /* Error styling for fields */
            *[state="error"] {
                border: 2px solid #FCA5A5;
                background-color: #FEF2F2;
            }
            /* Success styling for fields */
            *[state="success"] {
                border: 2px solid #A7F3D0;
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

    // ID Field (New)
    QLabel* idLabel = new QLabel("🆔  ID Livraison (Doit commencer par LIV-)");
    idLabel->setFont(labelFont);
    idLabel->setStyleSheet("color: #2C3E50; margin-bottom: 2px;");
    formLayout->addWidget(idLabel);

    idEdit = new QLineEdit();
    idEdit->setPlaceholderText("LIV-1234");
    idEdit->setFixedHeight(45);
    idEdit->setStyleSheet(getInputStyle());
    if (isEdit && livraisonData) {
        idEdit->setText(livraisonData->getID());
        idEdit->setEnabled(false); // Can't change ID on edit
    } else {
        idEdit->setText("LIV-");
    }
    formLayout->addWidget(idEdit);

    errorId = new QLabel("⚠ Cet ID existe déjà ou est invalide (Format: LIV-...)");
    errorId->setStyleSheet("color: #EF4444; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
    errorId->setVisible(false);
    formLayout->addWidget(errorId);
    connect(idEdit, &QLineEdit::textChanged, this, &AddLivraisonDialog::onIdChanged);

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

    errorAdresse = new QLabel("⚠ L'adresse ne peut pas être vide");
    errorAdresse->setStyleSheet("color: #EF4444; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
    errorAdresse->setVisible(false);
    formLayout->addWidget(errorAdresse);
    connect(adresseEdit, &QTextEdit::textChanged, this, &AddLivraisonDialog::onAdresseChanged);

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
    
    vehiculeEdit = new QComboBox();
    vehiculeEdit->addItems({
        "Van Agile-01",
        "Van Swift-02",
        "Camionnette PortFlow-03",
        "Fourgon Logistics-04",
        "Bateau de Transport-05"
    });
    vehiculeEdit->setFixedHeight(45);
    vehiculeEdit->setStyleSheet(getInputStyle());
    formLayout->addWidget(vehiculeEdit);

    errorVehicule = new QLabel(""); // Keep initialized but hidden/empty as it's no longer used
    errorVehicule->setVisible(false);

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
    QLabel* prixLabel = new QLabel("💰  Prix (DT / $ / €)");
    prixLabel->setFont(labelFont);
    prixLabel->setStyleSheet("color: #2C3E50;");
    prixCol->addWidget(prixLabel);
    prixEdit = new QLineEdit();
    prixEdit->setPlaceholderText("Ex: 150 DT, 50$ ou 45€");
    prixEdit->setFixedHeight(45);
    prixEdit->setStyleSheet(getInputStyle());
    prixCol->addWidget(prixEdit);

    errorPrix = new QLabel("⚠ Doit finir par DT, $ ou €");
    errorPrix->setStyleSheet("color: #EF4444; font-size: 8pt; font-weight: bold;");
    errorPrix->setVisible(false);
    prixCol->addWidget(errorPrix);
    connect(prixEdit, &QLineEdit::textChanged, this, &AddLivraisonDialog::onPrixChanged);
    row2->addLayout(prixCol);

    formLayout->addLayout(row2);

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
    connect(saveBtn, &QPushButton::clicked, this, &AddLivraisonDialog::handleSave);
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
    vehiculeEdit->setCurrentText(livraisonData->getVehicule());
    transportEdit->setCurrentText(livraisonData->getTransport());
    QString p = livraisonData->getPrix();
    if (!p.isEmpty() && !p.endsWith("DT") && !p.endsWith("$") && !p.endsWith("€")) {
        p += " DT";
    }
    prixEdit->setText(p);
}

void AddLivraisonDialog::onIdChanged() {
    QString id = idEdit->text().trimmed();
    bool validFormat = id.startsWith("LIV-") && id.length() > 4;
    bool exists = false;
    
    if (validFormat && !isEdit) {
        exists = Livraison::idExists(id);
    }
    
    bool ok = validFormat && !exists;
    updateFieldStyle(idEdit, ok);
    
    if (exists) {
        errorId->setText("⚠ Cet ID existe déjà dans la base de données");
        errorId->setVisible(true);
    } else if (!validFormat) {
        errorId->setText("⚠ L'ID doit commencer par 'LIV-' suivi de caractères");
        errorId->setVisible(true);
    } else {
        errorId->setVisible(false);
    }
}

Livraison AddLivraisonDialog::getData() const
{
    Livraison data;
    data.setDate(dateEdit->date().toString("dd/MM/yyyy"));
    data.setAdresse(adresseEdit->toPlainText());
    data.setVehicule(vehiculeEdit->currentText());
    data.setTransport(transportEdit->currentText());
    
    QString prix = prixEdit->text().trimmed();
    data.setPrix(prix);
    data.setID(idEdit->text().trimmed());
    
    // Status is automated: "En attente" by default for new, preserved for edit
    if (isEdit && livraisonData) {
        data.setStatut(livraisonData->getStatut());
    } else {
        data.setStatut("En attente");
    }
    
    // Automatic duration calculation (randomized for demo/completeness)
    if (isEdit && livraisonData) {
        data.setDuree(livraisonData->getDuree());
    } else {
        data.setDuree(15 + (rand() % 45)); // 15 to 60 mins
    }
    
    return data;
}

void AddLivraisonDialog::onAdresseChanged() {
    QString addr = adresseEdit->toPlainText().trimmed();
    
    // Address was modified, reset validation state
    addressFound = false;
    waitingForSave = false;
    
    if (addr.isEmpty()) {
        addressDebounceTimer->stop();
        updateFieldStyle(adresseEdit, false);
        errorAdresse->setText("⚠ L'adresse ne peut pas être vide");
        errorAdresse->setStyleSheet("color: #EF4444; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
        errorAdresse->setVisible(true);
    } else {
        adresseEdit->setProperty("state", "");
        adresseEdit->style()->unpolish(adresseEdit);
        adresseEdit->style()->polish(adresseEdit);
        
        errorAdresse->setText("⌛ Recherche et vérification de la localisation...");
        errorAdresse->setStyleSheet("color: #64748B; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
        errorAdresse->setVisible(true);
        
        addressDebounceTimer->start(1200);
    }
}

void AddLivraisonDialog::onAddressDebounceTimeout() {
    QString addr = adresseEdit->toPlainText().trimmed();
    if (!addr.isEmpty()) {
        validateAddressViaAPI(addr);
    }
}

void AddLivraisonDialog::onPrixChanged() {
    QString p = prixEdit->text();
    if (p.contains("-")) {
        p.remove("-");
        prixEdit->setText(p);
        return; // Signal will re-trigger
    }
    p = p.trimmed();
    bool valid = !p.isEmpty() && (p.endsWith("DT") || p.endsWith("$") || p.endsWith("€"));
    updateFieldStyle(prixEdit, valid);
    errorPrix->setVisible(!valid);
}

void AddLivraisonDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

bool AddLivraisonDialog::validateInputs() {
    bool ok = true;
    
    // Validate ID
    QString id = idEdit->text().trimmed();
    bool idOk = id.startsWith("LIV-") && id.length() > 4;
    if (idOk && !isEdit) {
        idOk = !Livraison::idExists(id);
    }
    if (!idOk) {
        updateFieldStyle(idEdit, false);
        errorId->setVisible(true);
        ok = false;
    }

    if (adresseEdit->toPlainText().trimmed().isEmpty()) {
        updateFieldStyle(adresseEdit, false);
        errorAdresse->setVisible(true);
        ok = false;
    }
    
    QString p = prixEdit->text().trimmed();
    if (p.isEmpty() || !(p.endsWith("DT") || p.endsWith("$") || p.endsWith("€"))) {
        updateFieldStyle(prixEdit, false);
        errorPrix->setVisible(true);
        ok = false;
    }
    
    return ok;
}

void AddLivraisonDialog::validateAddressViaAPI(const QString& address) {
    if (addressValidating) return;
    
    addressValidating = true;
    errorAdresse->setText("⌛ Validation de l'adresse par satellite...");
    errorAdresse->setStyleSheet("color: #2563EB; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
    errorAdresse->setVisible(true);

    QString url = "https://nominatim.openstreetmap.org/search?q=" + QUrl::toPercentEncoding(address) + "&format=json&limit=1";
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "PortFlowApp/1.0"); // Nominatim requires User-Agent
    networkManager->get(request);
}

void AddLivraisonDialog::onAddressValidationFinished(QNetworkReply* reply) {
    addressValidating = false;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response);
        QJsonArray array = json.array();
        
        addressFound = !array.isEmpty();
        
        if (addressFound) {
            errorAdresse->setText("✅ Adresse valide confirmée par satellite !");
            errorAdresse->setStyleSheet("color: #10B981; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
            errorAdresse->setVisible(true);
            updateFieldStyle(adresseEdit, true);
            
            if (waitingForSave) handleSave();
        } else {
            errorAdresse->setText("⚠ Adresse introuvable ou invalide");
            errorAdresse->setStyleSheet("color: #EF4444; font-size: 9pt; font-weight: bold; margin-top: 5px; margin-left: 5px;");
            errorAdresse->setVisible(true);
            updateFieldStyle(adresseEdit, false);
        }
    } else {
        // Network error, assume valid for now but show warning?
        addressFound = true; // Fallback
        errorAdresse->setVisible(false);
        updateFieldStyle(adresseEdit, true);
        if (waitingForSave) handleSave();
    }
    reply->deleteLater();
}

void AddLivraisonDialog::handleSave() {
    if (validateInputs()) {
        // Only proceed if address has been validated through API
        if (!addressFound && !addressValidating) {
            waitingForSave = true;
            validateAddressViaAPI(adresseEdit->toPlainText().trimmed());
            return;
        }
        
        if (addressValidating) {
            waitingForSave = true;
            return;
        }
        
        accept();
    }
}
