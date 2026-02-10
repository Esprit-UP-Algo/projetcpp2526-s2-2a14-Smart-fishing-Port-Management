#include "bateauwindow.h"
#include "bateaudialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>

BateauWindow::BateauWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Données initiales de test
    Bateau b1{"B001", "Neptune", "TN-001", "50", "20", "Ahmed Ben Ali", "Au port", "15/01/2025", "Oui"};
    Bateau b2{"B002", "Poséidon", "TN-002", "70", "25", "Mohamed Trabelsi", "En mer", "10/01/2025", "Oui"};
    Bateau b3{"B003", "Triton", "TN-003", "45", "18", "Karim Gharbi", "En maintenance", "20/12/2024", "Non"};

    bateaux.append(b1);
    bateaux.append(b2);
    bateaux.append(b3);

    populateTable();
}

BateauWindow::~BateauWindow()
{
}

void BateauWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Bateaux");
    setMinimumSize(1400, 800);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame* sidebar = createSidebar();
    mainLayout->addWidget(sidebar);

    // Content area
    QWidget* contentArea = createContentArea();
    mainLayout->addWidget(contentArea, 1);
}

QFrame* BateauWindow::createSidebar()
{
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(280);
    sidebar->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #2B5EA6,
                stop:1 #5D9CEC
            );
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Logo section
    QFrame* logoFrame = new QFrame();
    logoFrame->setFixedHeight(160);
    logoFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setContentsMargins(20, 15, 20, 15);

    // Cadre gris avec demi-arc pour le logo
    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(180, 100);
    logoContainer->setStyleSheet(R"(
        QFrame {
            background-color: #d1d5db;
            border-radius: 22px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(logoContainer);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    containerLayout->setAlignment(Qt::AlignCenter);

    // Logo image
    QLabel* logoLabel = new QLabel();
    QPixmap logoPix("C:/images/logo.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Logo chargé depuis: C:/images/logo.png";
    } else {
        logoLabel->setText("⛵");
        logoLabel->setStyleSheet(R"(
            QLabel {
                font-size: 50px;
                color: #2C3E50;
            }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Attention: Logo non trouvé à C:/images/logo.png - utilisation emoji";
    }

    logoLabel->setStyleSheet("background: transparent;");
    containerLayout->addWidget(logoLabel);

    logoLayout->addWidget(logoContainer);

    layout->addWidget(logoFrame);

    // Navigation
    QFrame* navFrame = new QFrame();
    navFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* navLayout = new QVBoxLayout(navFrame);
    navLayout->setSpacing(8);
    navLayout->setContentsMargins(20, 20, 20, 20);

    navLayout->addWidget(createNavButton("🏠", "Dashboard"));
    navLayout->addWidget(createNavButton("⛵", "Bateaux", true));
    navLayout->addWidget(createNavButton("🐟", "Pêche"));
    navLayout->addWidget(createNavButton("👥", "Employés"));
    navLayout->addWidget(createNavButton("🧊", "Frigos"));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();

    // Quit button
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);

    return sidebar;
}

QPushButton* BateauWindow::createNavButton(const QString& icon, const QString& text, bool isActive, bool isLogout)
{
    QPushButton* btn = new QPushButton(icon + "  " + text);
    QFont btnFont("Segoe UI", 12, QFont::Medium);
    btn->setFont(btnFont);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(55);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.1);
                color: white;
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
            }
            QPushButton:hover {
                background-color: rgba(239, 68, 68, 0.8);
            }
        )");
        connect(btn, &QPushButton::clicked, this, &BateauWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.25);
                color: white;
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
                font-weight: bold;
            }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: rgba(255, 255, 255, 0.9);
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.15);
            }
        )");
    }

    return btn;
}

QWidget* BateauWindow::createContentArea()
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

