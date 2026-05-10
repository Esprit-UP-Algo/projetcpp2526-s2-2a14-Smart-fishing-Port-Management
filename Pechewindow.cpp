#include "pechewindow.h"
#include "pechedialog.h"
#include "PecheStatisticsDialog.h"
#include "PecheExportDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QComboBox>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QSet>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

// ==================== UI HELPERS ====================
class MoveFilter : public QObject {
    QDialog* d; bool m=false; QPoint o;
public:
    MoveFilter(QDialog* dlg) : QObject(dlg), d(dlg) {}
    bool eventFilter(QObject* obj, QEvent* e) override {
        Q_UNUSED(obj);
        if (e->type()==QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(e);
            if (me->button()==Qt::LeftButton) { m=true; o=me->globalPosition().toPoint()-d->frameGeometry().topLeft(); return true; }
        } else if (e->type()==QEvent::MouseMove && m) {
            auto* me = static_cast<QMouseEvent*>(e);
            d->move(me->globalPosition().toPoint()-o); return true;
        } else if (e->type()==QEvent::MouseButtonRelease) { m=false; return true; }
        return false;
    }
};

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle) {
    if (!dialog || !dragHandle) return;
    dragHandle->setCursor(Qt::OpenHandCursor);
    dragHandle->installEventFilter(new MoveFilter(dialog));
}

static bool showStyledActionDialog(QWidget* parent, const QString& title, const QString& message,
                                   const QString& gradientStart, const QString& gradientEnd,
                                   const QString& icon, bool hasCancel = true,
                                   const QString& confirmText = QString(),
                                   const QString& cancelText = "Annuler") {
    QDialog* popup = new QDialog(parent);
    popup->setFixedSize(540, 320);
    popup->setModal(true);
    popup->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(popup);
    shadow->setBlurRadius(40); shadow->setOffset(0, 8); shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(popup);
    container->setGeometry(10, 10, 520, 300);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

    QVBoxLayout* lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0); lay->setSpacing(0);

    QFrame* hdr = new QFrame();
    hdr->setFixedHeight(64);
    hdr->setStyleSheet(QString("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2); border-radius: 20px 20px 0 0; }").arg(gradientStart, gradientEnd));
    QHBoxLayout* hdrLay = new QHBoxLayout(hdr);
    hdrLay->setContentsMargins(22, 0, 16, 0);

    QLabel* hdrIcon = new QLabel(icon);
    hdrIcon->setFont(QFont("Segoe UI", 18));
    hdrIcon->setStyleSheet("background: transparent;");
    hdrLay->addWidget(hdrIcon);

    QLabel* hdrTitle = new QLabel(title);
    hdrTitle->setFont(QFont("Segoe UI", 13, QFont::Bold));
    hdrTitle->setStyleSheet("color: white; background: transparent;");
    hdrLay->addWidget(hdrTitle, 1);

    QPushButton* xBtn = new QPushButton("✕");
    xBtn->setFixedSize(30, 30); xBtn->setCursor(Qt::PointingHandCursor);
    xBtn->setStyleSheet("QPushButton { background: rgba(255,255,255,0.2); color: white; border: none; border-radius: 15px; font-weight: bold; } QPushButton:hover { background: rgba(255,255,255,0.35); }");
    QObject::connect(xBtn, &QPushButton::clicked, popup, &QDialog::reject);
    hdrLay->addWidget(xBtn);
    lay->addWidget(hdr);
    makeDialogMovable(popup, hdr);

    QLabel* msgLbl = new QLabel(message);
    msgLbl->setWordWrap(true); msgLbl->setFont(QFont("Segoe UI", 11));
    msgLbl->setStyleSheet("color: #374151; background: transparent;");
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setContentsMargins(24, 22, 24, 6);
    lay->addWidget(msgLbl, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(20, 8, 20, 20); btnLay->setSpacing(10);
    btnLay->addStretch();

    if (hasCancel) {
        QPushButton* noBtn = new QPushButton(cancelText);
        noBtn->setFixedHeight(40); noBtn->setMinimumWidth(100);
        noBtn->setCursor(Qt::PointingHandCursor);
        noBtn->setStyleSheet("QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; font-weight: 500; } QPushButton:hover { background: #E5E7EB; }");
        QObject::connect(noBtn, &QPushButton::clicked, popup, &QDialog::reject);
        btnLay->addWidget(noBtn);
    }

    QPushButton* okBtn = new QPushButton(confirmText.isEmpty() ? (hasCancel ? "Confirmer" : "Compris") : confirmText);
    okBtn->setFixedHeight(40); okBtn->setMinimumWidth(140);
    okBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(QString(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2);
                      color: white; border: none; border-radius: 10px; padding: 0 16px; }
        QPushButton:hover { background: %1; }
    )").arg(gradientStart, gradientEnd));
    QObject::connect(okBtn, &QPushButton::clicked, popup, &QDialog::accept);
    btnLay->addWidget(okBtn);
    lay->addLayout(btnLay);

    const bool accepted = (popup->exec() == QDialog::Accepted);
    popup->deleteLater();
    return accepted;
}

