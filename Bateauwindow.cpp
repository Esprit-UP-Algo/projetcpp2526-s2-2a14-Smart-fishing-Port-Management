#include "bateauwindow.h"
#include "quaiswindow.h"
#include "bateaudialog.h"
#include "BateauStatisticsDialog.h"
#include "PredictMaintenanceDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlRecord>
#include <QPixmap>
#include <QComboBox>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDate>
#include <QBrush>
#include <QColor>
#include <QInputDialog>
#include <QSqlQuery>
#include <QScrollArea>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <algorithm>
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

QList<BateauWindow*> BateauWindow::s_instances;

BateauWindow::BateauWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Make sure column exists
    QSqlQuery alter;
    alter.exec("ALTER TABLE BATEAUX ADD DATE_PROCHAINE_MAINTENANCE DATE");
    
    s_instances.append(this);
    setupUi();
    populateTable();
}

BateauWindow::~BateauWindow()
{
    s_instances.removeAll(this);
}

void BateauWindow::refreshAllTables()
{
    for (BateauWindow* win : s_instances) {
        win->populateTable();
    }
}


void BateauWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Bateaux");
    setMinimumSize(1300, 650);
    setStyleSheet("QMainWindow { background-color: #F0F4F8; }");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(createSidebar());
    root->addWidget(createContentArea(), 1);
}

QFrame* BateauWindow::createSidebar()
{
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1e4fa3, stop:1 #4a87e0); }");
    QVBoxLayout* lay = new QVBoxLayout(sidebar);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

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
        btn->setStyleSheet("QPushButton{background:rgba(255,255,255,0.08);color:white;border:none;border-radius:10px;text-align:left;padding-left:16px;} QPushButton:hover{background:rgba(239,68,68,0.75);}");
        connect(btn, &QPushButton::clicked, this, &BateauWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet("QPushButton{background:rgba(255,255,255,0.22);color:white;border:none;border-radius:10px;text-align:left;padding-left:16px;}");
    } else {
        btn->setStyleSheet("QPushButton{background:transparent;color:rgba(255,255,255,0.88);border:none;border-radius:10px;text-align:left;padding-left:16px;} QPushButton:hover{background:rgba(255,255,255,0.12);}");
    }
    return btn;
}

QWidget* BateauWindow::createContentArea()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:#F0F4F8;border:none;} QScrollBar:vertical{width:6px;background:transparent;} QScrollBar::handle:vertical{background:#c1c9d6;border-radius:3px;}");
    QWidget* inner = new QWidget();
    inner->setStyleSheet("background:#F0F4F8;");
    QVBoxLayout* lay = new QVBoxLayout(inner);
    lay->setSpacing(18); lay->setContentsMargins(28, 24, 28, 24);
    lay->addWidget(createHeader());
    lay->addWidget(createToolbar());
    lay->addWidget(createTableCard(), 1);
    scroll->setWidget(inner);
    return scroll;
}

