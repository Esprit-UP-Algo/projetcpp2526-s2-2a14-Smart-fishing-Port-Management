#include "quaiswindow.h"
#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPushButton>
#include <cmath>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QFileDialog>
#include <QDate>
#include <QPrintDialog>
#include <QTextTable>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QTextLength>
#include <QDateTime>
#include <QStandardPaths>
#include <QDateEdit>
#include <QPageSize>

#include "addquaidialog.h"

#include "circularprogress.h"
#include "contractgenerator.h"
#include "contractdialog.h"


QuaisWindow::QuaisWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(1400, 800);
    setWindowTitle("PortFlow - Gestion des Quais");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    for (int i = 0; i < 12; ++i) {
        Quai q;
        q.setId(QString("QK%1").arg(i + 1, 3, 10, QChar('0')));
        q.setNom("Quai " + QString::number(i + 1));
        q.setCapacite(3);
        q.setTailleMax(15.0);
        q.setStatut((i % 3 == 0) ? "Disponible" :
                        (i % 3 == 1) ? "Occupé" : "Maintenance");
        q.setTarif(50.0);
        q.setClient((i % 3 == 1) ? "Sea Harvest Ltd" : "-");

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

    QFrame* logoFrame = new QFrame();
    logoFrame->setFixedHeight(160);
    logoFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setContentsMargins(20, 15, 20, 15);

    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(180, 100);
    logoContainer->setStyleSheet("QFrame { background-color: transparent; border-radius: 0px; }");

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
        logoLabel->setStyleSheet("QLabel { font-size: 50px; color: white; }");
        logoLabel->setAlignment(Qt::AlignCenter);
    }

    logoLabel->setStyleSheet("background: transparent;");
    containerLayout->addWidget(logoLabel);
    logoLayout->addWidget(logoContainer);
    layout->addWidget(logoFrame);

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
    connect(statsBtn, &QPushButton::clicked, this, &QuaisWindow::afficherStatistiques);

    QPushButton* pdfBtn = makeBtn("📄  Exporter PDF", "#059669", "#047857");
    // Connect the PDF button to show a dialog to select a quai and generate contract
    connect(pdfBtn, &QPushButton::clicked, this, [this]() {
        if (quais.isEmpty()) {
            QMessageBox::warning(this, "Aucun quai", "Aucun quai disponible.");
            return;
        }

        QDialog* dlg = new QDialog(this);
        dlg->setFixedSize(560, 460);
        dlg->setModal(true);
        dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        dlg->setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 80));

        QWidget* container = new QWidget(dlg);
        container->setGeometry(10, 10, 540, 440);  // inset for shadow room
        container->setGraphicsEffect(shadow);
        container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

        QVBoxLayout* mainLay = new QVBoxLayout(container);
        mainLay->setContentsMargins(0, 0, 0, 0);
        mainLay->setSpacing(0);

        // --- Header ---
        QFrame* header = new QFrame();
        header->setFixedHeight(75);
        header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #059669, stop:1 #34D399);
            border-radius: 20px 20px 0 0;
        }
    )");
        QHBoxLayout* headerLay = new QHBoxLayout(header);
        headerLay->setContentsMargins(25, 0, 18, 0);

        QLabel* titleLbl = new QLabel("📄  Exporter un contrat PDF");
        titleLbl->setFont(QFont("Segoe UI", 15, QFont::Bold));
        titleLbl->setStyleSheet("color: white; background: transparent;");
        headerLay->addWidget(titleLbl, 1);

        QPushButton* closeBtn = new QPushButton("✕");
        closeBtn->setFixedSize(32, 32);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 16px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
        headerLay->addWidget(closeBtn);
        mainLay->addWidget(header);

        // --- Body ---
        QWidget* body = new QWidget();
        body->setStyleSheet("background: transparent;");
        QVBoxLayout* bodyLay = new QVBoxLayout(body);
        bodyLay->setContentsMargins(24, 18, 24, 0);
        bodyLay->setSpacing(10);

        QLabel* instrLbl = new QLabel("Sélectionnez un quai pour générer son contrat de location :");
        instrLbl->setFont(QFont("Segoe UI", 10));
        instrLbl->setStyleSheet("color: #6b7280; background: transparent;");
        instrLbl->setWordWrap(true);
        bodyLay->addWidget(instrLbl);

        // List widget
        QListWidget* quaiList = new QListWidget();
        quaiList->setFont(QFont("Segoe UI", 10));
        quaiList->setStyleSheet(R"(
        QListWidget {
            background: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 6px;
            outline: none;
        }
        QListWidget::item {
            padding: 10px 14px;
            border-radius: 8px;
            color: #1f2937;
        }
        QListWidget::item:selected {
            background: #D1FAE5;
            color: #065F46;
            font-weight: bold;
        }
        QListWidget::item:hover:!selected {
            background: #F3F4F6;
        }
    )");

        for (int i = 0; i < quais.size(); ++i) {
            const Quai& q = quais[i];
            QString emoji = (q.getStatut() == "Disponible") ? "🟢"
                            : (q.getStatut() == "Occupé")     ? "🟡" : "🔴";
            QString text = QString("%1  %2  —  %3     %4 m  |  %5 DT/j")
                               .arg(emoji)
                               .arg(q.getId())
                               .arg(q.getNom())
                               .arg(q.getTailleMax())
                               .arg(q.getTarif());
            QListWidgetItem* item = new QListWidgetItem(text);
            item->setData(Qt::UserRole, i);
            if (q.getStatut() == "Maintenance") {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                item->setForeground(QColor("#9ca3af"));
            }
            quaiList->addItem(item);
        }

        bodyLay->addWidget(quaiList, 1);
        mainLay->addWidget(body, 1);

        // --- Buttons ---
        QHBoxLayout* btnLay = new QHBoxLayout();
        btnLay->setContentsMargins(24, 12, 24, 20);
        btnLay->setSpacing(12);

        QPushButton* cancelBtn = new QPushButton("Annuler");
        cancelBtn->setFixedHeight(42);
        cancelBtn->setMinimumWidth(110);
        cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
        connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

        QPushButton* generateBtn = new QPushButton("📄  Générer le contrat");
        generateBtn->setFixedHeight(42);
        generateBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        generateBtn->setCursor(Qt::PointingHandCursor);
        generateBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #059669, stop:1 #34D399);
                      color: white; border: none; border-radius: 10px; padding: 0 20px; }
        QPushButton:hover { background: #047857; }
    )");

        connect(generateBtn, &QPushButton::clicked, [this, dlg, quaiList]() {
            QListWidgetItem* current = quaiList->currentItem();
            if (!current || !(current->flags() & Qt::ItemIsEnabled)) {
                QMessageBox::warning(dlg, "Sélection requise",
                                     "Veuillez sélectionner un quai disponible ou occupé.");
                return;
            }
            int row = current->data(Qt::UserRole).toInt();
            dlg->accept();
            onGenerateContract(row);
        });

        btnLay->addStretch();
        btnLay->addWidget(cancelBtn);
        btnLay->addWidget(generateBtn);
        mainLay->addLayout(btnLay);

        dlg->exec();
    });

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
    quaiTable->setColumnCount(7);
    quaiTable->setHorizontalHeaderLabels({"référence", "Nom du Quai", "Capacité", "Taille Max", "Statut", "Tarif", "Actions"});
    quaiTable->horizontalHeader()->setStretchLastSection(true);
    quaiTable->verticalHeader()->setVisible(false);
    quaiTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    quaiTable->setSelectionMode(QAbstractItemView::SingleSelection);
    quaiTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    quaiTable->setShowGrid(true);
    quaiTable->horizontalHeader()->setFont(QFont("Segoe UI", 11, QFont::Bold));
    quaiTable->horizontalHeader()->setFixedHeight(50);
    quaiTable->setStyleSheet("QTableWidget { background-color: white; border: 2px solid #d1d5db; border-radius: 16px; gridline-color: #d1d5db; } QTableWidget::item { padding: 12px; border-right: 1px solid #d1d5db; border-bottom: 1px solid #d1d5db; color: #1f2937; background-color: white; font-family: 'Segoe UI'; font-size: 11pt; } QTableWidget::item:selected { background-color: #EBF5FF; color: #2563EB; } QHeaderView::section { background-color: #d1d5db; color: #1f2937; padding: 12px; border: none; font-weight: 600; font-family: 'Segoe UI'; font-size: 11pt; }");

    quaiTable->setColumnWidth(0, 130);
    quaiTable->setColumnWidth(1, 150);
    quaiTable->setColumnWidth(2, 130);
    quaiTable->setColumnWidth(3, 120);
    quaiTable->setColumnWidth(4, 130);
    quaiTable->setColumnWidth(5, 110);
}

