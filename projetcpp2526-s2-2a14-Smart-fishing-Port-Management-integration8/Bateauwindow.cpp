#include "bateauwindow.h"
#include "bateaudialog.h"
#include "StatisticsDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QComboBox>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDate>
#include <QDateTime>
#include <QScrollArea>
#include <QSizePolicy>
#include <algorithm>

BateauWindow::BateauWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    bateaux.append({"B001","Neptune",  "TN-001","50","20","Ahmed Ben Ali",   "Au port",       "15/01/2025","Oui","8"});
    bateaux.append({"B002","Poséidon","TN-002","70","25","Mohamed Trabelsi","En mer",         "10/01/2025","Oui","5"});
    bateaux.append({"B003","Triton",  "TN-003","45","18","Karim Gharbi",    "En maintenance","20/12/2024","Non","12"});
    populateTable();
}
BateauWindow::~BateauWindow() {}

// ═══════════════════════════════════════════════════════════
//  Top-level UI
// ═══════════════════════════════════════════════════════════
void BateauWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Bateaux");
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
QFrame* BateauWindow::createSidebar()
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
        logoLbl->setText("⛵");
        logoLbl->setStyleSheet("font-size:42px;");
    }
    logoLbl->setAlignment(Qt::AlignCenter);
    logoLbl->setStyleSheet("background:transparent;");
    boxLay->addWidget(logoLbl);
    logoLay->addWidget(logoBox);
    lay->addWidget(logoFrame);

    /* ── App name ── */
    QLabel* appName = new QLabel("PortFlow");
    appName->setFont(QFont("Segoe UI", 14, QFont::Bold));
    appName->setAlignment(Qt::AlignCenter);
    appName->setStyleSheet("color:white;background:transparent;margin-bottom:6px;");
    lay->addWidget(appName);

    /* ── Nav ── */
    QFrame* nav = new QFrame();
    nav->setStyleSheet("background: transparent;");
    QVBoxLayout* navLay = new QVBoxLayout(nav);
    navLay->setSpacing(4);
    navLay->setContentsMargins(12, 12, 12, 12);

    navLay->addWidget(createNavButton("🏠", "Tableau de bord"));
    navLay->addWidget(createNavButton("⛵", "Bateaux", true));
    navLay->addWidget(createNavButton("🐟", "Pêche"));
    navLay->addWidget(createNavButton("👥", "Employés"));
    navLay->addWidget(createNavButton("🧊", "Frigos"));
    navLay->addWidget(createNavButton("⚙️", "Paramètres"));
    navLay->addStretch();
    navLay->addWidget(createNavButton("🚪", "Quitter", false, true));

    lay->addWidget(nav, 1);
    return sidebar;
}

QPushButton* BateauWindow::createNavButton(const QString& icon, const QString& text,
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
        connect(btn, &QPushButton::clicked, this, &BateauWindow::onLogout);
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
QWidget* BateauWindow::createContentArea()
{
    /* Outer scroll area so nothing clips when window is small */
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
QFrame* BateauWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Bateaux");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Suivi de la flotte, états et disponibilités");
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

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* pdfBtn   = makeBtn("📄  Exporter PDF",  "#059669", "#047857");
    QPushButton* addBtn   = makeBtn("➕  Nouveau Bateau",  "#2563EB", "#1D4ED8");

    connect(statsBtn, &QPushButton::clicked, this, &BateauWindow::onShowStatistics);
    connect(pdfBtn,   &QPushButton::clicked, this, &BateauWindow::onGeneratePDF);
    connect(addBtn,   &QPushButton::clicked, this, &BateauWindow::onAddBateau);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

// ─── Filter / Sort toolbar ─────────────────────────────────
QFrame* BateauWindow::createToolbar()
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

    /* Search input */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher un bateau par nom, immatriculation...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchInput->setStyleSheet(R"(
        QLineEdit{
            background:#ffffff;
            border:2px solid #e2e8f0;
            border-radius:12px;
            padding:4px 16px;
            color:#1f2937;
        }
        QLineEdit:focus{
            border:2px solid #2563EB;
            background:white;
        }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &BateauWindow::onSearch);
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
    sortCombo->addItems({
        "Défaut",
        "Capacité de pêche (Croissante)",
        "Capacité de pêche (Décroissante)",
        "Date maintenance (Récente)",
        "Date maintenance (Ancienne)",
        "Âge (Croissant)",
        "Âge (Décroissant)"
    });
    sortCombo->setStyleSheet(R"(
        QComboBox{
            background:transparent;
            border:none;
            padding:4px 12px;
            color:#1f2937;
            font-weight: 600;
        }
        QComboBox:hover {
            color: #2563EB;
        }
        QComboBox::drop-down{
            border:none;
            width:30px;
        }
        QComboBox QAbstractItemView{
            background:white;
            border:1px solid #e2e8f0;
            border-radius:12px;
            selection-background-color:#eff6ff;
            selection-color:#2563EB;
            outline: none;
            padding: 8px;
        }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BateauWindow::onSort);
    lay->addWidget(sortCombo, 2);

    return bar;
}

