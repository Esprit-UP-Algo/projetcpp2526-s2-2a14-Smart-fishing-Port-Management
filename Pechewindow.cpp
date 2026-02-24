#include "pechewindow.h"
#include "pechedialog.h"
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
#include <QDateTime>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSet>
#include <algorithm>

PecheWindow::PecheWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    peches.append({"LOT001", "REF-2025-001", "Sardine",  "450", "15/01/2025"});
    peches.append({"LOT002", "REF-2025-002", "Thon",     "280", "16/01/2025"});
    peches.append({"LOT003", "REF-2025-003", "Merlan",   "320", "17/01/2025"});
    peches.append({"LOT004", "REF-2025-004", "Crevette", "150", "18/01/2025"});

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
    navLay->addWidget(createNavButton("⛵", "Bateaux"));
    navLay->addWidget(createNavButton("🐟", "Pêche", true));
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

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* pdfBtn   = makeBtn("📄  Exporter PDF",  "#059669", "#047857");
    QPushButton* addBtn   = makeBtn("➕  Nouveau Lot",     "#2563EB", "#1D4ED8");

    connect(statsBtn, &QPushButton::clicked, this, &PecheWindow::onShowStatistics);
    connect(pdfBtn,   &QPushButton::clicked, this, &PecheWindow::onGeneratePDF);
    connect(addBtn,   &QPushButton::clicked, this, &PecheWindow::onAddPeche);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
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

    /* Search input */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher un lot par catégorie, référence...");
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
    connect(searchInput, &QLineEdit::textChanged, this, &PecheWindow::onSearch);
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
        "Quantité ↑",
        "Quantité ↓",
        "Date ↑ (ancienne)",
        "Date ↓ (récente)",
        "Catégorie ↑ (A→Z)",
        "Catégorie ↓ (Z→A)"
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
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PecheWindow::onSort);
    lay->addWidget(sortCombo, 2);

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
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({
        "Référence", "Catégorie", "Quantité (Kg)", "Date de Capture", "Actions"
    });

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

    QVector<int> idx;
    for (int i = 0; i < peches.size(); ++i) {
        const Peche& p = peches[i];
        if (!filterText.isEmpty()) {
            QString f = filterText.toLower();
            if (!p.reference.toLower().contains(f) &&
                !p.espece.toLower().contains(f) &&
                !p.dateCapture.toLower().contains(f))
                continue;
        }
        idx.append(i);
    }

    int si = sortCombo ? sortCombo->currentIndex() : 0;
    auto parseD = [](const QString& d) { return QDate::fromString(d, "dd/MM/yyyy"); };

    if (si == 1) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return peches[a].quantiteKg.toDouble()<peches[b].quantiteKg.toDouble(); });
    else if (si==2) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return peches[a].quantiteKg.toDouble()>peches[b].quantiteKg.toDouble(); });
    else if (si==3) std::sort(idx.begin(),idx.end(),[this,parseD](int a,int b){ return parseD(peches[a].dateCapture)<parseD(peches[b].dateCapture); });
    else if (si==4) std::sort(idx.begin(),idx.end(),[this,parseD](int a,int b){ return parseD(peches[a].dateCapture)>parseD(peches[b].dateCapture); });
    else if (si==5) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return peches[a].espece < peches[b].espece; });
    else if (si==6) std::sort(idx.begin(),idx.end(),[this](int a,int b){ return peches[a].espece > peches[b].espece; });

    for (int i : idx) {
        const Peche& p = peches[i];
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 58);

        auto it = [&](const QString& t) { QTableWidgetItem* item = new QTableWidgetItem(t); item->setFont(cellFont); return item; };
        table->setItem(r,0,it(p.reference));
        table->item(r,0)->setData(Qt::UserRole, i);
        table->setCellWidget(r, 1, createEspeceBadge(p.espece));
        
        QTableWidgetItem* qItem = it(p.quantiteKg + " Kg");
        qItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        qItem->setForeground(QBrush(QColor("#166534")));
        table->setItem(r, 2, qItem);

        table->setItem(r, 3, it(p.dateCapture));
        table->setCellWidget(r, 4, createActionButtons(i));
    }
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
    int mx=0; for(const Peche&p:peches){ int n=p.idLot.mid(3).toInt(); if(n>mx)mx=n; }
    return QString("LOT%1").arg(mx+1, 3, 10, QChar('0'));
}

void PecheWindow::onSearch(const QString& t) { populateTable(t); }
void PecheWindow::onSort(int) { populateTable(searchInput->text()); }

