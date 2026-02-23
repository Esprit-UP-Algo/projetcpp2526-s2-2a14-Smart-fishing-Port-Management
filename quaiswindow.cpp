#include "quaiswindow.h"
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
#include "addquaidialog.h"

QuaisWindow::QuaisWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(1400, 800);
    setWindowTitle("PortFlow - Gestion des Quais");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    // Pre-fill quais list
    for (int i = 0; i < 12; ++i) {
        Quai q;
        q.id = QString("QK%1").arg(i + 1, 3, 10, QChar('0'));
        q.nom = "Quai " + QString::number(i + 1);
        q.capacite = "2-3 bateaux";
        q.tailleMax = "15m";
        q.statut = (i % 3 == 0) ? "Disponible" : ((i % 3 == 1) ? "Occupé" : "Maintenance");
        q.tarif = "50 DT/jour";
        q.client = (i % 3 == 1) ? "Sea Harvest Ltd" : "-";
        quais.append(q);
    }

    setupUI();
    setupQuaiTable();
    populateTable();
}

// ---------- SETUP UI ----------
void QuaisWindow::setupUI()
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
QFrame* QuaisWindow::createSidebar()
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

    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(180, 100);
    logoContainer->setStyleSheet(R"(
        QFrame {
            background-color: transparent;
            border-radius: 0px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(logoContainer);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    containerLayout->setAlignment(Qt::AlignCenter);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix(":/images/images/logo.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
    } else {
        logoLabel->setText("⚓");
        logoLabel->setStyleSheet(R"(
            QLabel { font-size: 50px; color: white; }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
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
    navLayout->addWidget(createNavButton("⚓", "Quais", true));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);
    return sidebar;
}

QPushButton* QuaisWindow::createNavButton(const QString& icon, const QString& text,
                                          bool isActive, bool isLogout)
{
    QPushButton* btn = new QPushButton(icon + "  " + text);
    btn->setFont(QFont("Segoe UI", 12, QFont::Medium));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(55);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255, 255, 255, 0.1); color: white; border: none; border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(239, 68, 68, 0.8); }
        )");
        connect(btn, &QPushButton::clicked, this, &QuaisWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255, 255, 255, 0.25); color: white; border: none; border-radius: 12px; text-align: left; padding-left: 20px; font-weight: bold; }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton { background-color: transparent; color: rgba(255, 255, 255, 0.9); border: none; border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(255, 255, 255, 0.15); }
        )");
    }
    return btn;
}

QWidget* QuaisWindow::createContentArea()
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

QFrame* QuaisWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");
    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Quais");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Administration des emplacements et disponibilités");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color:#6b7280;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(45);
        btn->setMinimumWidth(160);
        btn->setStyleSheet(QString("QPushButton{ background:%1; color:white; border:none; border-radius:12px; padding:0 20px; } QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* pdfBtn   = makeBtn("📄  Exporter PDF",  "#059669", "#047857");
    QPushButton* addBtn   = makeBtn("➕  Nouveau Quai",   "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &QuaisWindow::onAddQuai);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* QuaisWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet("QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }");
    bar->setFixedHeight(62);
    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher un quai par numéro, type...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setStyleSheet("QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; } QLineEdit:focus{ border:2px solid #2563EB; background:white; }");
    connect(searchInput, &QLineEdit::textChanged, this, &QuaisWindow::onSearch);
    lay->addWidget(searchInput, 3);

    QFrame* div = new QFrame(); div->setFrameShape(QFrame::VLine);
    div->setStyleSheet("color:#e2e8f0;"); div->setFixedWidth(1);
    lay->addWidget(div);

    QLabel* sortLabel = new QLabel("Trier par :");
    sortLabel->setFont(QFont("Segoe UI", 10, QFont::Medium));
    sortLabel->setStyleSheet("color:#64748b; margin-left:10px;");
    lay->addWidget(sortLabel);

    sortCombo = new QComboBox();
    sortCombo->setFont(QFont("Segoe UI", 10));
    sortCombo->setFixedHeight(45);
    sortCombo->setMinimumWidth(200);
    sortCombo->addItems({"Défaut", "Numéro ↑", "Numéro ↓", "Tarif ↑", "Tarif ↓"});
    sortCombo->setStyleSheet("QComboBox{ background:transparent; border:none; padding:4px 12px; color:#1f2937; font-weight:600; } QComboBox:hover { color:#2563EB; } QComboBox::drop-down{ border:none; width:30px; } QComboBox QAbstractItemView{ background:white; border:1px solid #e2e8f0; border-radius:12px; selection-background-color:#eff6ff; selection-color:#2563EB; outline:none; padding:8px; }");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QuaisWindow::onSort);
    lay->addWidget(sortCombo, 2);

    return bar;
}

QFrame* QuaisWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet("QFrame { background-color: #5D9CEC; border-radius: 20px; padding: 3px; }");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(3, 3, 3, 3);

    QFrame* whiteContainer = new QFrame();
    whiteContainer->setStyleSheet("QFrame { background-color: white; border-radius: 17px; }");
    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    quaiTable = new QTableWidget();
    containerLayout->addWidget(quaiTable);
    layout->addWidget(whiteContainer);

    return card;
}

