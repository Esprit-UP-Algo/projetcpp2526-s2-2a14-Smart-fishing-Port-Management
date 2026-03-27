#define LIVRAISONWINDOW_CPP
#include "Livraisonwindow.h"
#include <algorithm>
#include "AddLivraisonDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QDate>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QFile>
#include <QSqlRecord>
#include <QSqlError>
#include "LivraisonStatisticsDialog.h"


LivraisonWindow::LivraisonWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Initial load from database
    populateTable();

    updateStats();
    // populateTable(); // REMOVED redundant call
    loadStyleSheet();
}

void LivraisonWindow::loadStyleSheet()
{
    QFile file(":/style/livraison.css");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString style = stream.readAll();
        if (centralWidget()) {
            centralWidget()->setStyleSheet(style);
        }
        this->setStyleSheet(style); // Also keep it on window for direct usage
        qDebug() << "Successfully loaded CSS from resources";
    } else {
        qDebug() << "Failed to load CSS from resources:" << file.errorString();
    }
}




LivraisonWindow::~LivraisonWindow()
{
}

void LivraisonWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Livraisons");
    
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Content area (since this widget is used as a page in MainWindow's StackedWidget)
    QWidget* contentArea = createContentArea();
    mainLayout->addWidget(contentArea);
}

QWidget* LivraisonWindow::createContentArea()
{
    QWidget* content = new QWidget();
    content->setObjectName("livraisonContentArea");

    QVBoxLayout* layout = new QVBoxLayout(content);

    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    // Header
    QFrame* header = createHeader();
    layout->addWidget(header);

    // Filter Toolbar (restored)
    layout->addWidget(createToolbar());

    // Stats Area
    QFrame* statsArea = createStatsArea();
    layout->addWidget(statsArea);

    // Table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* LivraisonWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Livraisons");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Coordination des transports et logistique");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color:#6b7280;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    /* Actions */
    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(45);
        btn->setMinimumWidth(160);
        btn->setStyleSheet(QString(
            "QPushButton{ background:%1; color:white; border:none; border-radius:12px; padding:0 20px; }"
            "QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* addBtn   = makeBtn("➕  Nouvelle Livraison", "#2563EB", "#1D4ED8");

    connect(statsBtn, &QPushButton::clicked, this, &LivraisonWindow::onShowStatistics);
    connect(addBtn, &QPushButton::clicked, this, &LivraisonWindow::onAddLivraison);

    lay->addWidget(statsBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* LivraisonWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet(R"(
        QFrame {
            background: white;
            border-radius: 14px;
            border: 1.5px solid #e2e8f0;
        }
    )");
    bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bar->setMinimumHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    /* Search bar */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher par date, adresse ou statut...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &LivraisonWindow::onSearch);
    lay->addWidget(searchInput);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Date (Récent)", "Date (Ancien)", "Prix ↑", "Prix ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LivraisonWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

QFrame* LivraisonWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setObjectName("livraisonTableCard");

    QVBoxLayout* layout = new QVBoxLayout(card);

    layout->setContentsMargins(3, 3, 3, 3);

    // Container blanc pour le tableau
    QFrame* whiteContainer = new QFrame();
    whiteContainer->setObjectName("livraisonWhiteContainer");

    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);

    containerLayout->setContentsMargins(25, 25, 25, 25);

    setupTable();
    containerLayout->addWidget(table);

    layout->addWidget(whiteContainer);

    return card;
}

QFrame* LivraisonWindow::createStatsArea()
{
    QFrame* statsArea = new QFrame();
    statsArea->setObjectName("statsArea");
    
    QHBoxLayout* layout = new QHBoxLayout(statsArea);
    layout->setSpacing(20);
    layout->setContentsMargins(0, 0, 0, 0);

    // Total Deliveries Card
    QFrame* totalCard = new QFrame();
    totalCard->setProperty("class", "stats-card");
    totalCard->setFixedHeight(120);
    
    QHBoxLayout* totalLayout = new QHBoxLayout(totalCard);
    totalLayout->setContentsMargins(25, 15, 25, 15);
    
    QVBoxLayout* totalLabels = new QVBoxLayout();
    QLabel* totalTitle = new QLabel("Total Livraisons");
    totalTitle->setProperty("class", "stats-label");
    totalDeliveriesLabel = new QLabel("0");
    totalDeliveriesLabel->setProperty("class", "stats-value");
    totalLabels->addWidget(totalTitle);
    totalLabels->addWidget(totalDeliveriesLabel);
    
    QLabel* totalIcon = new QLabel("📦");
    totalIcon->setProperty("class", "stats-icon");
    
    totalLayout->addLayout(totalLabels);
    totalLayout->addStretch();
    totalLayout->addWidget(totalIcon);
    
    // Efficiency Card
    QFrame* efficiencyCard = new QFrame();
    efficiencyCard->setProperty("class", "stats-card");
    efficiencyCard->setFixedHeight(120);
    
    QHBoxLayout* effLayout = new QHBoxLayout(efficiencyCard);
    effLayout->setContentsMargins(25, 15, 25, 15);
    
    QVBoxLayout* effLabels = new QVBoxLayout();
    QLabel* effTitle = new QLabel("Efficacité (Livrées)");
    effTitle->setProperty("class", "stats-label");
    efficiencyLabel = new QLabel("0%");
    efficiencyLabel->setProperty("class", "stats-value");
    effLabels->addWidget(effTitle);
    effLabels->addWidget(efficiencyLabel);
    
    QLabel* effIcon = new QLabel("📈");
    effIcon->setProperty("class", "stats-icon");
    
    effLayout->addLayout(effLabels);
    effLayout->addStretch();
    effLayout->addWidget(effIcon);

    layout->addWidget(totalCard);
    layout->addWidget(efficiencyCard);

    return statsArea;
}

void LivraisonWindow::updateStats()
{
    int total = 0;
    int delivered = 0;
    
    // Scoping the query ensures the handle is released immediately
    {
        QSqlQuery query("SELECT STATUT FROM LIVRAISONS");
        while (query.next()) {
            total++;
            if (query.value(0).toString() == "Livré") {
                delivered++;
            }
        }
    }
    
    double efficiency = (total > 0) ? (static_cast<double>(delivered) / total * 100.0) : 0.0;
    
    totalDeliveriesLabel->setText(QString::number(total));
    efficiencyLabel->setText(QString::number(efficiency, 'f', 1) + "%");
}

void LivraisonWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Date", "Adresse", "Statut", "Transport", "Prix", "Actions"});

    /* Responsive columns (matching Bateau styling) */
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Adresse stretch

    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);
    table->setFrameShape(QFrame::NoFrame);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFont hFont("Segoe UI", 11, QFont::Bold);
    table->horizontalHeader()->setFont(hFont);
    table->horizontalHeader()->setFixedHeight(50);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    table->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 16px;
            gridline-color: #d1d5db;
        }
        QTableWidget::item {
            padding: 12px;
            border-right: 1px solid #d1d5db;
            border-bottom: 1px solid #d1d5db;
            color: #1f2937;
            background-color: white;
            font-family: 'Segoe UI';
            font-size: 11pt;
        }
        QTableWidget::item:selected {
            background-color: #EBF5FF;
            color: #2563EB;
        }
        QHeaderView::section {
            background-color: #d1d5db;
            color: #1f2937;
            padding: 12px;
            border: none;
            font-weight: 600;
            font-family: 'Segoe UI';
            font-size: 11pt;
        }
    )");

    table->setObjectName("livraisonTable");

    table->setColumnWidth(0, 130);   // Date
    table->setColumnWidth(2, 140);   // Statut
    table->setColumnWidth(3, 160);   // Transport
    table->setColumnWidth(4, 120);   // Prix
    table->setColumnWidth(5, 160);   // Actions (Widened for buttons)
}