void PecheWindow::onGeneratePDF()
{
    QString path = QFileDialog::getSaveFileName(this,"Exporter PDF","justificatif_peches.pdf","PDF (*.pdf)");
    if(path.isEmpty()) return;

    QPdfWriter w(path);
    w.setPageSize(QPageSize(QPageSize::A4));
    w.setPageMargins(QMarginsF(15,15,15,15),QPageLayout::Millimeter);
    w.setResolution(96);

    QPainter p(&w);
    p.setRenderHint(QPainter::Antialiasing);
    const int W=w.width();
    int y=40;

    p.setPen(QColor("#1e40af")); p.setFont(QFont("Segoe UI",20,QFont::Bold));
    p.drawText(0,y,W,50,Qt::AlignHCenter,"Justificatif de Pêche"); y+=55;
    p.setPen(QColor("#6b7280")); p.drawText(0,y,W,20,Qt::AlignHCenter,"Généré le "+QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")); y+=35;
    p.setPen(QPen(QColor("#3b82f6"),3)); p.drawLine(0,y,W,y); y+=24;

    /* Stats summary */
    double tot=0; QSet<QString> dts; for(const Peche&cp:peches){ tot+=cp.quantiteKg.toDouble(); dts.insert(cp.dateCapture); }
    p.setPen(QColor("#1e3a5f")); p.setFont(QFont("Segoe UI",12,QFont::Bold));
    p.drawText(0, y, W, 30, Qt::AlignLeft, "  📊  Statistiques Globales :"); y+=35;
    p.setPen(QColor("#1f2937")); p.setFont(QFont("Segoe UI",10));
    p.drawText(20, y, "Total captures : " + QString::number(peches.size()));
    p.drawText(W/2, y, "Poids total : " + QString::number(tot,'f',1) + " Kg"); y+=25;
    p.drawText(20, y, "Moyenne/jour : " + QString::number(tot/qMax(1,dts.size()),'f',1) + " Kg"); y+=30;

    /* Table */
    QStringList hd={"Référence","Catégorie","Quantité","Date"};
    QList<int> cw={W/3, W/4, W/5, W/4};
    int rh=34; p.setBrush(QColor("#1e40af")); p.setPen(Qt::NoPen); p.drawRect(0,y,W,rh);
    p.setPen(Qt::white); p.setFont(QFont("Segoe UI",10,QFont::Bold));
    int cx=0; for(int c=0;c<hd.size();++c){ p.drawText(cx+8,y,cw[c]-8,rh,Qt::AlignVCenter,hd[c]);cx+=cw[c]; }
    y+=rh;

    p.setFont(QFont("Segoe UI",10)); bool alt=false;
    for(const Peche&pe:peches){
        if(y+rh>w.height()-40){w.newPage();y=40;}
        p.setBrush(alt?QColor("#f0f9ff"):Qt::white); p.setPen(Qt::NoPen); p.drawRect(0,y,W,rh);
        p.setPen(QColor("#1f2937"));
        QStringList v={pe.reference, pe.espece, pe.quantiteKg+" Kg", pe.dateCapture};
        cx=0; for(int c=0;c<v.size();++c){p.drawText(cx+8,y,cw[c]-8,rh,Qt::AlignVCenter,v[c]);cx+=cw[c];}
        p.setPen(QPen(QColor("#e5e7eb"),1)); p.drawLine(0,y+rh,W,y+rh); y+=rh; alt=!alt;
    }
    p.end();
    QMessageBox::information(this,"PDF généré","Enregistré : "+path);
}

void PecheWindow::onShowStatistics()
{
    QMap<QString,int> cat,qty;
    for(const Peche&p:peches){
        cat[p.espece]++;
        qty[p.espece] += (int)p.quantiteKg.toDouble();
    }
    StatisticsDialog dlg(cat,qty,this);
    dlg.setWindowTitle("Statistiques des Pêches");
    dlg.exec();
}

void PecheWindow::onAddPeche() {
    PecheDialog d(this);
    if(d.exec()==QDialog::Accepted){ Peche np=d.getData(); np.idLot=generatePecheId(); peches.append(np); populateTable(searchInput->text()); }
}
void PecheWindow::onEditPeche(int row) {
    if(row<0||row>=peches.size()) return;
    PecheDialog d(this,&peches[row]);
    if(d.exec()==QDialog::Accepted){ Peche u=d.getData(); u.idLot=peches[row].idLot; peches[row]=u; populateTable(searchInput->text()); }
}
void PecheWindow::onDeletePeche(int row) {
    if(row<0||row>=peches.size()) return;
    if(QMessageBox::question(this,"Confirmation","Supprimer le lot '"+peches[row].reference+"' ?", QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){ peches.removeAt(row); populateTable(searchInput->text()); }
}
void PecheWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}