QFrame* BateauWindow::createHeader()
{
    QFrame* header = new QFrame();
    header->setStyleSheet("background: transparent;");
    header->setFixedHeight(120);

    QVBoxLayout* layout = new QVBoxLayout(header);
    layout->setSpacing(15);

    // Title
    QLabel* title = new QLabel("Gestion des Bateaux");
    QFont titleFont("Segoe UI", 28, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2C3E50;");
    layout->addWidget(title);

    // Search and Add button row
    QHBoxLayout* actionRow = new QHBoxLayout();

    // Search
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher un bateau...");
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
    connect(searchInput, &QLineEdit::textChanged, this, &BateauWindow::onSearch);
    actionRow->addWidget(searchInput);

    actionRow->addStretch();

    // Add button
    QPushButton* addBtn = new QPushButton("➕  Ajouter un bateau");
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
        QPushButton:pressed {
            background-color: #3B77C4;
        }
    )");
    connect(addBtn, &QPushButton::clicked, this, &BateauWindow::onAddBateau);
    actionRow->addWidget(addBtn);

    layout->addLayout(actionRow);

    return header;
}

QFrame* BateauWindow::createTableCard()
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

void BateauWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        "ID", "Nom", "Immatriculation", "Capacité (t)", "Longueur (m)",
        "Propriétaire", "État", "Dernière Maint.", "Disponible", "Actions"
    });

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

    table->setColumnWidth(0, 80);   // ID
    table->setColumnWidth(1, 140);  // Nom
    table->setColumnWidth(2, 130);  // Immatriculation
    table->setColumnWidth(3, 100);  // Capacité
    table->setColumnWidth(4, 100);  // Longueur
    table->setColumnWidth(5, 150);  // Propriétaire
    table->setColumnWidth(6, 130);  // État
    table->setColumnWidth(7, 130);  // Dernière Maint.
    table->setColumnWidth(8, 100);  // Disponible
}

void BateauWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);

    QFont cellFont("Segoe UI", 11);

    for (int i = 0; i < bateaux.size(); ++i) {
        const Bateau& bateau = bateaux[i];

        // Filter
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!bateau.idBateau.toLower().contains(searchLower) &&
                !bateau.nomBateau.toLower().contains(searchLower) &&
                !bateau.immatriculation.toLower().contains(searchLower) &&
                !bateau.proprietaire.toLower().contains(searchLower)) {
                continue;
            }
        }

        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 65);

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(bateau.idBateau);
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        QFont idFont("Segoe UI", 11, QFont::Bold);
        idItem->setFont(idFont);
        idItem->setData(Qt::UserRole, i);
        table->setItem(row, 0, idItem);

        // Nom
        QTableWidgetItem* nomItem = new QTableWidgetItem(bateau.nomBateau);
        nomItem->setFont(cellFont);
        table->setItem(row, 1, nomItem);

        // Immatriculation
        QTableWidgetItem* immatItem = new QTableWidgetItem(bateau.immatriculation);
        immatItem->setFont(cellFont);
        table->setItem(row, 2, immatItem);

        // Capacité
        QTableWidgetItem* capaciteItem = new QTableWidgetItem(bateau.capacitePeche + " t");
        capaciteItem->setFont(cellFont);
        table->setItem(row, 3, capaciteItem);

        // Longueur
        QTableWidgetItem* longueurItem = new QTableWidgetItem(bateau.longueur + " m");
        longueurItem->setFont(cellFont);
        table->setItem(row, 4, longueurItem);

        // Propriétaire
        QTableWidgetItem* propItem = new QTableWidgetItem(bateau.proprietaire);
        propItem->setFont(cellFont);
        table->setItem(row, 5, propItem);

        // État
        table->setCellWidget(row, 6, createStatusBadge(bateau.etatBateau));

        // Date maintenance
        QTableWidgetItem* dateItem = new QTableWidgetItem(bateau.dateDerniereMaintenance);
        dateItem->setFont(cellFont);
        table->setItem(row, 7, dateItem);

        // Disponible
        table->setCellWidget(row, 8, createDisponibleBadge(bateau.disponible));

        // Actions
        table->setCellWidget(row, 9, createActionButtons(i));
    }
}

