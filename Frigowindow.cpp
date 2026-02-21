#include "frigowindow.h"
#include "addfrigodialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>

FrigoWindow::FrigoWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Données initiales de test
    Frigo f1{"FR001", "800", "65", "-4", "Disponible", "Sardine"};
    Frigo f2{"FR002", "1200", "72", "-6", "Occupé", "Thon"};
    Frigo f3{"FR003", "600", "60", "-3", "Disponible", "Crevette"};
    Frigo f4{"FR004", "1000", "68", "-5", "Maintenance", "Merlan"};

    frigos.append(f1);
    frigos.append(f2);
    frigos.append(f3);
    frigos.append(f4);

    populateTable();
}

FrigoWindow::~FrigoWindow()
{
}

void FrigoWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Frigos");
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

QFrame* FrigoWindow::createSidebar()
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
        logoLabel->setText("🧊");
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
    navLayout->addWidget(createNavButton("🧊", "Frigos", true));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();

    // Quit button
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);

    return sidebar;
}

QPushButton* FrigoWindow::createNavButton(const QString& icon, const QString& text, bool isActive, bool isLogout)
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
        connect(btn, &QPushButton::clicked, this, &FrigoWindow::onLogout);
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

QWidget* FrigoWindow::createContentArea()
{
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #F0F4F8;");

    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    // Header
    layout->addWidget(createHeader());

    // Toolbar
    layout->addWidget(createToolbar());

    // Table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* FrigoWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Frigos");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Surveillance des stocks froids et températures");
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
    QPushButton* addBtn   = makeBtn("➕  Nouveau Frigo", "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &FrigoWindow::onAddFrigo);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* FrigoWindow::createToolbar()
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
    searchInput->setPlaceholderText("Rechercher un frigo par statut, contenu...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setStyleSheet(R"(
        QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; }
        QLineEdit:focus{ border:2px solid #2563EB; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &FrigoWindow::onSearch);
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
    sortCombo->addItems({"Défaut", "Capacité ↑", "Capacité ↓", "Température ↑", "Température ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox{ background:transparent; border:none; padding:4px 12px; color:#1f2937; font-weight:600; }
        QComboBox:hover { color:#2563EB; }
        QComboBox::drop-down{ border:none; width:30px; }
        QComboBox QAbstractItemView{
            background:white; border:1px solid #e2e8f0; border-radius:12px;
            selection-background-color:#eff6ff; selection-color:#2563EB; outline:none; padding:8px;
        }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FrigoWindow::onSort);
    lay->addWidget(sortCombo, 2);

    return bar;
}

QFrame* FrigoWindow::createTableCard()
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

void FrigoWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({"ID", "Capacité (Kg)", "Humidité (%)", "Température (°C)", "Statut", "Poisson", "Actions"});

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
    table->setColumnWidth(1, 150);   // Capacité
    table->setColumnWidth(2, 140);   // Humidité
    table->setColumnWidth(3, 170);   // Température
    table->setColumnWidth(4, 120);   // Statut
    table->setColumnWidth(5, 130);   // Poisson
}

void FrigoWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);

    QFont cellFont("Segoe UI", 11);

    for (int i = 0; i < frigos.size(); ++i) {
        const Frigo& frigo = frigos[i];

        // Filter
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!frigo.id.toLower().contains(searchLower) &&
                !frigo.capacite.toLower().contains(searchLower) &&
                !frigo.poisson.toLower().contains(searchLower) &&
                !frigo.statut.toLower().contains(searchLower)) {
                continue;
            }
        }

        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 65);

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(frigo.id);
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        QFont idFont("Segoe UI", 11, QFont::Bold);
        idItem->setFont(idFont);
        idItem->setData(Qt::UserRole, i);
        table->setItem(row, 0, idItem);

        // Capacité
        QTableWidgetItem* capaciteItem = new QTableWidgetItem(frigo.capacite + " Kg");
        capaciteItem->setFont(cellFont);
        table->setItem(row, 1, capaciteItem);

        // Humidité
        QTableWidgetItem* humiditeItem = new QTableWidgetItem(frigo.humidite + " %");
        humiditeItem->setFont(cellFont);
        table->setItem(row, 2, humiditeItem);

        // Température
        QTableWidgetItem* tempItem = new QTableWidgetItem(frigo.temperature + " °C");
        tempItem->setFont(cellFont);
        table->setItem(row, 3, tempItem);

        // Statut
        table->setCellWidget(row, 4, createStatusBadge(frigo.statut));

        // Poisson
        QTableWidgetItem* poissonItem = new QTableWidgetItem(frigo.poisson);
        poissonItem->setFont(cellFont);
        table->setItem(row, 5, poissonItem);

        // Actions (Modifier et Supprimer)
        table->setCellWidget(row, 6, createActionButtons(i));
    }
}

QWidget* FrigoWindow::createStatusBadge(const QString& status)
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

QWidget* FrigoWindow::createActionButtons(int row)
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
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditFrigo(row); });
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
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteFrigo(row); });
    layout->addWidget(deleteBtn);

    return widget;
}

QString FrigoWindow::generateFrigoId()
{
    int maxId = 0;
    for (const Frigo& f : frigos) {
        QString numStr = f.id.mid(2); // "FR001" -> "001"
        int num = numStr.toInt();
        if (num > maxId) {
            maxId = num;
        }
    }
    return QString("FR%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void FrigoWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void FrigoWindow::onSort(int index)
{
    Q_UNUSED(index);
    // TODO: Implement sorting logic for Frigo
}

void FrigoWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}

void FrigoWindow::onAddFrigo()
{
    AddFrigoDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Frigo newFrigo = dialog.getData();
        newFrigo.id = generateFrigoId();
        frigos.append(newFrigo);
        populateTable(searchInput->text());
        qDebug() << "Frigo ajouté:" << newFrigo.id;
    }
}

void FrigoWindow::onEditFrigo(int row)
{
    if (row < 0 || row >= frigos.size()) return;

    AddFrigoDialog dialog(this, &frigos[row]);
    if (dialog.exec() == QDialog::Accepted) {
        Frigo updatedFrigo = dialog.getData();
        updatedFrigo.id = frigos[row].id;
        frigos[row] = updatedFrigo;
        populateTable(searchInput->text());
        qDebug() << "Frigo modifié:" << frigos[row].id;
    }
}

void FrigoWindow::onDeleteFrigo(int row)
{
    if (row < 0 || row >= frigos.size()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir supprimer le frigo '" + frigos[row].id + "' ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Frigo supprimé:" << frigos[row].id;
        frigos.removeAt(row);
        populateTable(searchInput->text());
    }
}
