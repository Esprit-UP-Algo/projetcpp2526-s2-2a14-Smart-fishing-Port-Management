#include "dockswindow.h"
#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include "adddockdialog.h"

DocksWindow::DocksWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(1400, 800);
    setWindowTitle("PortFlow - Gestion des Docks");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    // Pre-fill docks list
    for (int i = 0; i < 12; ++i) {
        Dock d;
        d.id = QString("DK%1").arg(i + 1, 3, 10, QChar('0'));
        d.nom = "Dock " + QString::number(i + 1);
        d.capacite = "2-3 bateaux";
        d.tailleMax = "15m";
        d.statut = (i % 3 == 0) ? "Disponible" : ((i % 3 == 1) ? "Occupé" : "Maintenance");
        d.tarif = "€50/jour";
        d.client = (i % 3 == 1) ? "Sea Harvest Ltd" : "-";
        docks.append(d);
    }

    setupUI();
    setupDockTable();
    populateTable();
}

// ---------- SETUP UI ----------
void DocksWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(createSidebar());
    mainLayout->addWidget(createContentArea(), 1);
}

// ---------- SIDEBAR ----------
QFrame* DocksWindow::createSidebar()
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
    QPixmap logoPix("C:/Users/yoser/OneDrive/Bureau/logoportflow.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Logo chargé depuis: C:/images/logo.png";
    } else {
        logoLabel->setText("⚓");
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
    navLayout->addWidget(createNavButton("⛵", "Bateaux"));
    navLayout->addWidget(createNavButton("🐟", "Pêche"));
    navLayout->addWidget(createNavButton("👥", "Employés"));
    navLayout->addWidget(createNavButton("⚓", "Docks", true));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();

    // Quit button
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);

    return sidebar;
}

QPushButton* DocksWindow::createNavButton(const QString& icon, const QString& text,
                                          bool isActive, bool isLogout)
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
        connect(btn, &QPushButton::clicked, this, &DocksWindow::onLogout);
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

// ---------- CONTENT AREA ----------
QWidget* DocksWindow::createContentArea()
{
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #F0F4F8;");

    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    layout->addWidget(createHeader());
    layout->addWidget(createToolbar());
    layout->addWidget(createTableCard(), 1);

    return content;
}

// ---------- HEADER ----------
QFrame* DocksWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Docks");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Administration des emplacements et disponibilités");
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
    QPushButton* addBtn   = makeBtn("➕  Nouveau Dock",   "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &DocksWindow::onAddDock);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* DocksWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet(R"(
        QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }
    )");
    bar->setFixedHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    /* Search Input */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher un dock par numéro, type...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setStyleSheet(R"(
        QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; }
        QLineEdit:focus{ border:2px solid #2563EB; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &DocksWindow::onSearch);
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
    sortCombo->addItems({"Défaut", "Numéro ↑", "Numéro ↓", "Tarif ↑", "Tarif ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox{ background:transparent; border:none; padding:4px 12px; color:#1f2937; font-weight:600; }
        QComboBox:hover { color:#2563EB; }
        QComboBox::drop-down{ border:none; width:30px; }
        QComboBox QAbstractItemView{
            background:white; border:1px solid #e2e8f0; border-radius:12px;
            selection-background-color:#eff6ff; selection-color:#2563EB; outline:none; padding:8px;
        }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DocksWindow::onSort);
    lay->addWidget(sortCombo, 2);

    return bar;
}

// ---------- TABLE CARD ----------
QFrame* DocksWindow::createTableCard()
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

    dockTable = new QTableWidget();
    containerLayout->addWidget(dockTable);

    layout->addWidget(whiteContainer);

    return card;
}

// ---------- TABLE SETUP ----------
void DocksWindow::setupDockTable()
{
    dockTable->setColumnCount(8);
    dockTable->setHorizontalHeaderLabels({"ID", "Nom du Dock", "Capacité", "Taille Max", "Statut", "Tarif", "Client Actuel", "Actions"});

    dockTable->horizontalHeader()->setStretchLastSection(true);
    dockTable->verticalHeader()->setVisible(false);
    dockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    dockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dockTable->setShowGrid(true);
    dockTable->setAlternatingRowColors(false);

    QFont headerFont("Segoe UI", 11, QFont::Bold);
    dockTable->horizontalHeader()->setFont(headerFont);
    dockTable->horizontalHeader()->setFixedHeight(50);

    dockTable->setStyleSheet(R"(
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

    dockTable->setColumnWidth(0, 80);    // ID
    dockTable->setColumnWidth(1, 150);   // Nom du Dock
    dockTable->setColumnWidth(2, 130);   // Capacité
    dockTable->setColumnWidth(3, 120);   // Taille Max
    dockTable->setColumnWidth(4, 130);   // Statut
    dockTable->setColumnWidth(5, 110);   // Tarif
    dockTable->setColumnWidth(6, 180);   // Client Actuel
}

// ---------- POPULATE TABLE ----------
void DocksWindow::populateTable(const QString& filterText)
{
    dockTable->setRowCount(0);

    QFont cellFont("Segoe UI", 11);

    for (int i = 0; i < docks.size(); ++i) {
        const Dock& d = docks[i];

        // Filter
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!d.nom.toLower().contains(searchLower) &&
                !d.statut.toLower().contains(searchLower) &&
                !d.client.toLower().contains(searchLower) &&
                !d.id.toLower().contains(searchLower)) {
                continue;
            }
        }

        int row = dockTable->rowCount();
        dockTable->insertRow(row);
        dockTable->setRowHeight(row, 65);

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(d.id);
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        QFont idFont("Segoe UI", 11, QFont::Bold);
        idItem->setFont(idFont);
        idItem->setData(Qt::UserRole, i);
        dockTable->setItem(row, 0, idItem);

        // Nom du Dock
        QTableWidgetItem* nomItem = new QTableWidgetItem(d.nom);
        nomItem->setFont(cellFont);
        dockTable->setItem(row, 1, nomItem);

        // Capacité
        QTableWidgetItem* capaciteItem = new QTableWidgetItem(d.capacite);
        capaciteItem->setFont(cellFont);
        dockTable->setItem(row, 2, capaciteItem);

        // Taille Max
        QTableWidgetItem* tailleItem = new QTableWidgetItem(d.tailleMax);
        tailleItem->setFont(cellFont);
        dockTable->setItem(row, 3, tailleItem);

        // Statut
        dockTable->setCellWidget(row, 4, createStatusBadge(d.statut));

        // Tarif
        QTableWidgetItem* tarifItem = new QTableWidgetItem(d.tarif);
        tarifItem->setFont(cellFont);
        dockTable->setItem(row, 5, tarifItem);

        // Client Actuel
        QTableWidgetItem* clientItem = new QTableWidgetItem(d.client);
        clientItem->setFont(cellFont);
        dockTable->setItem(row, 6, clientItem);

        // Actions
        dockTable->setCellWidget(row, 7, createActionButtons(i));
    }
}