QFrame* BateauWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");
    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0); lay->setSpacing(12);

    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Bateaux");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Suivi de la flotte et des maintenances");
    sub->setFont(QFont("Segoe UI", 10)); sub->setStyleSheet("color:#6b7280;");
    titleCol->addWidget(title); titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor); btn->setFixedHeight(45); btn->setMinimumWidth(160);
        btn->setStyleSheet(QString("QPushButton{background:%1; color:white; border:none; border-radius:12px; padding:0 20px;} QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };
    QPushButton* addBtn   = makeBtn("➕  Nouveau Bateau",  "#2563EB", "#1D4ED8");
    connect(addBtn,   &QPushButton::clicked, this, &BateauWindow::onAddBateau);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* BateauWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet("QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }");
    bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); bar->setMinimumHeight(62);
    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0); lay->setSpacing(12);
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher par nom ou immatriculation...");
    searchInput->setFixedWidth(350); searchInput->setFixedHeight(40);
    searchInput->setStyleSheet("QLineEdit { background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px; padding-left:12px; font-size:13px; color:#334155; } QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }");
    connect(searchInput, &QLineEdit::textChanged, this, &BateauWindow::onSearch);
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
    QPushButton* predBtn  = makeToolBtn("🔮  Prédiction", "#F59E0B", "#D97706");
    QPushButton* pdfBtn   = makeToolBtn("📄  PDF", "#059669", "#047857");

    connect(statsBtn, &QPushButton::clicked, this, &BateauWindow::onShowStatistics);
    connect(predBtn,  &QPushButton::clicked, this, &BateauWindow::onPredictMaintenanceGlobal);
    connect(pdfBtn,   &QPushButton::clicked, this, &BateauWindow::onGeneratePDF);

    lay->addWidget(statsBtn);
    lay->addWidget(predBtn);
    lay->addWidget(pdfBtn);

    lay->addStretch();
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);
    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200); sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Nom (A-Z)", "Nom (Z-A)", "Capacité ↑", "Capacité ↓", "Âge ↑", "Âge ↓", "Maintenance ↑", "Maintenance ↓"});
    sortCombo->setStyleSheet("QComboBox { background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px; padding:0 12px; color:#334155; } QComboBox::drop-down { border:none; } QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BateauWindow::onSort);
    lay->addWidget(sortCombo);
    return bar;
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
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(3, 3, 3, 3);

    QFrame* whiteContainer = new QFrame();
    whiteContainer->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 17px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);
    table = new QTableWidget();
    setupTable();
    containerLayout->addWidget(table);
    layout->addWidget(whiteContainer);
    return card;
}

void BateauWindow::setupTable()
{
    table->setColumnCount(15);
    table->setHorizontalHeaderLabels({"ID", "Nom Bateau", "Immatriculation", "Capacité (T)", "Longueur (m)", "Âge (ans)", "Dernière Maintenance", "Prochaine Maint. (Est.)", "Employé", "Quai", "État du Bateau", "Code Secret", "Actions", "ID_EMP", "ID_QUAI"});
    table->setColumnHidden(0, true);
    table->setColumnHidden(13, true);
    table->setColumnHidden(14, true);

    table->horizontalHeader()->setStretchLastSection(true);

    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
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

    // Define fixed column widths for a more professional look
    table->setColumnWidth(1, 180); // Nom Bateau
    table->setColumnWidth(2, 180); // Immatriculation
    table->setColumnWidth(3, 110); // Capacité
    table->setColumnWidth(4, 110); // Longueur
    table->setColumnWidth(5, 90);  // Âge
    table->setColumnWidth(6, 180); // Dernière Maint
    table->setColumnWidth(7, 200); // Prochaine Maint
    table->setColumnWidth(8, 140); // Employé
    table->setColumnWidth(9, 110); // Quai
    table->setColumnWidth(10, 140); // État
    table->setColumnWidth(11, 110); // Code
    table->setColumnWidth(12, 110); // Actions
}

void BateauWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);
    Bateau b;
    QSqlQueryModel* model;
    if (!filterText.isEmpty()) model = b.rechercher(filterText);
    else {
        int si = sortCombo ? sortCombo->currentIndex() : 0;
        if (si == 0) model = b.afficher();
        else {
            QString critere = "IDBATEAU", ordre = "ASC";
            if (si == 1)      { critere = "NOMBATEAU"; ordre = "ASC"; }
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
        table->insertRow(r); table->setRowHeight(r, 65);
        for(int j = 0; j < 7; ++j) {
            QString val = model->data(model->index(i, j)).toString();
            if (j == 3 && !val.isEmpty()) val += " T";
            if (j == 4 && !val.isEmpty()) val += " m";
            if (j == 5 && !val.isEmpty()) val += " ans";
            QTableWidgetItem* item = new QTableWidgetItem(val);
            
            // Align Left for Name (1) and Immatriculation (2), Center for others
            if (j == 1 || j == 2) {
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            } else {
                item->setTextAlignment(Qt::AlignCenter);
            }
            
            item->setFont(cellFont);
            table->setItem(r, j, item);
        }

        // Vérifier si une prédiction existe dans la map pour cette immatriculation
        QString immat = model->data(model->index(i, 2)).toString();
        QTableWidgetItem* nextMaintItem = nullptr;
        
        QString nextMaintStr = model->data(model->index(i, 12)).toString();
        if (!nextMaintStr.isEmpty()) {
            nextMaintItem = new QTableWidgetItem(nextMaintStr);
            QDate nextMaintDate = QDate::fromString(nextMaintStr, "dd/MM/yyyy");
            
            // Calcul du niveau d'urgence pour la couleur
            qint64 daysTo = QDate::currentDate().daysTo(nextMaintDate);
            if (daysTo <= 0) {
                nextMaintItem->setForeground(QBrush(QColor("#dc2626"))); // Rouge
            } else if (daysTo <= 30) {
                nextMaintItem->setForeground(QBrush(QColor("#ea580c"))); // Orange
            } else {
                nextMaintItem->setForeground(QBrush(QColor("#16a34a"))); // Vert
            }
        } else {
            nextMaintItem = new QTableWidgetItem("-");
            nextMaintItem->setForeground(QBrush(QColor("#64748b"))); // Gris foncé par défaut
        }
        
        nextMaintItem->setTextAlignment(Qt::AlignCenter); 
        nextMaintItem->setFont(cellFont);
        table->setItem(r, 7, nextMaintItem);

        // Employé & Quai
        for(int j = 7; j <= 8; ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(model->data(model->index(i, j)).toString());
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter); 
            item->setFont(cellFont);
            table->setItem(r, j + 1, item); // Décalé de +1
        }

        // Column 10: État du Bateau
        QString etat = model->data(model->index(i, 11)).toString();
        table->setCellWidget(r, 10, createDisponibleBadge(etat));

        // Column 11: Code Secret
        QString sec = model->data(model->index(i, 13)).toString(); // Index 13 in model is b.CODE_SECRET (fetched in bateau.cpp)
        QTableWidgetItem* secItem = new QTableWidgetItem(sec);
        secItem->setTextAlignment(Qt::AlignCenter); secItem->setFont(cellFont);
        table->setItem(r, 11, secItem);

        // Action buttons
        table->setCellWidget(r, 12, createActionButtons(r));
        // Hidden IDs
        table->setItem(r, 13, new QTableWidgetItem(model->data(model->index(i, 9)).toString()));
        table->setItem(r, 14, new QTableWidgetItem(model->data(model->index(i, 10)).toString()));
    }
    delete model;
}

QWidget* BateauWindow::createDisponibleBadge(const QString& status)
{
    QWidget* w = new QWidget(); QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(4,0,4,0); l->setAlignment(Qt::AlignCenter);
    QString style;
    if (status == "En mer") {
        style = "QLabel{background:#dbeafe;color:#1e40af;border-radius:7px;padding:2px 10px;}";
    } else if (status == "En maintenance") {
        style = "QLabel{background:#ffedd5;color:#9a3412;border-radius:7px;padding:2px 10px;}";
    } else { // Au port or default
        style = "QLabel{background:#dcfce7;color:#166534;border-radius:7px;padding:2px 10px;}";
    }
    QString displayName = status.isEmpty() ? "Au port" : status;
    QLabel* b = new QLabel(displayName);
    b->setFont(QFont("Segoe UI",9,QFont::Medium));
    b->setFixedHeight(28); b->setAlignment(Qt::AlignCenter);
    b->setStyleSheet(style);
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
    
    e->setToolTip("Modifier");
    d->setToolTip("Supprimer");

    connect(e,&QPushButton::clicked,[this,row](){ onEditBateau(row); });
    connect(d,&QPushButton::clicked,[this,row](){ onDeleteBateau(row); });
    l->addWidget(e); l->addWidget(d);
    return w;
}