void QuaisWindow::setupQuaiTable()
{
    quaiTable->setColumnCount(8);
    quaiTable->setHorizontalHeaderLabels({"ID", "Nom du Quai", "Capacité", "Taille Max", "Statut", "Tarif", "Client Actuel", "Actions"});
    quaiTable->horizontalHeader()->setStretchLastSection(true);
    quaiTable->verticalHeader()->setVisible(false);
    quaiTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    quaiTable->setSelectionMode(QAbstractItemView::SingleSelection);
    quaiTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    quaiTable->setShowGrid(true);
    quaiTable->horizontalHeader()->setFont(QFont("Segoe UI", 11, QFont::Bold));
    quaiTable->horizontalHeader()->setFixedHeight(50);
    quaiTable->setStyleSheet("QTableWidget { background-color: white; border: 2px solid #d1d5db; border-radius: 16px; gridline-color: #d1d5db; } QTableWidget::item { padding: 12px; border-right: 1px solid #d1d5db; border-bottom: 1px solid #d1d5db; color: #1f2937; background-color: white; font-family: 'Segoe UI'; font-size: 11pt; } QTableWidget::item:selected { background-color: #EBF5FF; color: #2563EB; } QHeaderView::section { background-color: #d1d5db; color: #1f2937; padding: 12px; border: none; font-weight: 600; font-family: 'Segoe UI'; font-size: 11pt; }");

    quaiTable->setColumnWidth(0, 80);    // ID
    quaiTable->setColumnWidth(1, 150);   // Nom
    quaiTable->setColumnWidth(2, 130);   // Capacité
    quaiTable->setColumnWidth(3, 120);   // Taille Max
    quaiTable->setColumnWidth(4, 130);   // Statut
    quaiTable->setColumnWidth(5, 110);   // Tarif
    quaiTable->setColumnWidth(6, 180);   // Client
}

void QuaisWindow::populateTable(const QString& filterText)
{
    quaiTable->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!q.nom.toLower().contains(searchLower) && !q.statut.toLower().contains(searchLower) && !q.client.toLower().contains(searchLower) && !q.id.toLower().contains(searchLower)) continue;
        }

        int row = quaiTable->rowCount();
        quaiTable->insertRow(row);
        quaiTable->setRowHeight(row, 65);

        QTableWidgetItem* idItem = new QTableWidgetItem(q.id);
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        idItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
        quaiTable->setItem(row, 0, idItem);
        quaiTable->setItem(row, 1, new QTableWidgetItem(q.nom));
        quaiTable->setItem(row, 2, new QTableWidgetItem(q.capacite));
        quaiTable->setItem(row, 3, new QTableWidgetItem(q.tailleMax));
        quaiTable->setCellWidget(row, 4, createStatusBadge(q.statut));
        quaiTable->setItem(row, 5, new QTableWidgetItem(q.tarif));
        quaiTable->setItem(row, 6, new QTableWidgetItem(q.client));
        quaiTable->setCellWidget(row, 7, createActionButtons(i));
    }
}

QWidget* QuaisWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    QLabel* badge = new QLabel(status);
    badge->setFont(QFont("Segoe UI", 10, QFont::Medium));
    badge->setFixedHeight(32);
    badge->setAlignment(Qt::AlignCenter);

    if (status == "Disponible") badge->setStyleSheet("QLabel { background-color: #D1FAE5; color: #065F46; border-radius: 8px; padding: 6px 16px; }");
    else if (status == "Occupé") badge->setStyleSheet("QLabel { background-color: #FEF3C7; color: #92400E; border-radius: 8px; padding: 6px 16px; }");
    else badge->setStyleSheet("QLabel { background-color: #FEE2E2; color: #991B1B; border-radius: 8px; padding: 6px 16px; }");

    layout->addWidget(badge);
    return widget;
}

QWidget* QuaisWindow::createActionButtons(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);

    auto makeBtn = [&](const QString& icon, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(icon);
        btn->setFixedSize(36, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString("QPushButton { background-color: %1; border: none; border-radius: 8px; font-size: 16px; } QPushButton:hover { background-color: %2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* editBtn = makeBtn("✏️", "#FEF3C7", "#FDE68A");
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditQuai(row); });
    layout->addWidget(editBtn);

    QPushButton* deleteBtn = makeBtn("🗑️", "#FEE2E2", "#FECACA");
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteQuai(row); });
    layout->addWidget(deleteBtn);

    QPushButton* updateBtn = makeBtn("🔄", "#D1FAE5", "#A7F3D0");
    connect(updateBtn, &QPushButton::clicked, [this, row]() { onUpdateQuai(row); });
    layout->addWidget(updateBtn);

    return widget;
}

void QuaisWindow::onSearch(const QString& text) { populateTable(text); }

void QuaisWindow::onAddQuai()
{
    AddQuaiDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Quai q = dialog.getData();
        q.id = QString("QK%1").arg(quais.size() + 1, 3, 10, QChar('0'));
        quais.append(q);
        populateTable(searchInput->text());
    }
}

void QuaisWindow::onEditQuai(int row) { if (row >= 0 && row < quais.size()) QMessageBox::information(this, "Modifier", "Modifier Quai: " + quais[row].id); }

void QuaisWindow::onDeleteQuai(int row)
{
    if (row >= 0 && row < quais.size() && QMessageBox::question(this, "Confirmation", "Supprimer le quai '" + quais[row].id + "' ?") == QMessageBox::Yes) {
        quais.removeAt(row);
        populateTable(searchInput->text());
    }
}

void QuaisWindow::onUpdateQuai(int row) { if (row >= 0 && row < quais.size()) QMessageBox::information(this, "Mise à jour", "Mise à jour Quai: " + quais[row].id); }

void QuaisWindow::onLogout() { if (QMessageBox::question(this, "Quitter", "Quitter l'application ?") == QMessageBox::Yes) this->close(); }

void QuaisWindow::onSort(int index)
{
    if (index == 3) std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b) { return a.tarif < b.tarif; });
    else if (index == 4) std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b) { return a.tarif > b.tarif; });
    populateTable(searchInput->text());
}