void QuaisWindow::populateTable(const QString& filterText)
{
    quaiTable->setRowCount(0);

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];

        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!q.getNom().toLower().contains(searchLower) &&
                !q.getStatut().toLower().contains(searchLower) &&
                !q.getClient().toLower().contains(searchLower) &&
                !q.getId().toLower().contains(searchLower))
                continue;
        }

        int row = quaiTable->rowCount();
        quaiTable->insertRow(row);
        quaiTable->setRowHeight(row, 65);

        QTableWidgetItem* idItem = new QTableWidgetItem(q.getId());
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        idItem->setFont(QFont("Segoe UI", 11, QFont::Bold));

        quaiTable->setItem(row, 0, idItem);
        quaiTable->setItem(row, 1, new QTableWidgetItem(q.getNom()));
        quaiTable->setItem(row, 2, new QTableWidgetItem(QString::number(q.getCapacite())));
        quaiTable->setItem(row, 3, new QTableWidgetItem(QString::number(q.getTailleMax()) + " m"));
        quaiTable->setCellWidget(row, 4, createStatusBadge(q.getStatut()));
        quaiTable->setItem(row, 5, new QTableWidgetItem(QString::number(q.getTarif()) + " DT"));
        quaiTable->setCellWidget(row, 6, createActionButtons(i));
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

    if (status == "Disponible")
        badge->setStyleSheet("QLabel { background-color: #D1FAE5; color: #065F46; border-radius: 8px; padding: 6px 16px; }");
    else if (status == "Occupé")
        badge->setStyleSheet("QLabel { background-color: #FEF3C7; color: #92400E; border-radius: 8px; padding: 6px 16px; }");
    else
        badge->setStyleSheet("QLabel { background-color: #FEE2E2; color: #991B1B; border-radius: 8px; padding: 6px 16px; }");

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

    auto makeBtn = [&](const QString& icon, const QString& bg, const QString& hover, const QString& tooltip = "") {
        QPushButton* btn = new QPushButton(icon);
        btn->setFixedSize(36, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setStyleSheet(QString("QPushButton { background-color: %1; border: none; border-radius: 8px; font-size: 16px; } QPushButton:hover { background-color: %2; }").arg(bg, hover));
        return btn;
    };

    // Edit button
    QPushButton* editBtn = makeBtn("✏️", "#FEF3C7", "#FDE68A", "Modifier le quai");
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditQuai(row); });
    layout->addWidget(editBtn);

    // Delete button
    QPushButton* deleteBtn = makeBtn("🗑️", "#FEE2E2", "#FECACA", "Supprimer le quai");
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteQuai(row); });
    layout->addWidget(deleteBtn);

    // Contract button (NEW)
    QPushButton* contractBtn = makeBtn("📄", "#E0F2FE", "#BAE6FD", "Générer un contrat");
    connect(contractBtn, &QPushButton::clicked, [this, row]() {
        if (row >= 0 && row < quais.size()) {
            onGenerateContract(row);
        }
    });
    layout->addWidget(contractBtn);

    return widget;
}