void LivraisonWindow::populateTable(const QString& filterText, const QString& sortCritere, const QString& sortOrdre)
{
    table->setRowCount(0);
    QSqlQueryModel* model;
    
    if (!sortCritere.isEmpty()) {
        model = livraisons.trier(sortCritere, sortOrdre);
    } else if (filterText.isEmpty()) {
        model = livraisons.afficher();
    } else {
        model = livraisons.rechercher(filterText);
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 60);

        QString id = model->record(i).value("ID").toString();
        QString date = model->record(i).value("Date").toDate().toString("dd/MM/yyyy");
        QString adresse = model->record(i).value("Adresse").toString();
        QString statut = model->record(i).value("STATUT").toString();
        QString transport = model->record(i).value("Transport").toString();
        QString prix = model->record(i).value("Prix").toString();
        if (!prix.endsWith(" DT")) prix += " DT";

        table->setItem(row, 0, new QTableWidgetItem(date));
        table->setItem(row, 1, new QTableWidgetItem(adresse));
        table->setCellWidget(row, 2, createStatusBadge(statut));
        table->setItem(row, 3, new QTableWidgetItem(transport));
        table->setItem(row, 4, new QTableWidgetItem(prix));
        // We store the ID in the first column's toolTip or data for easy access
        table->item(row, 0)->setData(Qt::UserRole, id); 
        table->setCellWidget(row, 5, createActionButtons(row)); 
    }
    delete model;
    updateStats();
}