// ---------- STATUS BADGE ----------
QWidget* DocksWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* badge = new QLabel(status);
    QFont badgeFont("Segoe UI", 10, QFont::Medium);
    badge->setFont(badgeFont);
    badge->setFixedHeight(32);
    badge->setAlignment(Qt::AlignCenter);

    QString styleSheet;
    if (status == "Disponible") {
        styleSheet = R"(
            QLabel {
                background-color: #D1FAE5;
                color: #065F46;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else if (status == "Occupé") {
        styleSheet = R"(
            QLabel {
                background-color: #FEF3C7;
                color: #92400E;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else { // Maintenance
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

// ---------- ACTION BUTTONS ----------
QWidget* DocksWindow::createActionButtons(int row)
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
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditDock(row); });
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
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteDock(row); });
    layout->addWidget(deleteBtn);

    // Update button
    QPushButton* updateBtn = new QPushButton("🔄");
    updateBtn->setFixedSize(36, 36);
    updateBtn->setCursor(Qt::PointingHandCursor);
    updateBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #D1FAE5;
            color: #065F46;
            border: none;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #A7F3D0;
        }
    )");
    connect(updateBtn, &QPushButton::clicked, [this, row]() { onUpdateDock(row); });
    layout->addWidget(updateBtn);

    return widget;
}

// ---------- SLOTS ----------
void DocksWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void DocksWindow::onAddDock()
{
    AddDockDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Dock d = dialog.getData();
        d.id = QString("DK%1").arg(docks.size() + 1, 3, 10, QChar('0'));
        docks.append(d);
        populateTable(searchInput->text());
        qDebug() << "Dock ajouté:" << d.id;
    }
}

void DocksWindow::onEditDock(int row)
{
    if (row < 0 || row >= docks.size()) return;

    QMessageBox::information(this, "Modifier", "Modifier Dock: " + docks[row].id);
    qDebug() << "Modifier dock:" << docks[row].id;
}

void DocksWindow::onDeleteDock(int row)
{
    if (row < 0 || row >= docks.size()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir supprimer le dock '" + docks[row].id + "' ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Dock supprimé:" << docks[row].id;
        docks.removeAt(row);
        populateTable(searchInput->text());
    }
}

void DocksWindow::onUpdateDock(int row)
{
    if (row < 0 || row >= docks.size()) return;

    QMessageBox::information(this, "Mise à jour", "Mise à jour Dock: " + docks[row].id);
    qDebug() << "Mise à jour dock:" << docks[row].id;
}

void DocksWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}
void DocksWindow::onSort(int index)
{
    if (index == 1) { // Tarif ↑
        std::sort(docks.begin(), docks.end(), [](const Dock &a, const Dock &b) {
            return a.tarif < b.tarif;
        });
    } else if (index == 2) { // Tarif ↓
        std::sort(docks.begin(), docks.end(), [](const Dock &a, const Dock &b) {
            return a.tarif > b.tarif;
        });
    }

    populateTable(searchInput->text());
}