PecheWindow::PecheWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    populateTable();
}

PecheWindow::~PecheWindow() {}

// ═══════════════════════════════════════════════════════════
//  Top-level UI
// ═══════════════════════════════════════════════════════════
void PecheWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Pêches");
    setMinimumSize(1000, 650);
    setStyleSheet("QMainWindow { background-color: #F0F4F8; }");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(createSidebar());
    root->addWidget(createContentArea(), 1);
}

// ═══════════════════════════════════════════════════════════
//  Sidebar
// ═══════════════════════════════════════════════════════════
QFrame* PecheWindow::createSidebar()
{
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1e4fa3, stop:1 #4a87e0);
        }
    )");

    QVBoxLayout* lay = new QVBoxLayout(sidebar);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    /* ── Logo ── */
    QFrame* logoFrame = new QFrame();
    logoFrame->setMinimumHeight(130);
    logoFrame->setMaximumHeight(160);
    logoFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* logoLay = new QVBoxLayout(logoFrame);
    logoLay->setAlignment(Qt::AlignCenter);
    logoLay->setContentsMargins(16, 12, 16, 12);

    QFrame* logoBox = new QFrame();
    logoBox->setFixedSize(160, 88);
    logoBox->setStyleSheet("QFrame{background-color:#c8cdd6;border-radius:18px;}");
    QVBoxLayout* boxLay = new QVBoxLayout(logoBox);
    boxLay->setAlignment(Qt::AlignCenter);

    QLabel* logoLbl = new QLabel();
    QPixmap px("C:/images/logo.png");
    if (!px.isNull())
        logoLbl->setPixmap(px.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    else {
        logoLbl->setText("🐟");
        logoLbl->setStyleSheet("font-size:42px;");
    }
    logoLbl->setAlignment(Qt::AlignCenter);
    logoLbl->setStyleSheet("background:transparent;");
    boxLay->addWidget(logoLbl);
    logoLay->addWidget(logoBox);
    lay->addWidget(logoFrame);

    /* ── App name ── */

    /* ── Nav ── */
    QLabel* appName = new QLabel("PortFlow");
    appName->setFont(QFont("Segoe UI", 14, QFont::Bold));
    appName->setAlignment(Qt::AlignCenter);
    appName->setStyleSheet("color:white;background:transparent;margin-bottom:6px;");
    lay->addWidget(appName);

    QFrame* nav = new QFrame();
    nav->setStyleSheet("background: transparent;");
    QVBoxLayout* navLay = new QVBoxLayout(nav);
    navLay->setSpacing(4);
    navLay->setContentsMargins(12, 12, 12, 12);

    navLay->addWidget(createNavButton("🏠", "Tableau de bord"));
    navLay->addWidget(createNavButton("⛵", "Bateaux"));
    navLay->addWidget(createNavButton("�", "Pêche", true));
    navLay->addWidget(createNavButton("👥", "Employés"));
    navLay->addWidget(createNavButton("🧊", "Frigos"));
    navLay->addWidget(createNavButton("⚙️", "Paramètres"));
    navLay->addStretch();
    navLay->addWidget(createNavButton("🚪", "Quitter", false, true));

    lay->addWidget(nav, 1);
    return sidebar;
}