QString BateauWindow::generateBateauId() {
    // FIXED: Return a purely numeric string to resolve ORA-01722 if IDBATEAU is NUMBER
    return QString::number(QDateTime::currentMSecsSinceEpoch()).right(5);
}

void BateauWindow::onSearch(const QString& t){ populateTable(t); }
void BateauWindow::onSort(int index) { Q_UNUSED(index); populateTable(searchInput->text()); }

void BateauWindow::onGeneratePDF()
{
    Bateau b;
    QSqlQueryModel* model = b.afficher();
    
    QList<QStringList> mBoats;
    for (int i=0; i<model->rowCount(); ++i) {
        QString etat = model->data(model->index(i, 11)).toString();
        if (etat == "En maintenance") {
            QStringList boat;
            boat << model->data(model->index(i, 1)).toString(); // Nom
            boat << model->data(model->index(i, 2)).toString(); // Immat
            boat << model->data(model->index(i, 3)).toString(); // Cap
            boat << etat; // Etat
            boat << model->data(model->index(i, 6)).toString(); // Date
            mBoats.append(boat);
        }
    }
    delete model;

    if (mBoats.isEmpty()) {
        QMessageBox::information(this, "Export", "Aucun bateau en maintenance.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Remarques avant Exportation");
    dialog.resize(900, 400);
    QVBoxLayout layout(&dialog);
    
    QLabel label("Ajoutez vos remarques ou observations pour chaque bateau en maintenance :");
    layout.addWidget(&label);
    
    QTableWidget table(mBoats.size(), 6, &dialog);
    table.setHorizontalHeaderLabels(QStringList() << "Nom" << "Immatriculation" << "Capacité" << "État" << "Date" << "Vos Remarques (Éditable)");
    table.horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table.horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    
    for (int i=0; i<mBoats.size(); ++i) {
        for (int j=0; j<5; ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(mBoats[i][j]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            table.setItem(i, j, item);
        }
        table.setItem(i, 5, new QTableWidgetItem(""));
    }
    layout.addWidget(&table);
    
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout.addWidget(&buttons);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    // Récupérer les remarques écrites par l'utilisateur
    for (int i=0; i<mBoats.size(); ++i) {
        if (table.item(i, 5)) {
            mBoats[i].append(table.item(i, 5)->text());
        } else {
            mBoats[i].append("");
        }
    }
    
    QString path = QFileDialog::getSaveFileName(this, "Exporter PDF Direction", "Rapport_Direction_Maintenance.pdf", "PDF (*.pdf)");
    if(path.isEmpty()) return;

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    QPainter painter(&writer);
    
    int wPage = writer.width();
    int hPage = writer.height();
    int margin = (wPage * 5) / 100; // marge 5%  (division entière simple)
    int w = wPage - 2 * margin;
    int y = margin;
    
    QFont titleFont("Arial", 22, QFont::Bold);
    QFont subTitleFont("Arial", 14, QFont::Normal);
    QFont headerFont("Arial", 10, QFont::Bold);
    QFont bodyFont("Arial", 9);
    
    QColor mainBlue("#1E3A8A"); 
    QColor lightBlue("#F0F4F8");
    
    // Titre et Sous-titre
    painter.setPen(mainBlue);
    painter.setFont(titleFont);
    painter.drawText(margin, y, "SUIVI OFFICIEL DES BATEAUX EN MAINTENANCE");
    y += 150;
    
    painter.setPen(Qt::darkGray);
    painter.setFont(subTitleFont);
    painter.drawText(margin, y, "Document destiné à la Direction Générale du Port");
    y += 150;
    
    painter.drawText(margin, y, "Date d'édition : " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    y += 300;
    
    // En-tête du tableau (Fond bleu)
    painter.fillRect(margin, y - 80, w, 130, mainBlue); 
    painter.setPen(Qt::white);
    painter.setFont(headerFont);
    
    int c1 = margin + (w * 1) / 100;
    int c2 = margin + (w * 15) / 100;
    int c3 = margin + (w * 32) / 100;
    int c4 = margin + (w * 44) / 100;
    int c5 = margin + (w * 58) / 100;
    int c6 = margin + (w * 72) / 100;
    int rW = (w * 28) / 100; // largeur de la remarque augmentée (28%)
    
    painter.drawText(c1, y, "NOM");
    painter.drawText(c2, y, "IMMATRICULATION");
    painter.drawText(c3, y, "CAPACITÉ");
    painter.drawText(c4, y, "ÉTAT");
    painter.drawText(c5, y, "DATE MAINT.");
    painter.drawText(c6, y, "OBSERVATIONS / REMARQUES");
    y += 100;
    
    // Contenu du tableau
    painter.setFont(bodyFont);
    for (int i=0; i<mBoats.size(); ++i) {
        if (y > hPage - margin - 500) { // Nouvelle page si besoin
            writer.newPage();
            y = margin + 100;
            // Répéter l'en-tête
            painter.fillRect(margin, y - 80, w, 130, mainBlue); 
            painter.setPen(Qt::white); painter.setFont(headerFont);
            painter.drawText(c1, y, "NOM"); painter.drawText(c2, y, "IMMATRICULATION"); painter.drawText(c3, y, "CAPACITÉ");
            painter.drawText(c4, y, "ÉTAT"); painter.drawText(c5, y, "DATE MAINT."); painter.drawText(c6, y, "OBSERVATIONS / REMARQUES");
            y += 100; painter.setFont(bodyFont);
        }
        
        // Zébrure (Ligne sur deux gris clair)
        if (i % 2 == 0) {
            painter.fillRect(margin, y - 50, w, 180, lightBlue);
        }
        
        painter.setPen(Qt::black);
        painter.drawText(c1, y + 50, mBoats[i][0]); // Nom
        painter.drawText(c2, y + 50, mBoats[i][1]); // Immatriculation
        painter.drawText(c3, y + 50, mBoats[i][2].isEmpty() ? "-" : mBoats[i][2] + " T"); // Capacité
        painter.drawText(c4, y + 50, mBoats[i][3]); // Etat
        painter.drawText(c5, y + 50, mBoats[i][4]); // Date
        
        // Remarque sur plusieurs lignes (WordWrap)
        QString remark = mBoats[i][5].isEmpty() ? "-" : mBoats[i][5];
        QRect remRect(c6, y - 20, rW - 20, 160); // Cadre confiné
        painter.drawText(remRect, Qt::AlignLeft | Qt::TextWordWrap, remark);
        
        y += 180; // Interligne augmenté
    }
    
    painter.setPen(QPen(mainBlue, 3));
    painter.drawLine(margin, y - 30, wPage - margin, y - 30);
    y += 150;
    
    // Signature de la direction générale
    if (y > hPage - margin - 400) { writer.newPage(); y = margin + 100; }
    
    painter.setFont(subTitleFont);
    painter.setPen(Qt::black);
    int sigW = (w * 40) / 100;
    int sigX = wPage - margin - sigW;
    
    painter.drawText(sigX, y, "Signature du Responsable :");
    painter.setPen(QPen(Qt::gray, 2, Qt::DashLine));
    painter.drawRect(sigX, y + 50, sigW, 300);
    
    QMessageBox::information(this, "Succès", "Rapport PDF Officiel pour la Direction Générale généré avec succès !");
}

void BateauWindow::onShowStatistics()
{
    Bateau model; QSqlQueryModel* data = model.afficher();
    QMap<QString, double> availabilityData;
    int total = data->rowCount(); double totalCap = 0; double totalAge = 0;
    for(int i = 0; i < total; ++i) {
        QString dispo = data->record(i).value("ETAT").toString();
        if (dispo.isEmpty()) dispo = "Au port";
        availabilityData[dispo]++;
        totalCap += data->record(i).value("CAPACITE").toDouble();
        totalAge += data->record(i).value("AGE_BATEAU").toDouble();
    }
    double avgAge = (total > 0) ? (totalAge / total) : 0;
    delete data; BateauStatisticsDialog dlg(availabilityData, total, totalCap, avgAge, this); dlg.exec();
}

void BateauWindow::onAddBateau()
{
    BateauDialog diag(this);
    QList<QPair<QString, QString>> employees;
    { QSqlQuery query; if (!query.exec("SELECT ID_EMPLOYE, NOM || ' ' || PRENOM FROM EMPLOYEES")) query.exec("SELECT ID_EMPLOYE, NOM FROM EMPLOYEE");
        while (query.next()) employees.append({query.value(0).toString(), query.value(1).toString()});
    } diag.setEmployeeList(employees);
    QList<QPair<QString, QString>> quais;
    { 
        QSqlQuery query; 
        if (!query.exec("SELECT IDQUAI, NUMERO, LOCATION FROM QUAIS WHERE ETAT = 'Disponible'")) 
            query.exec("SELECT IDQUAI, NUMERO, LOCATION FROM QUAI WHERE ETAT = 'Disponible'");
        while (query.next()) quais.append({query.value(0).toString(), QString("Quai %1 (%2)").arg(query.value(1).toString(), query.value(2).toString())});
    } diag.setQuaiList(quais);

    if (diag.exec() == QDialog::Accepted) {
        Bateau b = diag.getData(); b.setIdBateau(generateBateauId());
        if (b.ajouter()) { 
            QMessageBox::information(this, "OK", "Ajout effectué."); 
            populateTable(); 
            QuaisWindow::refreshAll(); 
        }
        else QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
    }
}

void BateauWindow::onEditBateau(int row)
{
    Bateau b;
    b.setIdBateau(table->item(row, 0)->text());
    b.setNomBateau(table->item(row, 1)->text());
    b.setImmatriculation(table->item(row, 2)->text());
    b.setCapacite(table->item(row, 3)->text().split(" ").first());
    b.setLongueur(table->item(row, 4)->text().split(" ").first());
    b.setAgeBateau(table->item(row, 5)->text().split(" ").first());
    b.setDateMaintenance(table->item(row, 6)->text());
    b.setIdEmploye(table->item(row, 13)->text());
    b.setIdQuai(table->item(row, 14)->text());
    b.setCodeSecret(table->item(row, 11)->text().toInt());
    
    // Status from badge text or model? Easier to get from Model when populating.
    // Or just check radio button logic in Dialog correctly.
    // We already have b.setDisponible logic if we get it from table cell if it was stored.
    // Let's store raw dispo in another hidden column or just let Dialog handle it?
    // Actually, populateFields() in Dialog needs it.
    
    // For now, let's assume it's retrieved from the badge label
    QWidget* badge = table->cellWidget(row, 10);
    if(badge) {
        QLabel* l = badge->findChild<QLabel*>();
        if(l) {
            QString t = l->text();
            b.setEtat(t);
        }
    }

    BateauDialog diag(this, &b);
    QList<QPair<QString, QString>> employees;
    { QSqlQuery query; if (!query.exec("SELECT ID_EMPLOYE, NOM || ' ' || PRENOM FROM EMPLOYEES")) query.exec("SELECT ID_EMPLOYE, NOM FROM EMPLOYEE");
        while (query.next()) employees.append({query.value(0).toString(), query.value(1).toString()});
    } diag.setEmployeeList(employees);
    QList<QPair<QString, QString>> quais;
    { 
        QSqlQuery query; 
        query.prepare("SELECT IDQUAI, NUMERO, LOCATION FROM QUAIS WHERE ETAT = 'Disponible' OR IDQUAI = :idq");
        query.bindValue(":idq", b.getIdQuai());
        if (!query.exec()) {
            query.prepare("SELECT IDQUAI, NUMERO, LOCATION FROM QUAI WHERE ETAT = 'Disponible' OR IDQUAI = :idq");
            query.bindValue(":idq", b.getIdQuai());
            query.exec();
        }
        while (query.next()) quais.append({query.value(0).toString(), QString("Quai %1 (%2)").arg(query.value(1).toString(), query.value(2).toString())});
    } diag.setQuaiList(quais);

    if (diag.exec() == QDialog::Accepted) {
        Bateau nb = diag.getData();
        if (nb.modifier(b.getIdBateau())) { 
            QMessageBox::information(this, "OK", "Modification effectuée."); 
            populateTable();
            QuaisWindow::refreshAll(); // <--- AJOUTÉ
        }
        else QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
    }
}

void BateauWindow::onDeleteBateau(int row)
{
    QString id = table->item(row, 0)->text();
    QString nom = table->item(row, 1)->text();
    
    if (showStyledActionDialog(this, "Suppression Bateau", 
                               QString("Êtes-vous sûr de vouloir supprimer le bateau <b>%1</b> ?<br><br>Cette action est irréversible.").arg(nom),
                               "#EF4444", "#B91C1C", "🗑️")) {
        Bateau b; 
        if (b.supprimer(id)) { 
            showStyledActionDialog(this, "Succès", "Le bateau a été supprimé avec succès.", "#10B981", "#059669", "✅", false);
            populateTable(); 
            QuaisWindow::refreshAll(); 
        }
        else QMessageBox::critical(this, "Erreur", "Echec : " + Bateau::getLastError());
    }
}

void BateauWindow::onLogout()
{
    if (showStyledActionDialog(this, "Quitter", "Voulez-vous vraiment fermer la gestion des bateaux ?", "#1E3A5F", "#3B82F6", "🚪", true, "Oui, Quitter", "Annuler")) {
        this->close();
    }
}



void BateauWindow::onPredictMaintenanceGlobal()
{
    bool ok;
    QString immat = QInputDialog::getText(this, "Prédiction Maintenance", "Entrez l'immatriculation du bateau :", QLineEdit::Normal, "", &ok);
    if (!ok || immat.trimmed().isEmpty()) return;

    QSqlQuery query;
    query.prepare("SELECT AGE_BATEAU, TO_CHAR(DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY'), ETAT, IDBATEAU, COALESCE(FREQUENCE_SORTIES, 0) FROM BATEAUX WHERE IMMATRICULATION = :immat");
    query.bindValue(":immat", immat.trimmed());

    if (query.exec() && query.next()) {
        int age = query.value(0).toInt();
        QString dateStr = query.value(1).toString();
        QString etat = query.value(2).toString();
        QString idBateau = query.value(3).toString();
        int frequence = query.value(4).toInt();

        QDate maintDate = QDate::fromString(dateStr, "dd/MM/yyyy");
        int moisMaintenance = 0;
        if (maintDate.isValid()) {
            int yearDiff = QDate::currentDate().year() - maintDate.year();
            int monthDiff = QDate::currentDate().month() - maintDate.month();
            moisMaintenance = yearDiff * 12 + monthDiff;
            if (moisMaintenance < 0) moisMaintenance = 0;
        }

        if (etat.isEmpty()) {
            etat = "Au port";
        }

        PredictMaintenanceDialog diag(age, moisMaintenance, frequence, etat, this);
        if (diag.exec() == QDialog::Accepted) {
            QDate chosenDate = diag.getSelectedDate();
            QString nextDateStr = chosenDate.toString("dd/MM/yyyy");
            
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE BATEAUX SET DATE_PROCHAINE_MAINTENANCE = TO_DATE(:ndate, 'DD/MM/YYYY') WHERE IMMATRICULATION = :immat");
            updateQuery.bindValue(":ndate", nextDateStr);
            updateQuery.bindValue(":immat", immat.trimmed());
            updateQuery.exec();
            
            populateTable(searchInput->text()); // Mettre à jour le tableau
        }
    } else {
        QMessageBox::warning(this, "Erreur", "Bateau introuvable avec cette immatriculation.");
    }
}