QWidget* LivraisonWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* badge = new QLabel(status);
    badge->setFixedHeight(30);
    badge->setAlignment(Qt::AlignCenter);

    badge->setProperty("class", "status-badge");
    if (status == "Livré") {
        badge->setProperty("class", "status-badge status-livre");
    } else if (status == "En cours") {
        badge->setProperty("class", "status-badge status-en-cours");
    } else if (status == "Annulé" || status == "Canceled") {
        badge->setProperty("class", "status-badge status-annule");
    } else {
        badge->setProperty("class", "status-badge status-en-attente");
    }
    // Re-polish to apply styles from the new properties
    badge->style()->unpolish(badge);
    badge->style()->polish(badge);

    
    layout->addWidget(badge);
    return widget;
}

QWidget* LivraisonWindow::createActionButtons(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);

    QPushButton* editBtn = new QPushButton("✏️");
    QPushButton* deleteBtn = new QPushButton("🗑️");
    QPushButton* pdfBtn = new QPushButton("📄");

    editBtn->setFixedSize(36, 36);
    deleteBtn->setFixedSize(36, 36);
    pdfBtn->setFixedSize(36, 36);
    
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    pdfBtn->setCursor(Qt::PointingHandCursor);
    
    editBtn->setProperty("class", "action-btn edit-btn");
    deleteBtn->setProperty("class", "action-btn delete-btn");
    pdfBtn->setProperty("class", "action-btn pdf-btn");

    // Force style refresh for dynamic buttons
    for (QPushButton* btn : {editBtn, deleteBtn, pdfBtn}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }


    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditLivraison(row); });
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteLivraison(row); });
    connect(pdfBtn, &QPushButton::clicked, [this, row]() { onExportPDF(row); });

    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    layout->addWidget(pdfBtn);

    return widget;
}