// ─── Table card ────────────────────────────────────────────
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

void BateauWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        "Nom","Immatriculation","Capacité (t)","Longueur (m)",
        "Propriétaire","État","Dernière Maint.","Âge (ans)","Disponible","Actions"
    });

    /* All columns resize to their content; horizontal scrollbar appears when needed */
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Give text columns a sensible minimum so they never collapse
    table->horizontalHeader()->setMinimumSectionSize(90);

    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);
    table->setFrameShape(QFrame::NoFrame);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
}

// ═══════════════════════════════════════════════════════════
//  Populate
// ═══════════════════════════════════════════════════════════
void BateauWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    QVector<int> idx;
    for (int i = 0; i < bateaux.size(); ++i) {
        const Bateau& b = bateaux[i];
        if (!filterText.isEmpty()) {
            QString f = filterText.toLower();
            if (!b.nomBateau.toLower().contains(f) &&
                !b.immatriculation.toLower().contains(f) &&
                !b.proprietaire.toLower().contains(f))
                continue;
        }
        idx.append(i);
    }

    int si = sortCombo ? sortCombo->currentIndex() : 0;
    if (si == 1) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return bateaux[a].capacitePeche.toInt()<bateaux[b].capacitePeche.toInt(); });
    else if (si==2) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return bateaux[a].capacitePeche.toInt()>bateaux[b].capacitePeche.toInt(); });
    else if (si==3) std::sort(idx.begin(),idx.end(),[this](int a,int b){
        return QDate::fromString(bateaux[a].dateDerniereMaintenance,"dd/MM/yyyy")
             > QDate::fromString(bateaux[b].dateDerniereMaintenance,"dd/MM/yyyy"); });
    else if (si==4) std::sort(idx.begin(),idx.end(),[this](int a,int b){
        return QDate::fromString(bateaux[a].dateDerniereMaintenance,"dd/MM/yyyy")
             < QDate::fromString(bateaux[b].dateDerniereMaintenance,"dd/MM/yyyy"); });
    else if (si==5) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return bateaux[a].ageBateau.toInt()<bateaux[b].ageBateau.toInt(); });
    else if (si==6) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return bateaux[a].ageBateau.toInt()>bateaux[b].ageBateau.toInt(); });

    for (int i : idx) {
        const Bateau& b = bateaux[i];
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 58);

        auto item = [&](const QString& t) {
            QTableWidgetItem* it = new QTableWidgetItem(t);
            it->setFont(cellFont);
            return it;
        };
        table->setItem(r,0,item(b.nomBateau));
        table->item(r,0)->setData(Qt::UserRole, i);
        table->setItem(r,1,item(b.immatriculation));
        table->setItem(r,2,item(b.capacitePeche+" t"));
        table->setItem(r,3,item(b.longueur+" m"));
        table->setItem(r,4,item(b.proprietaire));
        table->setCellWidget(r,5,createStatusBadge(b.etatBateau));
        table->setItem(r,6,item(b.dateDerniereMaintenance));
        table->setItem(r,7,item(b.ageBateau+" ans"));
        table->setCellWidget(r,8,createDisponibleBadge(b.disponible));
        table->setCellWidget(r,9,createActionButtons(i));
    }
}

// ═══════════════════════════════════════════════════════════
//  Badges & action buttons
// ═══════════════════════════════════════════════════════════
QWidget* BateauWindow::createStatusBadge(const QString& etat)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setAlignment(Qt::AlignCenter);
    QLabel* b = new QLabel(etat);
    b->setFont(QFont("Segoe UI",9,QFont::Medium));
    b->setFixedHeight(28); b->setAlignment(Qt::AlignCenter);
    if (etat=="En mer")
        b->setStyleSheet("QLabel{background:#dbeafe;color:#1e40af;border-radius:7px;padding:2px 12px;}");
    else if (etat=="Au port")
        b->setStyleSheet("QLabel{background:#dcfce7;color:#166534;border-radius:7px;padding:2px 12px;}");
    else
        b->setStyleSheet("QLabel{background:#fef9c3;color:#854d0e;border-radius:7px;padding:2px 12px;}");
    l->addWidget(b); return w;
}