QPushButton* PecheWindow::createNavButton(const QString& icon, const QString& text,
                                          bool isActive, bool isLogout)
{
    QPushButton* btn = new QPushButton(icon + "  " + text);
    btn->setFont(QFont("Segoe UI", 11, isActive ? QFont::Bold : QFont::Medium));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(48);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton{background:rgba(255,255,255,0.08);color:white;border:none;
                border-radius:10px;text-align:left;padding-left:16px;}
            QPushButton:hover{background:rgba(239,68,68,0.75);}
        )");
        connect(btn, &QPushButton::clicked, this, &PecheWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton{background:rgba(255,255,255,0.22);color:white;border:none;
                border-radius:10px;text-align:left;padding-left:16px;}
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton{background:transparent;color:rgba(255,255,255,0.88);border:none;
                border-radius:10px;text-align:left;padding-left:16px;}
            QPushButton:hover{background:rgba(255,255,255,0.12);}
        )");
    }
    return btn;
}

// ═══════════════════════════════════════════════════════════
//  Content area (scrollable)
// ═══════════════════════════════════════════════════════════
QWidget* PecheWindow::createContentArea()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:#F0F4F8;border:none;}"
                          "QScrollBar:vertical{width:6px;background:transparent;}"
                          "QScrollBar::handle:vertical{background:#c1c9d6;border-radius:3px;}");

    QWidget* inner = new QWidget();
    inner->setStyleSheet("background:#F0F4F8;");

    QVBoxLayout* lay = new QVBoxLayout(inner);
    lay->setSpacing(18);
    lay->setContentsMargins(28, 24, 28, 24);

    lay->addWidget(createHeader());
    lay->addWidget(createToolbar());
    lay->addWidget(createTableCard(), 1);

    scroll->setWidget(inner);
    return scroll;
}

// ─── Title row ────────────────────────────────────────────
QFrame* PecheWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Pêches");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Journal des captures, lots et catégories");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color:#6b7280;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    /* Action buttons */
    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(45);
        btn->setMinimumWidth(160);
        btn->setStyleSheet(QString(
            "QPushButton{"
            "  background:%1; color:white; border:none; border-radius:12px; padding:0 20px;"
            "}"
            "QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* addBtn   = makeBtn("➕  Nouveau Lot",     "#2563EB", "#1D4ED8");

    connect(addBtn,   &QPushButton::clicked, this, &PecheWindow::onAddPeche);

    lay->addWidget(addBtn);
    return hdr;
}

// ─── Filter / Sort toolbar ─────────────────────────────────
QFrame* PecheWindow::createToolbar()
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
    searchInput->setPlaceholderText("🔍  Rechercher par catégorie, référence...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &PecheWindow::onSearch);
    lay->addWidget(searchInput);

    auto makeToolBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 9, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(40);
        btn->setStyleSheet(QString(
            "QPushButton{ background:%1; color:white; border:none; border-radius:10px; padding:0 15px; }"
            "QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* statsBtn = makeToolBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* pdfBtn   = makeToolBtn("📄  PDF", "#059669", "#047857");

    connect(statsBtn, &QPushButton::clicked, this, &PecheWindow::onShowStatistics);
    connect(pdfBtn,   &QPushButton::clicked, this, &PecheWindow::onGeneratePDF);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({
        "Par défaut",
        "Quantité ↑",
        "Quantité ↓",
        "Date ↑ (ancienne)",
        "Date ↓ (récente)",
        "Catégorie (A→Z)",
        "Catégorie (Z→A)"
    });
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PecheWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

// ─── Table card ────────────────────────────────────────────
QFrame* PecheWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet(R"(
        QFrame {
            background-color: #5D9CEC;
            border-radius: 20px;
            padding: 3px;
        }
    )");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

void PecheWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        "Référence", "Catégorie", "Quantité (Kg)", "Date", "Bateau", "Frigo", "Actions", "ID_BAT", "ID_FRI", "ID_PECH"
    });
    table->setColumnHidden(7, true);
    table->setColumnHidden(8, true);
    table->setColumnHidden(9, true);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);
    table->setFrameShape(QFrame::NoFrame);

    table->horizontalHeader()->setFont(QFont("Segoe UI", 11, QFont::Bold));
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

    table->setColumnWidth(1, 160);
    table->setColumnWidth(2, 160);
    table->setColumnWidth(3, 180);
    table->setColumnWidth(4, 100);
}

