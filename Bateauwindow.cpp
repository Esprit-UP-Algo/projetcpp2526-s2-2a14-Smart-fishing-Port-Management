#include "bateauwindow.h"
#include "bateaudialog.h"
#include "BateauStatisticsDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QSqlRecord>
#include <QPixmap>
#include <QComboBox>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QScrollArea>
#include <QSizePolicy>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <algorithm>

BateauWindow::BateauWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
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
        logoLbl->setStyleSheet("font-size:42px;background:transparent;");
    }
    logoLbl->setAlignment(Qt::AlignCenter);
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
    navLay->addWidget(createNavButton("📦", "Pêche"));
    navLay->addWidget(createNavButton("👥", "Employés"));
    navLay->addWidget(createNavButton("🧊", "Frigos"));
    navLay->addWidget(createNavButton("⚙️", "Paramètres"));
    navLay->addStretch();
    navLay->addWidget(createNavButton("🚪", "Quitter", false, true));

    lay->addWidget(nav, 1);
    return sidebar;
}

QPushButton* BateauWindow::createNavButton(const QString& icon, const QString& text, bool isActive, bool isLogout)
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
    QLabel* sub = new QLabel("Suivi de la flotte et des maintenances");
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

    /* Search bar */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher par nom ou immatriculation...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &BateauWindow::onSearch);
    lay->addWidget(searchInput);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Nom (A-Z)", "Nom (Z-A)", "Capacité ↑", "Capacité ↓", "Âge ↑", "Âge ↓", "Maintenance ↑", "Maintenance ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BateauWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

// ─── Table card ────────────────────────────────────────────
QFrame* BateauWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet(R"(
        QFrame {
            background-color: #e2e8f0;
            border-radius: 20px;
        }
    )");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(3, 3, 3, 3);

    // White container for the table
    QFrame* whiteContainer = new QFrame();
    whiteContainer->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 17px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    table = new QTableWidget(); // Fixed: Initialize before setup
    setupTable();
    containerLayout->addWidget(table);

    layout->addWidget(whiteContainer);

    return card;
}

void BateauWindow::setupTable()
{
    table->setColumnCount(11);
    table->setHorizontalHeaderLabels({"ID", "Nom", "Immat.", "Capacité", "Longueur", "Age", "Maintenance", "Dispo", "Employé", "Quai", "Actions"});
    
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    
    // Column configuration
    table->setColumnHidden(0, true); // Hide ID column as requested
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Nom stretches
    table->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Fixed);
    table->setColumnWidth(10, 110);

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
}

// ═══════════════════════════════════════════════════════════
//  Populate
// ═══════════════════════════════════════════════════════════
void BateauWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    Bateau b;
    QSqlQueryModel* model;
    
    if (!filterText.isEmpty()) {
        model = b.rechercher(filterText);
    } else {
        int si = sortCombo ? sortCombo->currentIndex() : 0;
        if (si == 0) model = b.afficher();
        else {
            QString critere = "IDBATEAU", ordre = "ASC";
            if (si == 1) { critere = "NOMBATEAU"; ordre = "ASC"; }
            else if (si == 2) { critere = "NOMBATEAU"; ordre = "DESC"; }
            else if (si == 3) { critere = "CAPACITE"; ordre = "ASC"; }
            else if (si == 4) { critere = "CAPACITE"; ordre = "DESC"; }
            else if (si == 5) { critere = "AGE_BATEAU"; ordre = "ASC"; }
            else if (si == 6) { critere = "AGE_BATEAU"; ordre = "DESC"; }
            else if (si == 7) { critere = "DATE_DERNIERE_MAINTENANCE"; ordre = "ASC"; }
            else if (si == 8) { critere = "DATE_DERNIERE_MAINTENANCE"; ordre = "DESC"; }
            model = b.trier(critere, ordre);
        }
    }

    for(int i = 0; i < model->rowCount(); ++i) {
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 60);

        for(int j = 0; j < 10; ++j) {
            QString val = model->data(model->index(i, j)).toString();
            
            // Add units to specific numeric columns
            if (j == 3 && !val.isEmpty()) val += " T";      // Capacité
            if (j == 4 && !val.isEmpty()) val += " m";      // Longueur
            if (j == 5 && !val.isEmpty()) val += " ans";    // Age

            QTableWidgetItem* item = new QTableWidgetItem(val);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFont(cellFont);
            table->setItem(r, j, item);
        }

        // Apply badges for specific columns
        QString dispoStatus = model->data(model->index(i, 7)).toString();
        table->setCellWidget(r, 7, createDisponibleBadge(dispoStatus));

        table->setCellWidget(r, 10, createActionButtons(r));
    }
    delete model;
}

