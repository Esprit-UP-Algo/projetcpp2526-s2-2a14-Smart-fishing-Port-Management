#include "Livraisonwindow.h"
#include "AddLivraisonDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QDate>

LivraisonWindow::LivraisonWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Sample data
    livraisons.append({"LIV001", "2024-02-10", "123 Rue de la Marine, Tunis", "En cours", "Camion", "150 DT"});
    livraisons.append({"LIV002", "2024-02-11", "Port de Sfax, Zone Industrielle", "Livré", "Bateau", "500 DT"});
    livraisons.append({"LIV003", "2024-02-12", "Marché Central, Sousse", "En attente", "Camion Frigo", "200 DT"});

    populateTable();
}

LivraisonWindow::~LivraisonWindow()
{
}

void LivraisonWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Livraisons");
    
    QWidget* centralWidget = new QWidget(this);
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
    content->setStyleSheet("background-color: #F0F4F8;");

    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    // Header
    QFrame* header = createHeader();
    layout->addWidget(header);

    // Table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* LivraisonWindow::createHeader()
{
    QFrame* header = new QFrame();
    header->setStyleSheet("background: transparent;");
    header->setFixedHeight(120);

    QVBoxLayout* layout = new QVBoxLayout(header);
    layout->setSpacing(15);

    // Title
    QLabel* title = new QLabel("Gestion des Livraisons");
    QFont titleFont("Segoe UI", 28, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2C3E50;");
    layout->addWidget(title);

    // Search and Add button row
    QHBoxLayout* actionRow = new QHBoxLayout();

    // Search
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher une livraison...");
    QFont searchFont("Segoe UI", 12);
    searchInput->setFont(searchFont);
    searchInput->setFixedHeight(50);
    searchInput->setFixedWidth(400);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 12px;
            padding: 12px 20px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #5D9CEC;
        }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &LivraisonWindow::onSearch);
    actionRow->addWidget(searchInput);

    actionRow->addStretch();

    // Add button
    QPushButton* addBtn = new QPushButton("➕  Nouvelle Livraison");
    QFont btnFont("Segoe UI", 13, QFont::Bold);
    addBtn->setFont(btnFont);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(50);
    addBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 12px 30px;
        }
        QPushButton:hover {
            background-color: #4A89DC;
        }
    )");
    connect(addBtn, &QPushButton::clicked, this, &LivraisonWindow::onAddLivraison);
    actionRow->addWidget(addBtn);

    layout->addLayout(actionRow);

    return header;
}

QFrame* LivraisonWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            border-radius: 20px;
            padding: 3px;
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(3, 3, 3, 3);

    // Container blanc pour le tableau
    QFrame* whiteContainer = new QFrame();
    whiteContainer->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 17px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    setupTable();
    containerLayout->addWidget(table);

    layout->addWidget(whiteContainer);

    return card;
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
        QHeaderView::section:first {
            border-top-left-radius: 14px;
        }
        QHeaderView::section:last {
            border-top-right-radius: 14px;
        }
    )");

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

    QString style = "padding: 0 15px; border-radius: 15px; font-weight: bold; font-size: 10pt;";
    if (status == "Livré") {
        style += "background-color: #dcfce7; color: #15803d;";
    } else if (status == "En cours") {
        style += "background-color: #fef9c3; color: #a16207;";
    } else {
        style += "background-color: #f1f5f9; color: #475569;";
    }
    badge->setStyleSheet(style);
    
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

    editBtn->setFixedSize(35, 35);
    deleteBtn->setFixedSize(35, 35);
    
    editBtn->setStyleSheet("background-color: #fef3c7; border: none; border-radius: 8px; font-size: 14pt;");
    deleteBtn->setStyleSheet("background-color: #fee2e2; border: none; border-radius: 8px; font-size: 14pt;");

    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditLivraison(row); });
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteLivraison(row); });

    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);

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
        populateTable(searchInput->text());
    }
}

void LivraisonWindow::onDeleteLivraison(int row)
{
    if (QMessageBox::question(this, "Confirmation", "Supprimer cette livraison ?") == QMessageBox::Yes) {
        livraisons.removeAt(row);
        populateTable(searchInput->text());
    }
}