// ═══════════════════════════════════════════════════════════
//  Populate
// ═══════════════════════════════════════════════════════════
void PecheWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    QSqlQueryModel* model;
    if (!filterText.isEmpty()) {
        model = pecheModel.rechercher(filterText);
    } else {
        int si = sortCombo ? sortCombo->currentIndex() : 0;
        if (si == 0) model = pecheModel.afficher();
        else {
            QString critere = "ID", ordre = "ASC";
            if (si == 1) { critere = "Quantité";  ordre = "ASC"; }
            else if (si == 2) { critere = "Quantité";  ordre = "DESC"; }
            else if (si == 3) { critere = "Date";      ordre = "ASC"; }
            else if (si == 4) { critere = "Date";      ordre = "DESC"; }
            else if (si == 5) { critere = "Espèce";    ordre = "ASC"; }
            else if (si == 6) { critere = "Espèce";    ordre = "DESC"; }
            model = pecheModel.trier(critere, ordre);
        }
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 58);

        QString id        = model->record(i).value("ID").toString();
        QString reference = model->record(i).value("Référence").toString();
        QString espece    = model->record(i).value("Espèce").toString();
        QString quantite  = model->record(i).value("Quantité").toString();
        QString date      = model->record(i).value("Date").toDate().toString("dd/MM/yyyy");
        QString boat      = model->record(i).value("Bateau").toString();
        QString fridge    = model->record(i).value("Frigo").toString();

        auto it = [&](const QString& t) { QTableWidgetItem* item = new QTableWidgetItem(t); item->setFont(cellFont); return item; };
        
        table->setItem(r, 0, it(reference));
        table->item(r, 0)->setData(Qt::UserRole, id);
        table->setCellWidget(r, 1, createEspeceBadge(espece));
        
        QTableWidgetItem* qItem = it(quantite + " Kg");
        qItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        qItem->setForeground(QBrush(QColor("#166534")));
        table->setItem(r, 2, qItem);

        table->setItem(r, 3, it(date));
        
        // Stocker les IDs pour l'édition dans le UserRole des colonnes correspondantes
        QTableWidgetItem* bItem = it(boat);
        bItem->setData(Qt::UserRole, model->record(i).value("IDBATEAU").toString());
        bItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(r, 4, bItem);
        
        QTableWidgetItem* fItem = it(fridge);
        fItem->setData(Qt::UserRole, model->record(i).value("IDFRIGO").toString());
        fItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(r, 5, fItem);

        // Stocker l'ID du pêcheur dans la colonne cachée 9
        QTableWidgetItem* pItem = it("");
        pItem->setData(Qt::UserRole, model->record(i).value("ID_PECHEUR").toString());
        table->setItem(r, 9, pItem);

        table->setCellWidget(r, 6, createActionButtons(r));
    }
    delete model;
}

QWidget* PecheWindow::createEspeceBadge(const QString& esp)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setAlignment(Qt::AlignCenter);
    QLabel* b = new QLabel(esp);
    b->setFont(QFont("Segoe UI",9,QFont::Medium));
    b->setFixedHeight(28); b->setAlignment(Qt::AlignCenter);
    
    static const QMap<QString, QString> ss = {
        {"Sardine",  "background:#dbeafe;color:#1e40af;"},
        {"Thon",     "background:#fee2e2;color:#991b1b;"},
        {"Merlan",   "background:#fef9c3;color:#854d0e;"},
        {"Crevette", "background:#fce7f3;color:#9f1239;"},
        {"Saumon",   "background:#ffedd5;color:#9a3412;"},
    };
    b->setStyleSheet(QString("QLabel{%1 border-radius:7px; padding:2px 12px;}").arg(ss.value(esp, "background:#f1f5f9;color:#475569;")));
    l->addWidget(b); return w;
}

QWidget* PecheWindow::createActionButtons(int row)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setSpacing(6); l->setAlignment(Qt::AlignCenter);
    auto mk = [](const QString& ic, const QString& bg, const QString& hov) {
        QPushButton* b = new QPushButton(ic);
        b->setFixedSize(32,32); b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString("QPushButton{background:%1;border:none;border-radius:8px;font-size:14px;} QPushButton:hover{background:%2;}").arg(bg,hov));
        return b;
    };
    QPushButton* e = mk("✏️","#fef9c3","#fde68a");
    QPushButton* d = mk("🗑️","#fee2e2","#fecaca");
    connect(e,&QPushButton::clicked,[this,row](){ onEditPeche(row); });
    connect(d,&QPushButton::clicked,[this,row](){ onDeletePeche(row); });
    l->addWidget(e); l->addWidget(d);
    return w;
}