void QuaisWindow::onSearch(const QString& text) { populateTable(text); }

void QuaisWindow::onAddQuai()
{
    AddQuaiDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Quai q = dialog.getData();
        q.setId(QString("QK%1").arg(quais.size() + 1, 3, 10, QChar('0')));
        quais.append(q);
        populateTable(searchInput->text());
    }
}

void QuaisWindow::onEditQuai(int row) {
    if (row < 0 || row >= quais.size()) return;

    Quai& q = quais[row];

    // --- Create frameless dialog ---
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Modifier Quai");
    dlg->setFixedSize(500, 480);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 500, 480);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Header band
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(80);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2B5EA6, stop:1 #5D9CEC);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("✏️  Modifier le Quai");
    titleLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 18px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(headerBand);

    // Form area
    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(30, 20, 30, 10);
    formLay->setSpacing(14);

    auto makeField = [&](const QString& label, const QString& value) -> QLineEdit* {
        QLabel* lbl = new QLabel(label);
        lbl->setFont(QFont("Segoe UI", 10, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");

        QLineEdit* field = new QLineEdit(value);
        field->setFont(QFont("Segoe UI", 11));
        field->setFixedHeight(44);
        field->setStyleSheet(R"(
            QLineEdit { background: #F9FAFB; border: 2px solid #E5E7EB; border-radius: 10px;
                        padding: 4px 14px; color: #1f2937; }
            QLineEdit:focus { border: 2px solid #2563EB; background: white; }
        )");
        formLay->addWidget(lbl);
        formLay->addWidget(field);
        return field;
    };

    QLineEdit* nomField      = makeField("Nom du Quai", q.getNom());
    QLineEdit* capaciteField = makeField("Capacité", QString::number(q.getCapacite()));
    QLineEdit* tailleField   = makeField("Taille Max (m)", QString::number(q.getTailleMax()));
    QLineEdit* tarifField    = makeField("Tarif (DT)", QString::number(q.getTarif()));

    // Statut combo
    QLabel* statutLbl = new QLabel("Statut");
    statutLbl->setFont(QFont("Segoe UI", 10, QFont::Medium));
    statutLbl->setStyleSheet("color: #374151; background: transparent;");
    formLay->addWidget(statutLbl);

    QComboBox* statutCombo = new QComboBox();
    statutCombo->addItems({"Disponible", "Occupé", "Maintenance"});
    statutCombo->setCurrentText(q.getStatut());
    statutCombo->setFont(QFont("Segoe UI", 11));
    statutCombo->setFixedHeight(44);
    statutCombo->setStyleSheet(R"(
        QComboBox { background: #F9FAFB; border: 2px solid #E5E7EB; border-radius: 10px;
                    padding: 4px 14px; color: #1f2937; }
        QComboBox:focus { border: 2px solid #2563EB; background: white; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox QAbstractItemView { background: white; border: 1px solid #e2e8f0;
            border-radius: 10px; selection-background-color: #eff6ff;
            selection-color: #2563EB; padding: 6px; }
    )");
    formLay->addWidget(statutCombo);

    mainLay->addWidget(formArea, 1);

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(30, 0, 30, 24);
    btnLay->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setFont(QFont("Segoe UI", 11, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton("💾  Enregistrer");
    saveBtn->setFixedHeight(44);
    saveBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #2B5EA6, stop:1 #5D9CEC);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(saveBtn, &QPushButton::clicked, [&]() {
        q.setNom(nomField->text());
        q.setCapacite(capaciteField->text().toInt());
        q.setTailleMax(tailleField->text().toDouble());
        q.setTarif(tarifField->text().toDouble());
        q.setStatut(statutCombo->currentText());
        dlg->accept();
    });

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(saveBtn);
    mainLay->addLayout(btnLay);

    if (dlg->exec() == QDialog::Accepted) {
        populateTable(searchInput->text());
    }
}
void QuaisWindow::onDeleteQuai(int row) {
    if (row < 0 || row >= quais.size()) return;

    const Quai& q = quais[row];

    QDialog* dlg = new QDialog(this);
    dlg->setFixedSize(420, 260);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 420, 260);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Red header band
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(75);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #991B1B, stop:1 #EF4444);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("🗑️  Supprimer le Quai");
    titleLbl->setFont(QFont("Segoe UI", 15, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(34, 34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 17px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(headerBand);

    // Message
    QVBoxLayout* bodyLay = new QVBoxLayout();
    bodyLay->setContentsMargins(30, 24, 30, 10);

    QLabel* msgLbl = new QLabel(
        QString("Êtes-vous sûr de vouloir supprimer le quai\n<b>%1 — %2</b> ?")
            .arg(q.getId(), q.getNom()));
    msgLbl->setFont(QFont("Segoe UI", 11));
    msgLbl->setStyleSheet("color: #374151; background: transparent;");
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setTextFormat(Qt::RichText);
    bodyLay->addWidget(msgLbl);

    QLabel* warnLbl = new QLabel("⚠️  Cette action est irréversible.");
    warnLbl->setFont(QFont("Segoe UI", 9));
    warnLbl->setStyleSheet("color: #B91C1C; background: transparent;");
    warnLbl->setAlignment(Qt::AlignCenter);
    bodyLay->addWidget(warnLbl);

    mainLay->addLayout(bodyLay);
    mainLay->addStretch();

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(30, 0, 30, 24);
    btnLay->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setFont(QFont("Segoe UI", 11, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* deleteBtn = new QPushButton("🗑️  Supprimer");
    deleteBtn->setFixedHeight(44);
    deleteBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #991B1B, stop:1 #EF4444);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #B91C1C; }
    )");
    connect(deleteBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(deleteBtn);
    mainLay->addLayout(btnLay);

    if (dlg->exec() == QDialog::Accepted) {
        quais.removeAt(row);
        populateTable(searchInput->text());
    }
}
void QuaisWindow::onUpdateQuai(int id) {
    // Your logic here. Even an empty body will fix the linker:
    Q_UNUSED(id); // Avoid unused parameter warning if nothing is done yet
}
void QuaisWindow::onLogout() {
    if (QMessageBox::question(this, "Quitter", "Quitter l'application ?") == QMessageBox::Yes)
        this->close();
}

void QuaisWindow::onSort(int index)
{
    static QVector<Quai> originalOrder = quais;

    switch (index) {
    case 0: quais = originalOrder; break;
    case 1: std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b){ return a.getId() < b.getId(); }); break;
    case 2: std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b){ return a.getId() > b.getId(); }); break;
    case 3: std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b){ return a.getTarif() < b.getTarif(); }); break;
    case 4: std::sort(quais.begin(), quais.end(), [](const Quai &a, const Quai &b){ return a.getTarif() > b.getTarif(); }); break;
    default: break;
    }

    populateTable(searchInput->text());
}

// ==================== BEAUTIFUL STATISTICS DIALOG ====================
void QuaisWindow::afficherStatistiques()
{
    // Compute stats
    int totalQuais = quais.size();
    int totalBerths = 0, occupiedBerths = 0, availableBerths = 0, maintenanceBerths = 0;
    double totalRevenue = 0.0;

    for (const Quai& q : quais) {
        int cap = q.getCapacite();
        totalBerths += cap;
        if (q.getStatut() == "Occupé") {
            occupiedBerths += cap;
            totalRevenue += q.getTarif() * cap;
        } else if (q.getStatut() == "Disponible") {
            availableBerths += cap;
        } else {
            maintenanceBerths += cap;
        }
    }

    double occupancyRate = (totalBerths > 0) ? (double)occupiedBerths / totalBerths * 100 : 0;
    double averageRevenue = (totalQuais > 0) ? totalRevenue / totalQuais : 0;

    // Create dialog
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Statistiques des Quais");
    dlg->setFixedSize(720, 520);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    // Drop shadow
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    // Main container
    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 720, 520);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet(R"(
        QWidget {
            background: white;
            border-radius: 24px;
        }
    )");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // ---- Header band ----
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(90);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2B5EA6, stop:1 #5D9CEC);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("📊  Statistiques des Quais");
    titleLbl->setFont(QFont("Segoe UI", 18, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 18px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    headerLay->addWidget(closeBtn);

    mainLay->addWidget(headerBand);

    // ---- Subtitle ----
    QLabel* subLbl = new QLabel("Vue d'ensemble de l'occupation et des revenus du port");
    subLbl->setFont(QFont("Segoe UI", 10));
    subLbl->setStyleSheet("color: #6b7280; padding: 16px 30px 0px 30px; background: transparent;");
    mainLay->addWidget(subLbl);

    // ---- Circular charts row ----
    QFrame* chartsFrame = new QFrame();
    chartsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* chartsLay = new QHBoxLayout(chartsFrame);
    chartsLay->setContentsMargins(20, 20, 20, 10);
    chartsLay->setSpacing(10);

    // Chart data: value, max, label, unit, color
    struct ChartData { double val; double max; QString label; QString unit; QColor color; };
    QList<ChartData> charts = {
                               { (double)totalQuais,       20.0,             "Total\nQuais",           "",   QColor("#2B5EA6") },
                               { occupancyRate,            100.0,             "Taux\nd'Occupation",     "%",  QColor("#7C3AED") },
                               { (double)occupiedBerths,   (double)totalBerths, "Emplacements\nOccupés", "",   QColor("#EF8C2A") },
                               { (double)availableBerths,  (double)totalBerths, "Emplacements\nLibres",  "",   QColor("#059669") },
                               };

    QList<CircularProgress*> progressWidgets;
    for (const ChartData& cd : charts) {
        CircularProgress* cp = new CircularProgress(cd.val, cd.max, cd.label, cd.unit, cd.color);
        chartsLay->addWidget(cp, 0, Qt::AlignCenter);
        progressWidgets.append(cp);
    }
    mainLay->addWidget(chartsFrame);

    // ---- Stats cards row ----
    QFrame* cardsFrame = new QFrame();
    cardsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* cardsLay = new QHBoxLayout(cardsFrame);
    cardsLay->setContentsMargins(24, 4, 24, 4);
    cardsLay->setSpacing(14);

    auto makeCard = [&](const QString& icon, const QString& val, const QString& label,
                        const QString& bg, const QString& textColor) {
        QFrame* card = new QFrame();
        card->setFixedHeight(90);
        card->setStyleSheet(QString("QFrame { background: %1; border-radius: 16px; }").arg(bg));
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(16, 8, 16, 8);

        QLabel* iconLbl = new QLabel(icon);
        iconLbl->setFont(QFont("Segoe UI", 22));
        iconLbl->setStyleSheet("background: transparent;");
        cl->addWidget(iconLbl);

        QVBoxLayout* vl = new QVBoxLayout();
        QLabel* valLbl = new QLabel(val);
        valLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
        valLbl->setStyleSheet(QString("color: %1; background: transparent;").arg(textColor));
        QLabel* labLbl = new QLabel(label);
        labLbl->setFont(QFont("Segoe UI", 9));
        labLbl->setStyleSheet("color: #6b7280; background: transparent;");
        vl->addWidget(valLbl);
        vl->addWidget(labLbl);
        cl->addLayout(vl, 1);
        return card;
    };

    cardsLay->addWidget(makeCard("💰",
                                 QString("%1 DT").arg(totalRevenue, 0, 'f', 0),
                                 "Chiffre d'Affaires", "#EFF6FF", "#1D4ED8"));

    cardsLay->addWidget(makeCard("📈",
                                 QString("%1 DT").arg(averageRevenue, 0, 'f', 0),
                                 "Revenu Moyen / Quai", "#F5F3FF", "#6D28D9"));

    cardsLay->addWidget(makeCard("🔧",
                                 QString::number(maintenanceBerths),
                                 "En Maintenance", "#FEF2F2", "#991B1B"));

    mainLay->addWidget(cardsFrame);
    mainLay->addStretch();

    // ---- Close button at bottom ----
    QHBoxLayout* bottomLay = new QHBoxLayout();
    bottomLay->setContentsMargins(24, 0, 24, 20);
    QPushButton* okBtn = new QPushButton("  Fermer");
    okBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    okBtn->setFixedHeight(44);
    okBtn->setFixedWidth(160);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #2B5EA6, stop:1 #5D9CEC);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    bottomLay->addStretch();
    bottomLay->addWidget(okBtn);
    mainLay->addLayout(bottomLay);


    // Animate all wheels with a staggered delay
    for (int i = 0; i < progressWidgets.size(); ++i) {
        CircularProgress* cp = progressWidgets[i];
        QTimer::singleShot(i * 150, cp, [cp]() { cp->animateTo(); });
    }
    dlg->exec();

}
void QuaisWindow::onGenerateContract(int row)
{
    if (row < 0 || row >= quais.size()) return;

    const Quai& quai = quais[row];

    // Check if quai is available or occupied
    if (quai.getStatut() == "Maintenance") {
        QMessageBox::warning(this, "Quai en maintenance",
                             "Ce quai est en maintenance et ne peut pas faire l'objet d'un contrat.");
        return;
    }

    // Create and show contract dialog
    ContractDialog dialog(quai, this);

    if (dialog.exec() == QDialog::Accepted) {
        QString clientName = dialog.getClientName();
        QString company = dialog.getCompany();
        QString duration = dialog.getDuration();
        QDate startDate = dialog.getStartDate();

        // Validate required fields
        if (clientName.isEmpty() || company.isEmpty()) {
            QMessageBox::warning(this, "Champs requis",
                                 "Veuillez remplir tous les champs obligatoires.");
            return;
        }

        // Generate the contract
        if (ContractGenerator::generateContract(quai, clientName, company,
                                                duration, startDate, this)) {
            // Optionally update quai status to "Occupé" if needed
            if (quai.getStatut() == "Disponible") {
                int answer = QMessageBox::question(this, "Mettre à jour le statut",
                                                   "Souhaitez-vous marquer ce quai comme 'Occupé' ?",
                                                   QMessageBox::Yes | QMessageBox::No);

                if (answer == QMessageBox::Yes) {
                    // Update quai status
                    Quai& q = quais[row]; // Get non-const reference
                    q.setStatut("Occupé");
                    q.setClient(company);
                    populateTable(searchInput->text());
                }
            }
        }
    }
}