QWidget* BateauWindow::createDisponibleBadge(const QString& dispo)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setAlignment(Qt::AlignCenter);
    QLabel* b = new QLabel(dispo=="Oui" ? "✔  Oui" : "✘  Non");
    b->setFont(QFont("Segoe UI",9,QFont::Medium));
    b->setFixedHeight(28); b->setAlignment(Qt::AlignCenter);
    b->setStyleSheet(dispo=="Oui"
        ? "QLabel{background:#dcfce7;color:#166534;border-radius:7px;padding:2px 12px;}"
        : "QLabel{background:#fee2e2;color:#991b1b;border-radius:7px;padding:2px 12px;}");
    l->addWidget(b); return w;
}

QWidget* BateauWindow::createActionButtons(int row)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setSpacing(6); l->setAlignment(Qt::AlignCenter);

    auto mk = [](const QString& ic, const QString& bg, const QString& hov) {
        QPushButton* b = new QPushButton(ic);
        b->setFixedSize(32,32); b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(
            "QPushButton{background:%1;border:none;border-radius:8px;font-size:14px;}"
            "QPushButton:hover{background:%2;}").arg(bg,hov));
        return b;
    };
    QPushButton* e = mk("✏️","#fef9c3","#fde68a");
    QPushButton* d = mk("🗑️","#fee2e2","#fecaca");
    connect(e,&QPushButton::clicked,[this,row](){ onEditBateau(row); });
    connect(d,&QPushButton::clicked,[this,row](){ onDeleteBateau(row); });
    l->addWidget(e); l->addWidget(d);
    return w;
}

// ═══════════════════════════════════════════════════════════
//  Slots
// ═══════════════════════════════════════════════════════════
QString BateauWindow::generateBateauId()
{
    int mx=0;
    for(const Bateau&b:bateaux){ int n=b.idBateau.mid(1).toInt(); if(n>mx)mx=n; }
    return QString("B%1").arg(mx+1,3,10,QChar('0'));
}

void BateauWindow::onSearch(const QString& t){ populateTable(t); }
void BateauWindow::onSort(int){ populateTable(searchInput ? searchInput->text() : ""); }