QString PecheWindow::generatePecheId() {
    // Return a purely numeric random ID (5 digits) to match user request "num aleatoire"
    // and resolve any potential NUMBER column issues if the schema uses numbers for IDLOT
    return QString::number(QDateTime::currentMSecsSinceEpoch()).right(5);
}

void PecheWindow::onSearch(const QString& t) { populateTable(t); }
void PecheWindow::onSort(int) { populateTable(searchInput->text()); }

void PecheWindow::onGeneratePDF()
{
    PecheExportDialog dlg(this);
    if(dlg.exec() != QDialog::Accepted) return;

    QString path = QFileDialog::getSaveFileName(this,"Exporter PDF","rapport_peches.pdf","PDF (*.pdf)");
    if(path.isEmpty()) return;

    // ── Dynamic Query Construction ─────────────────────────────
    QString queryStr = "SELECT p.*, b.NOMBATEAU, q.LOCATION "
                       "FROM PECHES p "
                       "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                       "LEFT JOIN QUAIS q ON b.IDQUAI = q.IDQUAI ";
    QStringList filters;

    if (dlg.exportType() == PecheExportDialog::ByBoat || dlg.exportType() == 3) { // 3 = ByBoatAndDate
        filters << QString("p.IDBATEAU = '%1'").arg(dlg.selectedBoat());
    }
    
    if (dlg.exportType() == PecheExportDialog::ByDate || dlg.exportType() == 3) {
        filters << QString("p.DATECAPTURE BETWEEN TO_DATE('%1','YYYY-MM-DD') AND TO_DATE('%2','YYYY-MM-DD')")
                   .arg(dlg.startDate().toString("yyyy-MM-dd"))
                   .arg(dlg.endDate().toString("yyyy-MM-dd"));
    }

    if (!filters.isEmpty()) {
        queryStr += " WHERE " + filters.join(" AND ");
    }

    // Titles
    QString rpTitle = "Justificatif de Pêche";
    QString rpSub   = "Rapport global — toutes les captures";

    if (dlg.exportType() == PecheExportDialog::ByBoat) {
        QSqlQuery nameQ;
        nameQ.prepare("SELECT NOMBATEAU FROM BATEAUX WHERE IDBATEAU = :id");
        nameQ.bindValue(":id", dlg.selectedBoat());
        QString bName = dlg.selectedBoat();
        if(nameQ.exec() && nameQ.next()) bName = nameQ.value(0).toString();
        rpTitle = "Rapport de Pêche — Bateau " + bName;
        rpSub   = "Détails des captures pour ce bâtiment";
    } else if (dlg.exportType() == PecheExportDialog::ByDate) {
        rpTitle = "Rapport de Pêche — Période";
        rpSub   = QString("Du %1 au %2")
                  .arg(dlg.startDate().toString("dd/MM/yyyy"))
                  .arg(dlg.endDate().toString("dd/MM/yyyy"));
    } else if (dlg.exportType() == 3) {
        rpTitle = "Rapport Combiné — Bateau & Période";
        rpSub   = "Filtré par bâtiment et dates spécifiques";
    }

    QSqlQueryModel mdl;
    mdl.setQuery(queryStr);
    if (mdl.lastError().isValid()) {
        QMessageBox::critical(this, "Erreur SQL", "Impossible de générer le rapport :\n" + mdl.lastError().text());
        return;
    }

    // ── PDF writer setup ───────────────────────────────────────
    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(12,12,12,12), QPageLayout::Millimeter);
    writer.setResolution(96);

    QPainter p(&writer);
    p.setRenderHint(QPainter::Antialiasing);
    const int W  = writer.width();
    const int H  = writer.height();
    const int mX = 50;           // left/right margin in painter coords
    const int cW = W - 2*mX;    // usable content width

    // ═══════════════════════════════════════════════════════════
    //  1. HEADER BANNER
    // ═══════════════════════════════════════════════════════════
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1e3a8a"));
    p.drawRect(0, 0, W, 180);

    // Top micro-label
    p.setPen(QColor("#93c5fd"));
    p.setFont(QFont("Segoe UI", 8));
    p.drawText(mX, 14, cW, 24, Qt::AlignLeft | Qt::AlignVCenter,
               "Smart Fishing Port Management System");

    // Main title
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 24, QFont::Bold));
    p.drawText(mX, 38, cW, 68, Qt::AlignCenter, rpTitle);

    // Subtitle
    p.setPen(QColor("#bfdbfe"));
    p.setFont(QFont("Segoe UI", 11));
    p.drawText(mX, 108, cW, 36, Qt::AlignCenter, rpSub);

    // Generation date (bottom-right of banner)
    p.setPen(QColor("#93c5fd"));
    p.setFont(QFont("Segoe UI", 8));
    p.drawText(mX, 150, cW, 24, Qt::AlignRight | Qt::AlignVCenter,
               "Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm"));

    // Accent stripe at bottom of banner
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#3b82f6"));
    p.drawRect(0, 176, W, 4);

    int y = 205;   // cursor below banner

    // ═══════════════════════════════════════════════════════════
    //  2. STATISTICS CARD
    // ═══════════════════════════════════════════════════════════
    double tot = 0;
    QSet<QString> days;
    for(int i = 0; i < mdl.rowCount(); ++i){
        tot += mdl.record(i).value("QUANTITE").toDouble();
        days.insert(mdl.record(i).value("DATECAPTURE").toDate().toString("dd/MM/yyyy"));
    }
    int   nbr = mdl.rowCount();
    double avg = tot / qMax(1, days.size());

    const int cardH = 132;
    // Card shadow effect (offset rect)
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0,0,0,18));
    p.drawRoundedRect(mX+3, y+3, cW, cardH, 10, 10);
    // Card body
    p.setBrush(QColor("#f0f9ff"));
    p.setPen(QPen(QColor("#bae6fd"), 2));
    p.drawRoundedRect(mX, y, cW, cardH, 10, 10);
    // Card header band
    p.setPen(Qt::NoPen); p.setBrush(QColor("#dbeafe"));
    p.drawRoundedRect(mX, y, cW, 38, 10, 10);
    p.drawRect(mX, y+18, cW, 20);  // square bottom corners of the band

    p.setPen(QColor("#1e40af"));
    p.setFont(QFont("Segoe UI", 11, QFont::Bold));
    p.drawText(mX, y, cW, 38, Qt::AlignCenter, "Statistiques Globales");
    y += 46;

    // Three stat columns
    int col3 = cW / 3;
    struct Col { const char* label; QString val; const char* color; };
    QList<Col> cols = {
        { "Total captures",  QString::number(nbr),                "#1d4ed8" },
        { "Poids total",     QString::number(tot,'f',1) + " Kg",  "#0886ab" },
        { "Moyenne / jour",  QString::number(avg,'f',1) + " Kg",  "#059669" }
    };
    for(int c = 0; c < cols.size(); ++c){
        int x0 = mX + c * col3;
        p.setPen(QColor("#6b7280"));
        p.setFont(QFont("Segoe UI", 9));
        p.drawText(x0+8, y,    col3-16, 24, Qt::AlignCenter, cols[c].label);
        p.setPen(QColor(cols[c].color));
        p.setFont(QFont("Segoe UI", 16, QFont::Bold));
        p.drawText(x0+8, y+24, col3-16, 36, Qt::AlignCenter, cols[c].val);
    }
    // Dividers between stat columns
    p.setPen(QPen(QColor("#bae6fd"), 1));
    p.drawLine(mX+col3,   y, mX+col3,   y+64);
    p.drawLine(mX+col3*2, y, mX+col3*2, y+64);

    y += 80;   // below card

    // ── Boat Details (New Section) ──────────────────────────────
    if (mdl.rowCount() > 0 && dlg.exportType() == PecheExportDialog::ByBoat) {
        p.setPen(QColor("#1e3a8a")); p.setFont(QFont("Segoe UI", 12, QFont::Bold));
        p.drawText(mX, y, "Informations du Bateau :"); y += 30;
        p.setPen(QColor("#1f2937")); p.setFont(QFont("Segoe UI", 10));
        p.drawText(mX + 20, y, "Bateau ID : " + dlg.selectedBoat());
        p.drawText(mX + 250, y, "Quai : " + mdl.record(0).value("LOCATION").toString());
        y += 50;
    }

    // ═══════════════════════════════════════════════════════════
    //  3. TABLE
    // ═══════════════════════════════════════════════════════════
    // Column proportions (must total cW)
    bool isTotal = (dlg.exportType() == PecheExportDialog::Total || dlg.exportType() == PecheExportDialog::ByDate);
    QList<int> cw;
    QStringList hd;
    
    if (isTotal) {
        cw = { cW*18/100, cW*22/100, cW*20/100, cW*22/100, cW*18/100 };
        hd = { "Référence", "Catégorie", "Quantité", "Date", "Bateau" };
    } else {
        cw = { cW*22/100, cW*26/100, cW*24/100, cW*28/100 };
        hd = { "Référence", "Catégorie", "Quantité (Kg)", "Date" };
    }
    
    // Si c'est combiné, on préfère afficher le bateau aussi ? 
    // Non, restons sur la logique initiale : si ByBoat (ou combiné), on cache le bateau car il est en titre.
    if (dlg.exportType() == 3) {
        cw = { cW*22/100, cW*26/100, cW*24/100, cW*28/100 };
        hd = { "Référence", "Catégorie", "Quantité (Kg)", "Date" };
        isTotal = false;
    }
    const int rH = 40;

    // Helper: draw table header
    auto drawTableHeader = [&](){
        p.setPen(Qt::NoPen); p.setBrush(QColor("#1e40af"));
        p.drawRect(mX, y, cW, rH);
        p.setPen(Qt::white); p.setFont(QFont("Segoe UI", 10, QFont::Bold));
        int cx = mX;
        for(int c = 0; c < hd.size(); ++c){
            p.drawText(cx+12, y, cw[c]-12, rH, Qt::AlignVCenter|Qt::AlignLeft, hd[c]);
            cx += cw[c];
        }
        y += rH;
    };
    drawTableHeader();

    int tableTopY = y;   // remember where data rows start
    p.setFont(QFont("Segoe UI", 10));
    bool alt = false;
    for(int i = 0; i < mdl.rowCount(); ++i){
        // New page if needed
        if(y + rH > H - 80){
            // border around table so far
            p.setPen(QPen(QColor("#bfdbfe"), 2)); p.setBrush(Qt::NoBrush);
            p.drawRect(mX, tableTopY - rH, cW, y - tableTopY + rH);
            writer.newPage(); y = 40; tableTopY = y + rH;
            drawTableHeader();
            p.setFont(QFont("Segoe UI", 10));
        }
        // Row background
        p.setPen(Qt::NoPen);
        p.setBrush(alt ? QColor("#f0f9ff") : Qt::white);
        p.drawRect(mX, y, cW, rH);
        // Row text
        QStringList vals;
        if (isTotal) {
            vals = {
                mdl.record(i).value("REFERENCE").toString(),
                mdl.record(i).value("ESPECE").toString(),
                mdl.record(i).value("QUANTITE").toString() + " Kg",
                mdl.record(i).value("DATECAPTURE").toDate().toString("dd/MM/yyyy"),
                mdl.record(i).value("NOMBATEAU").toString()
            };
        } else {
            vals = {
                mdl.record(i).value("REFERENCE").toString(),
                mdl.record(i).value("ESPECE").toString(),
                mdl.record(i).value("QUANTITE").toString() + " Kg",
                mdl.record(i).value("DATECAPTURE").toDate().toString("dd/MM/yyyy")
            };
        }
        p.setPen(QColor("#1f2937"));
        int cx = mX;
        for(int c = 0; c < vals.size(); ++c){
            p.drawText(cx+12, y, cw[c]-12, rH, Qt::AlignVCenter|Qt::AlignLeft, vals[c]);
            cx += cw[c];
        }
        // Row separator
        p.setPen(QPen(QColor("#e2e8f0"), 1));
        p.drawLine(mX, y+rH, mX+cW, y+rH);
        y += rH; alt = !alt;
    }
    // Table outer border
    p.setPen(QPen(QColor("#bfdbfe"), 2)); p.setBrush(Qt::NoBrush);
    p.drawRect(mX, tableTopY-rH, cW, y - tableTopY + rH);

    // ═══════════════════════════════════════════════════════════
    //  4. FOOTER
    // ═══════════════════════════════════════════════════════════
    p.setPen(QPen(QColor("#e2e8f0"), 1));
    p.drawLine(mX, H-58, mX+cW, H-58);
    p.setPen(QColor("#9ca3af")); p.setFont(QFont("Segoe UI", 8));
    p.drawText(mX, H-50, cW/2, 28, Qt::AlignLeft|Qt::AlignVCenter,
               "Smart Fishing Port Management System");
    p.drawText(mX+cW/2, H-50, cW/2, 28, Qt::AlignRight|Qt::AlignVCenter, "Page 1");

    p.end();
    QMessageBox::information(this, "PDF généré", "Rapport enregistré avec succès :\n" + path);
}

