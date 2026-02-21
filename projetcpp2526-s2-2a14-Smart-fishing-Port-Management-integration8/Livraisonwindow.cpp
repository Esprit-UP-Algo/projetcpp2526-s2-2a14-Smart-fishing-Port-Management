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
#include <QTextStream>


LivraisonWindow::LivraisonWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Sample data
    livraisons.append({"LIV001", "2024-02-10", "123 Rue de la Marine, Tunis", "En cours", "Camion", "150 DT"});
    livraisons.append({"LIV002", "2024-02-11", "Port de Sfax, Zone Industrielle", "Livré", "Bateau", "500 DT"});
    livraisons.append({"LIV003", "2024-02-12", "Marché Central, Sousse", "En attente", "Camion Frigo", "200 DT"});

    updateStats();
    populateTable();
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
    QPushButton* pdfBtn   = makeBtn("📄  Exporter PDF",  "#059669", "#047857");
    QPushButton* addBtn   = makeBtn("➕  Nouvelle Livraison", "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &LivraisonWindow::onAddLivraison);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* LivraisonWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet(R"(
        QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }
    )");
    bar->setFixedHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    /* Search */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher une livraison par adresse, statut...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setStyleSheet(R"(
        QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; }
        QLineEdit:focus{ border:2px solid #2563EB; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &LivraisonWindow::onSearch);
    lay->addWidget(searchInput, 3);

    /* Divider */
    QFrame* div = new QFrame(); div->setFrameShape(QFrame::VLine);
    div->setStyleSheet("color:#e2e8f0;"); div->setFixedWidth(1);
    lay->addWidget(div);

    /* Sort label */
    QLabel* sortLabel = new QLabel("Trier par :");
    sortLabel->setFont(QFont("Segoe UI", 10, QFont::Medium));
    sortLabel->setStyleSheet("color:#64748b; margin-left:10px;");
    lay->addWidget(sortLabel);

    /* Sort combo */
    sortCombo = new QComboBox();
    sortCombo->setFont(QFont("Segoe UI", 10));
    sortCombo->setFixedHeight(45);
    sortCombo->setMinimumWidth(200);
    sortCombo->addItems({"Défaut", "Date (Récent)", "Date (Ancien)", "Prix ↑", "Prix ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox{ background:transparent; border:none; padding:4px 12px; color:#1f2937; font-weight:600; }
        QComboBox:hover { color:#2563EB; }
        QComboBox::drop-down{ border:none; width:30px; }
        QComboBox QAbstractItemView{
            background:white; border:1px solid #e2e8f0; border-radius:12px;
            selection-background-color:#eff6ff; selection-color:#2563EB; outline:none; padding:8px;
        }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LivraisonWindow::onSort);
    lay->addWidget(sortCombo, 2);

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
    int total = livraisons.size();
    int delivered = 0;
    
    for (const auto& liv : livraisons) {
        if (liv.statut == "Livré") {
            delivered++;
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
    table->setHorizontalHeaderLabels({"ID", "Date", "Adresse", "Statut", "Transport", "Actions"});

    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);

    QFont headerFont("Segoe UI", 11, QFont::Bold);
    table->horizontalHeader()->setFont(headerFont);
    table->horizontalHeader()->setFixedHeight(50);

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

    table->setColumnWidth(0, 90);    // ID


    table->setColumnWidth(0, 90);    // ID
    table->setColumnWidth(1, 130);   // Date
    table->setColumnWidth(2, 280);   // Adresse
    table->setColumnWidth(3, 140);   // Statut
    table->setColumnWidth(4, 150);   // Transport
}

void LivraisonWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);

    for (int i = 0; i < livraisons.size(); ++i) {
        const Livraison& liv = livraisons[i];

        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!liv.id.toLower().contains(searchLower) &&
                !liv.adresse.toLower().contains(searchLower) &&
                !liv.statut.toLower().contains(searchLower)) {
                continue;
            }
        }

        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 60);

        table->setItem(row, 0, new QTableWidgetItem(liv.id));
        table->setItem(row, 1, new QTableWidgetItem(liv.date));
        table->setItem(row, 2, new QTableWidgetItem(liv.adresse));
        table->setCellWidget(row, 3, createStatusBadge(liv.statut));
        table->setItem(row, 4, new QTableWidgetItem(liv.transport));
        table->setCellWidget(row, 5, createActionButtons(i));
    }
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

    editBtn->setFixedSize(35, 35);
    deleteBtn->setFixedSize(35, 35);
    pdfBtn->setFixedSize(35, 35);
    
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    pdfBtn->setCursor(Qt::PointingHandCursor);
    
    editBtn->setProperty("class", "action-btn edit-btn");
    deleteBtn->setProperty("class", "action-btn delete-btn");
    pdfBtn->setProperty("class", "action-btn pdf-btn");


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
    return QString("LIV%1").arg(livraisons.size() + 1, 3, 10, QChar('0'));
}

void LivraisonWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void LivraisonWindow::onAddLivraison()
{
    AddLivraisonDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison newLiv = dialog.getData();
        newLiv.id = generateLivraisonId();
        newLiv.date = QDate::currentDate().toString("yyyy-MM-dd");
        livraisons.append(newLiv);
        updateStats();
        populateTable(searchInput->text());
    }
}

void LivraisonWindow::onEditLivraison(int row)
{
    if (row < 0 || row >= livraisons.size()) return;

    AddLivraisonDialog dialog(this, &livraisons[row]);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison updatedData = dialog.getData();
        updatedData.id = livraisons[row].id;
        updatedData.date = livraisons[row].date;
        livraisons[row] = updatedData;
        updateStats();
        populateTable(searchInput->text());
    }
}

void LivraisonWindow::onDeleteLivraison(int row)
{
    if (QMessageBox::question(this, "Confirmation", "Supprimer cette livraison ?") == QMessageBox::Yes) {
        livraisons.removeAt(row);
        updateStats();
        populateTable(searchInput->text());
    }
}

void LivraisonWindow::onSort(int index)
{
    if (index == 1) { // Date (Récent)
        std::sort(livraisons.begin(), livraisons.end(), [](const Livraison &a, const Livraison &b) {
            return QDate::fromString(a.date, "yyyy-MM-dd") > QDate::fromString(b.date, "yyyy-MM-dd");
        });
    } else if (index == 2) { // Date (Ancien)
        std::sort(livraisons.begin(), livraisons.end(), [](const Livraison &a, const Livraison &b) {
            return QDate::fromString(a.date, "yyyy-MM-dd") < QDate::fromString(b.date, "yyyy-MM-dd");
        });
    }

    populateTable(searchInput->text());
}

void LivraisonWindow::onExportPDF(int row)
{
    if (row < 0 || row >= livraisons.size()) return;
    const Livraison& liv = livraisons[row];

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", 
                                                    QString("Livraison_%1.pdf").arg(liv.id),
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));

    QPainter painter(&writer);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));

    painter.drawText(100, 100, "CONFIRMATION DE LIVRAISON");
    
    painter.setFont(QFont("Segoe UI", 12));
    int y = 250;
    painter.drawText(100, y, "ID Livraison: " + liv.id); y += 150;
    painter.drawText(100, y, "Date: " + liv.date); y += 150;
    painter.drawText(100, y, "Adresse: " + liv.adresse); y += 150;
    painter.drawText(100, y, "Transport: " + liv.transport); y += 150;
    painter.drawText(100, y, "Statut: " + liv.statut); y += 150;
    painter.drawText(100, y, "Prix Total: " + liv.prix);

    painter.end();

    QMessageBox::information(this, "Export PDF", "La confirmation de livraison a été exportée avec succès !");
}