void BateauWindow::onGeneratePDF()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Enregistrer PDF", "rapport_maintenance.pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;

    // Collect maintenance boats first
    QVector<const Bateau*> liste;
    for (const Bateau& b : bateaux)
        if (b.etatBateau == "En maintenance") liste.append(&b);

    QPdfWriter w(path);
    w.setPageSize(QPageSize(QPageSize::A4));
    w.setPageOrientation(QPageLayout::Landscape);
    w.setPageMargins(QMarginsF(12,12,12,12), QPageLayout::Millimeter);
    w.setResolution(96);

    QPainter p(&w);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = w.width();
    int y = 30;

    // ── Title block ──────────────────────────────────────────
    // Blue banner
    p.setBrush(QColor("#1e40af")); p.setPen(Qt::NoPen);
    p.drawRoundedRect(0, y, W, 70, 8, 8);

    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 20, QFont::Bold));
    p.drawText(0, y, W, 44, Qt::AlignHCenter | Qt::AlignVCenter,
               "⚙  Rapport — Bateaux en Maintenance");

    p.setFont(QFont("Segoe UI", 9));
    p.drawText(0, y + 44, W, 26, Qt::AlignHCenter | Qt::AlignVCenter,
               "Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy à hh:mm")
               + "   |   Total : " + QString::number(liste.size()) + " bateau(x)");
    y += 82;

    // ── Column setup ─────────────────────────────────────────
    // 8 data columns
    QStringList hd = {"Nom", "Immat.", "Capacité (t)", "Longueur (m)",
                       "Propriétaire", "Âge (ans)", "Dernière Maint.", "Disponible"};
    // Proportional widths (must sum to W)
    QList<int> cw;
    cw << int(W * 0.14)   // Nom
       << int(W * 0.10)   // Immat
       << int(W * 0.10)   // Capacité
       << int(W * 0.10)   // Longueur
       << int(W * 0.20)   // Propriétaire
       << int(W * 0.09)   // Âge
       << int(W * 0.14)   // Date maint
       << int(W * 0.10);  // Disponible
    // Absorb rounding into last column
    int used = 0; for (int v : cw) used += v;
    cw.last() += (W - used);

    const int rh = 36;

    // ── Header row ───────────────────────────────────────────
    p.setBrush(QColor("#1e3a8a")); p.setPen(Qt::NoPen);
    p.drawRect(0, y, W, rh);
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 9, QFont::Bold));
    int cx = 0;
    for (int c = 0; c < hd.size(); ++c) {
        p.drawText(cx + 8, y, cw[c] - 8, rh, Qt::AlignVCenter, hd[c]);
        cx += cw[c];
    }
    y += rh;

    // ── Data rows ────────────────────────────────────────────
    p.setFont(QFont("Segoe UI", 9));
    bool alt = false;
    if (liste.isEmpty()) {
        p.setBrush(Qt::white); p.setPen(Qt::NoPen); p.drawRect(0, y, W, rh * 2);
        p.setPen(QColor("#6b7280"));
        p.setFont(QFont("Segoe UI", 11));
        p.drawText(0, y, W, rh * 2, Qt::AlignCenter,
                   "Aucun bateau en maintenance actuellement.");
    } else {
        for (const Bateau* b : liste) {
            if (y + rh > w.height() - 40) {
                w.newPage(); y = 30;
                // Reprint header on new page
                p.setBrush(QColor("#1e3a8a")); p.setPen(Qt::NoPen);
                p.drawRect(0, y, W, rh);
                p.setPen(Qt::white);
                p.setFont(QFont("Segoe UI", 9, QFont::Bold));
                cx = 0;
                for (int c = 0; c < hd.size(); ++c) {
                    p.drawText(cx + 8, y, cw[c] - 8, rh, Qt::AlignVCenter, hd[c]);
                    cx += cw[c];
                }
                y += rh;
                p.setFont(QFont("Segoe UI", 9));
            }

            // Row background
            p.setBrush(alt ? QColor("#eff6ff") : Qt::white);
            p.setPen(Qt::NoPen);
            p.drawRect(0, y, W, rh);

            // Cell text
            p.setPen(QColor("#1f2937"));
            QStringList v = {
                b->nomBateau,
                b->immatriculation,
                b->capacitePeche + " t",
                b->longueur + " m",
                b->proprietaire,
                b->ageBateau + " ans",
                b->dateDerniereMaintenance,
                b->disponible
            };
            cx = 0;
            for (int c = 0; c < v.size(); ++c) {
                p.drawText(cx + 8, y, cw[c] - 8, rh, Qt::AlignVCenter, v[c]);
                cx += cw[c];
            }

            // Row separator
            p.setPen(QPen(QColor("#bfdbfe"), 1));
            p.drawLine(0, y + rh, W, y + rh);

            y += rh;
            alt = !alt;
        }

        // ── Footer summary ───────────────────────────────────
        y += 14;
        p.setBrush(QColor("#dbeafe")); p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, y, W, 32, 6, 6);
        p.setPen(QColor("#1e40af"));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(8, y, W - 16, 32, Qt::AlignVCenter,
                   QString("Total bateaux en maintenance : %1").arg(liste.size()));
    }

    p.end();
    QMessageBox::information(this, "PDF généré",
        "Rapport enregistré avec succès :\n" + path);
}


void BateauWindow::onShowStatistics()
{
    QMap<QString,int> st,dp;
    st["En mer"]=0;st["Au port"]=0;st["En maintenance"]=0;
    dp["Oui"]=0;dp["Non"]=0;
    for(const Bateau&b:bateaux){
        if(st.contains(b.etatBateau))st[b.etatBateau]++;else st[b.etatBateau]++;
        if(dp.contains(b.disponible))dp[b.disponible]++;else dp[b.disponible]++;
    }
    StatisticsDialog dlg(st,dp,this); dlg.exec();
}

void BateauWindow::onAddBateau()
{
    BateauDialog d(this);
    if(d.exec()==QDialog::Accepted){
        Bateau nb=d.getData(); nb.idBateau=generateBateauId();
        bateaux.append(nb); populateTable(searchInput->text());
    }
}
void BateauWindow::onEditBateau(int row)
{
    if(row<0||row>=bateaux.size()) return;
    BateauDialog d(this,&bateaux[row]);
    if(d.exec()==QDialog::Accepted){
        Bateau u=d.getData(); u.idBateau=bateaux[row].idBateau;
        bateaux[row]=u; populateTable(searchInput->text());
    }
}
void BateauWindow::onDeleteBateau(int row)
{
    if(row<0||row>=bateaux.size()) return;
    if(QMessageBox::question(this,"Confirmation","Supprimer '"+bateaux[row].nomBateau+"' ?",
        QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
        bateaux.removeAt(row); populateTable(searchInput->text());
    }
}
void BateauWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}