void PecheWindow::onShowStatistics()
{
    QMap<QString, double> speciesCount, weightBySpecies;
    QSqlQueryModel* model = pecheModel.afficher();
    
    for(int i = 0; i < model->rowCount(); ++i){
        QString esp = model->record(i).value("Espèce").toString();
        double qte = model->record(i).value("Quantité").toDouble();
        
        speciesCount[esp]++;
        weightBySpecies[esp] += qte;
    }
    
    delete model;
    PecheStatisticsDialog dlg(speciesCount, weightBySpecies, this);
    dlg.exec();
}

void PecheWindow::onAddPeche() {
    PecheDialog d(this);
    if(d.exec()==QDialog::Accepted){ 
        Peche np=d.getData(); 
        np.setIdLot(generatePecheId()); 
        if (np.ajouter()) {
            QMessageBox::information(this, "OK", "Ajout effectué\nClick Cancel to exit.", QMessageBox::Cancel);
            populateTable(searchInput->text()); 
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le lot: " + Peche::getLastError());
        }
    }
}
void PecheWindow::onEditPeche(int row) {
    if(row<0) return;
    QString id = table->item(row, 0)->data(Qt::UserRole).toString();
    
    // Create a temporary Peche object for the dialog
    Peche current;
    current.setIdLot(id);
    current.setReference(table->item(row, 0)->text());
    // Get species from badge widget
    QWidget* badgeW = table->cellWidget(row, 1);
    QLabel* badgeL = badgeW ? badgeW->findChild<QLabel*>() : nullptr;
    current.setEspece(badgeL ? badgeL->text() : "");
    
    QString qteStr = table->item(row, 2)->text();
    qteStr.replace(" Kg", "");
    current.setQuantiteKg(qteStr);
    current.setDateCapture(table->item(row, 3)->text());
    current.setIdBateau(table->item(row, 4)->data(Qt::UserRole).toString());
    current.setIdFrigo(table->item(row, 5)->data(Qt::UserRole).toString());
    // Récupérer l'ID du pêcheur depuis la colonne cachée
    if (table->item(row, 9))
        current.setIdPecheur(table->item(row, 9)->data(Qt::UserRole).toString());

    PecheDialog d(this, &current);
    if(d.exec()==QDialog::Accepted){ 
        Peche u=d.getData(); 
        if (u.modifier(id)) {
            QMessageBox::information(this, "OK", "Modification effectuée\nClick Cancel to exit.", QMessageBox::Cancel);
            populateTable(searchInput->text());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de modifier le lot: " + Peche::getLastError());
        }
    }
}
void PecheWindow::onDeletePeche(int row) {
    if(row<0) return;
    QString ref = table->item(row, 0)->text();
    QString id = table->item(row, 0)->data(Qt::UserRole).toString();
    
    if (showStyledActionDialog(this, "Suppression Lot", 
                               QString("Voulez-vous vraiment supprimer le lot de pêche <b>%1</b> ?").arg(ref),
                               "#EF4444", "#B91C1C", "🗑️")) {
        if (pecheModel.supprimer(id)) {
            showStyledActionDialog(this, "Succès", "Le lot a été supprimé avec succès.", "#10B981", "#059669", "✅", false);
            populateTable(searchInput->text());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer le lot: " + Peche::getLastError());
        }
    }
}
void PecheWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}