QWidget* BateauWindow::createStatusBadge(const QString& etat)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* badge = new QLabel(etat);
    QFont badgeFont("Segoe UI", 10, QFont::Medium);
    badge->setFont(badgeFont);
    badge->setFixedHeight(32);
    badge->setAlignment(Qt::AlignCenter);

    QString styleSheet;
    if (etat == "En mer") {
        styleSheet = R"(
            QLabel {
                background-color: #DBEAFE;
                color: #1E40AF;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else if (etat == "Au port") {
        styleSheet = R"(
            QLabel {
                background-color: #D1FAE5;
                color: #065F46;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else { // En maintenance
        styleSheet = R"(
            QLabel {
                background-color: #FEF3C7;
                color: #92400E;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    }

    badge->setStyleSheet(styleSheet);
    layout->addWidget(badge);

    return widget;
}

QWidget* BateauWindow::createDisponibleBadge(const QString& disponible)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* badge = new QLabel(disponible);
    QFont badgeFont("Segoe UI", 10, QFont::Medium);
    badge->setFont(badgeFont);
    badge->setFixedHeight(32);
    badge->setAlignment(Qt::AlignCenter);

    QString styleSheet;
    if (disponible == "Oui") {
        styleSheet = R"(
            QLabel {
                background-color: #D1FAE5;
                color: #065F46;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else {
        styleSheet = R"(
            QLabel {
                background-color: #FEE2E2;
                color: #991B1B;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    }

    badge->setStyleSheet(styleSheet);
    layout->addWidget(badge);

    return widget;
}

QWidget* BateauWindow::createActionButtons(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);

    // Edit button
    QPushButton* editBtn = new QPushButton("✏️");
    editBtn->setFixedSize(36, 36);
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FEF3C7;
            color: #92400E;
            border: none;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #FDE68A;
        }
    )");
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditBateau(row); });
    layout->addWidget(editBtn);

    // Delete button
    QPushButton* deleteBtn = new QPushButton("🗑️");
    deleteBtn->setFixedSize(36, 36);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FEE2E2;
            color: #991B1B;
            border: none;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #FECACA;
        }
    )");
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteBateau(row); });
    layout->addWidget(deleteBtn);

    return widget;
}

QString BateauWindow::generateBateauId()
{
    int maxId = 0;
    for (const Bateau& b : bateaux) {
        QString numStr = b.idBateau.mid(1);
        int num = numStr.toInt();
        if (num > maxId) {
            maxId = num;
        }
    }
    return QString("B%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void BateauWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void BateauWindow::onAddBateau()
{
    BateauDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Bateau newBateau = dialog.getData();
        newBateau.idBateau = generateBateauId();
        bateaux.append(newBateau);
        populateTable(searchInput->text());
        qDebug() << "Bateau ajouté:" << newBateau.nomBateau;
    }
}

void BateauWindow::onEditBateau(int row)
{
    if (row < 0 || row >= bateaux.size()) return;

    BateauDialog dialog(this, &bateaux[row]);
    if (dialog.exec() == QDialog::Accepted) {
        Bateau updatedBateau = dialog.getData();
        updatedBateau.idBateau = bateaux[row].idBateau;
        bateaux[row] = updatedBateau;
        populateTable(searchInput->text());
        qDebug() << "Bateau modifié:" << updatedBateau.nomBateau;
    }
}

void BateauWindow::onDeleteBateau(int row)
{
    if (row < 0 || row >= bateaux.size()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir supprimer le bateau '" + bateaux[row].nomBateau + "' ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Bateau supprimé:" << bateaux[row].nomBateau;
        bateaux.removeAt(row);
        populateTable(searchInput->text());
    }
}

void BateauWindow::onLogout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Quitter",
                                  "Voulez-vous vraiment quitter l'application ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Fermeture de l'application";
        this->close();
    }
}