QString LivraisonWindow::generateLivraisonId()
{
    int maxId = 0;
    // Explicit scoping for the query
    {
        QSqlQuery query("SELECT MAX(IDLIVRAISON) FROM LIVRAISONS");
        if (query.next()) {
            maxId = query.value(0).toInt();
        }
    }
    return QString("LIV%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void LivraisonWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void LivraisonWindow::onAddLivraison()
{
    AddLivraisonDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison data = dialog.getData(); 
        data.setID(generateLivraisonId());
        
        if (data.ajouter()) {
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Impossible d'ajouter la livraison.\n\nErreur Base de Données: " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onEditLivraison(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    QString id = table->item(row, 0)->data(Qt::UserRole).toString();

    // We need to fetch current data to pass to dialog
    QSqlQuery query;
    query.prepare("SELECT * FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    int idNum = id.startsWith("LIV") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    
    if (!query.exec() || !query.next()) return;

    ::Livraison currentData; // The struct used by dialog
    currentData.setID(id);
    currentData.setDate(query.value("DATELIVRAISON").toDate().toString("dd/MM/yyyy"));
    currentData.setAdresse(query.value("ADRESSELIVRAISON").toString());
    currentData.setStatut(query.value("STATUT").toString());
    currentData.setTransport(query.value("TYPETRANSPORT").toString());
    currentData.setVehicule(query.value("VEHICULE").toString());
    currentData.setPrix(query.value("PRIXLIVRAISON").toString());
    currentData.setDuree(query.value("DUREE").toInt());

    AddLivraisonDialog dialog(this, &currentData);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison updatedData = dialog.getData();
        updatedData.setID(id);
        
        if (updatedData.modifier(id)) {
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Impossible de modifier la livraison.\n\nErreur Base de Données: " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onDeleteLivraison(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    QString id = table->item(row, 0)->data(Qt::UserRole).toString();

    if (QMessageBox::question(this, "Confirmation", "Supprimer cette livraison ?") == QMessageBox::Yes) {
        if (livraisons.supprimer(id)) {
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Impossible de supprimer la livraison.\n\nErreur Base de Données: " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onSort(int index)
{
    QString critere = "";
    QString ordre = "ASC";

    if (index == 1) { // Date (Récent)
        critere = "DATELIV"; ordre = "DESC";
    } else if (index == 2) { // Date (Ancien)
        critere = "DATELIV"; ordre = "ASC";
    } else if (index == 3) { // Prix ↑
        critere = "PRIX"; ordre = "ASC";
    } else if (index == 4) { // Prix ↓
        critere = "PRIX"; ordre = "DESC";
    }

    populateTable(searchInput->text(), critere, ordre);
}

void LivraisonWindow::onExportPDF(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    QString id = table->item(row, 0)->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    int idNum = id.startsWith("LIV") ? id.mid(3).toInt() : id.toInt();
    query.bindValue(":id", idNum);
    
    if (!query.exec() || !query.next()) return;

    Livraison liv;
    liv.setID(id);
    liv.setDate(query.value("DATELIVRAISON").toDate().toString("dd/MM/yyyy"));
    liv.setAdresse(query.value("ADRESSELIVRAISON").toString());
    liv.setStatut(query.value("STATUT").toString());
    liv.setTransport(query.value("TYPETRANSPORT").toString());
    liv.setVehicule(query.value("VEHICULE").toString());
    liv.setPrix(query.value("PRIXLIVRAISON").toString());
    liv.setDuree(query.value("DUREE").toInt());

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", 
                                                    QString("Livraison_%1.pdf").arg(liv.getID()),
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));

    QPainter painter(&writer);
    painter.setPen(Qt::black);
    
    // Header
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.drawText(QRect(0, 50, 5000, 100), Qt::AlignCenter, "REÇU DE LIVRAISON");
    
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(100, 200, 4900, 200);

    // Content
    painter.setFont(QFont("Segoe UI", 12));
    int y = 400;
    painter.drawText(500, y, "ID Livraison :");
    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.drawText(2000, y, liv.getID());
    
    y += 200;
    painter.setFont(QFont("Segoe UI", 12));
    painter.drawText(500, y, "Date :");
    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.drawText(2000, y, liv.getDate());
    
    y += 200;
    painter.setFont(QFont("Segoe UI", 12));
    painter.drawText(500, y, "Adresse :");
    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.drawText(2000, y, liv.getAdresse());

    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.drawLine(100, y + 200, 4900, y + 200);

    painter.end();

    QMessageBox::information(this, "Export PDF", "Le reçu de livraison a été exporté avec succès !");
}

void LivraisonWindow::onShowStatistics()
{
    QMap<QString, int> statusData;
    QMap<QString, int> vanTripsData;
    QMap<QString, QPair<int, int>> vanTimeMetrics; // vehicle -> {totalDuration, tripCount}

    QSqlQuery query("SELECT STATUT, VEHICULE, DUREE FROM LIVRAISONS");
    while (query.next()) {
        QString statut = query.value(0).toString();
        QString vehicule = query.value(1).toString();
        int duree = query.value(2).toInt();

        statusData[statut]++;
        
        QString van = vehicule.isEmpty() ? "Inconnu" : vehicule;
        vanTripsData[van]++;
        
        vanTimeMetrics[van].first += duree;
        vanTimeMetrics[van].second++;
    }

    QMap<QString, double> avgTimeData;
    for (auto it = vanTimeMetrics.begin(); it != vanTimeMetrics.end(); ++it) {
        if (it.value().second > 0) {
            avgTimeData[it.key()] = (double)it.value().first / it.value().second;
        }
    }

    LivraisonStatisticsDialog dialog(statusData, vanTripsData, avgTimeData, this);
    dialog.exec();
}

void LivraisonWindow::onExportAllPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter tout en PDF", 
                                                    "Rapport_Livraisons.pdf",
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));

    QPainter painter(&writer);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));

    // Header
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.drawText(QRect(0, 50, 6000, 100), Qt::AlignCenter, "RAPPORT GLOBAL DES LIVRAISONS");
    
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(100, 200, 5900, 200);

    painter.setFont(QFont("Segoe UI", 10));
    int y = 350;
    
    // Column Headers
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(QColor("#475569"));
    painter.drawText(200, y, "ID");
    painter.drawText(800, y, "Date");
    painter.drawText(1600, y, "Adresse");
    painter.drawText(3600, y, "Statut");
    painter.drawText(4400, y, "Van");
    painter.drawText(5200, y, "Prix");
    y += 150;
    
    painter.setPen(QPen(QColor("#e2e8f0"), 1));
    painter.drawLine(200, y, 5800, y);
    y += 150;

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(Qt::black);
    // Columns: IDLIVRAISON, DATELIVRAISON, ADRESSELIVRAISON, STATUT, VEHICULE, PRIXLIVRAISON
    QSqlQuery query("SELECT 'LIV' || LPAD(IDLIVRAISON, 3, '0'), DATELIVRAISON, ADRESSELIVRAISON, STATUT, VEHICULE, PRIXLIVRAISON FROM LIVRAISONS");
    while (query.next()) {
        if (y > 9000) { 
            writer.newPage();
            y = 100;
        }
        painter.drawText(200, y, query.value(0).toString());
        painter.drawText(800, y, query.value(1).toDate().toString("dd/MM/yyyy"));
        painter.drawText(1600, y, query.value(2).toString().left(35));
        painter.drawText(3600, y, query.value(3).toString());
        painter.drawText(4400, y, query.value(4).toString().left(15));
        painter.drawText(5200, y, query.value(5).toString());
        y += 200;
        
        painter.setPen(QPen(QColor("#f1f5f9"), 1));
        painter.drawLine(200, y - 50, 5800, y - 50);
        painter.setPen(Qt::black);
        
    }

    painter.end();

    QMessageBox::information(this, "Export PDF", "Le rapport global a été exporté avec succès !");
}