// ═══════════════════════════════════════════════════════════
//  Badges & action buttons
// ═══════════════════════════════════════════════════════════
QWidget* BateauWindow::createStatusBadge(const QString& etat)
{
    // This function is no longer used as 'Maintenance' column now displays date directly.
    // Keeping it for potential future use or if the requirement changes.
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
    
    QLabel* b = new QLabel(dispo == "Oui" ? "✔  Disponible" : "✘  Occupé");
    b->setFont(QFont("Segoe UI", 9, QFont::Medium));
    b->setFixedHeight(28); b->setAlignment(Qt::AlignCenter);
    
    if (dispo == "Oui")
        b->setStyleSheet("QLabel{background:#dcfce7; color:#166534; border-radius:7px; padding:2px 14px;}");
    else
        b->setStyleSheet("QLabel{background:#fee2e2; color:#991b1b; border-radius:7px; padding:2px 14px;}");
        
    l->addWidget(b); return w;
}

QWidget* BateauWindow::createActionButtons(int row)
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
    connect(e,&QPushButton::clicked,[this,row](){ onEditBateau(row); });
    connect(d,&QPushButton::clicked,[this,row](){ onDeleteBateau(row); });
    l->addWidget(e); l->addWidget(d);
    return w;
}

// ═══════════════════════════════════════════════════════════
//  Slots
// ═══════════════════════════════════════════════════════════
QString BateauWindow::generateBateauId() {
    return "B" + QString::number(QDateTime::currentMSecsSinceEpoch()).right(4);
}

void BateauWindow::onSearch(const QString& t){ populateTable(t); }
void BateauWindow::onSort(int index) {
    Q_UNUSED(index);
    populateTable(searchInput->text());
}

void BateauWindow::onGeneratePDF()
{
    QString path = QFileDialog::getSaveFileName(this, "Exporter PDF", "bateaux.pdf", "PDF (*.pdf)");
    if(path.isEmpty()) return;

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    QPainter painter(&writer);
    painter.drawText(100, 100, "Rapport des Bateaux - " + QDateTime::currentDateTime().toString());
    
    int y = 500;
    Bateau b;
    QSqlQueryModel* model = b.afficher();
    for(int i=0; i<model->rowCount(); ++i) {
        QString line = model->data(model->index(i,0)).toString() + " | " + model->data(model->index(i,1)).toString();
        painter.drawText(100, y, line);
        y += 200;
        if (y > 8000) { writer.newPage(); y = 500; }
    }
    delete model;
    QMessageBox::information(this, "PDF", "PDF généré avec succès.");
}

void BateauWindow::onShowStatistics()
{
    Bateau model;
    QSqlQueryModel* data = model.afficher();
    
    QMap<QString, double> availabilityData;
    int totalBateaux = data->rowCount();
    double totalCapacite = 0;
    double totalAge = 0;
    
    for(int i = 0; i < totalBateaux; ++i) {
        QString dispo = data->record(i).value("DISPONIBLE").toString();
        double cap = data->record(i).value("CAPACITE").toDouble();
        double age = data->record(i).value("AGE_BATEAU").toDouble();
        
        QString label = (dispo == "Oui") ? "Disponible" : "Occupé";
        availabilityData[label]++;
        
        totalCapacite += cap;
        totalAge += age;
    }
    
    double avgAge = (totalBateaux > 0) ? (totalAge / totalBateaux) : 0;
    delete data;
    
    BateauStatisticsDialog dlg(availabilityData, totalBateaux, totalCapacite, avgAge, this);
    dlg.exec();
}

void BateauWindow::onAddBateau()
{
    BateauDialog diag(this);
    if (diag.exec() == QDialog::Accepted) {
        Bateau b = diag.getData();
        b.setIdBateau(generateBateauId());
        if (b.ajouter()) {
            QMessageBox::information(this, "OK", "Ajout effectué\nClick Cancel to exit.", QMessageBox::Cancel);
            populateTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
        }
    }
}
void BateauWindow::onEditBateau(int row)
{
    Bateau b;
    b.setIdBateau(table->item(row, 0)->text());
    b.setNomBateau(table->item(row, 1)->text());
    b.setImmatriculation(table->item(row, 2)->text());
    b.setCapacite(table->item(row, 3)->text());
    b.setLongueur(table->item(row, 4)->text());
    b.setAgeBateau(table->item(row, 5)->text());
    b.setDateMaintenance(table->item(row, 6)->text());
    b.setDisponible(table->item(row, 7)->text());
    b.setIdEmploye(table->item(row, 8)->text());
    b.setIdQuai(table->item(row, 9)->text());

    BateauDialog diag(this, &b);
    if (diag.exec() == QDialog::Accepted) {
        Bateau newB = diag.getData();
        if (newB.modifier(b.getIdBateau())) {
            QMessageBox::information(this, "OK", "Modification effectuée\nClick Cancel to exit.", QMessageBox::Cancel);
            populateTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
        }
    }
}
void BateauWindow::onDeleteBateau(int row)
{
    QString id = table->item(row, 0)->text();
    if (QMessageBox::question(this, "Suppression", "Supprimer le bateau " + id + " ?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        Bateau b;
        if (b.supprimer(id)) {
            QMessageBox::information(this, "OK", "Suppression effectuée\nClick Cancel to exit.", QMessageBox::Cancel);
            populateTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
        }
    }
}
void BateauWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}
