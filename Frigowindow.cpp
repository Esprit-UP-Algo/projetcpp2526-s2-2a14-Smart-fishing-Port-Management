


#include "frigowindow.h"
#include "addfrigodialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <algorithm>
#include <QSqlQuery>
#include <QSqlError>
#include "FrigoStatisticsDialog.h"
#include "arduino.h"
#include "StorageAlert.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextCharFormat>
#include <QBrush>
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

FrigoWindow::FrigoWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
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

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix(":/images/images/logo.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Logo chargé depuis: :/images/images/logo.png";
    } else {
        logoLabel->setText("🧊");
        logoLabel->setStyleSheet(R"(
            QLabel {
                font-size: 50px;
                color: #2C3E50;
            }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Attention: Logo non trouvé à :/images/images/logo.png - utilisation emoji";
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

    // Bottom section: Table (Left) + Calendar (Right)
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(25);

    QFrame* tableCard = createTableCard();
    QFrame* calendarCard = createCalendarCard();

    bottomLayout->addWidget(tableCard, 2);    // Table takes more space
    bottomLayout->addWidget(calendarCard, 1); // Calendar takes less space

    layout->addLayout(bottomLayout, 1);

    loadCalendarData();
    updateCalendarDetails(QDate::currentDate());

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

    QPushButton* addBtn = makeBtn("➕  Nouveau Frigo", "#2563EB", "#1D4ED8");
    connect(addBtn, &QPushButton::clicked, this, &FrigoWindow::onAddFrigo);

    lay->addWidget(addBtn);
    return hdr;
}

QFrame* FrigoWindow::createToolbar()
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
    searchInput->setPlaceholderText("🔍  Rechercher par statut, contenu...");
    searchInput->setFixedWidth(320);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &FrigoWindow::onSearch);
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

    QPushButton* statsBtn  = makeToolBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    QPushButton* classBtn  = makeToolBtn("🌿  Classification", "#10B981", "#059669");
    QPushButton* smsBtn    = makeToolBtn("📱  SMS", "#F59E0B", "#D97706");
    QPushButton* pdfBtn    = makeToolBtn("📄  PDF", "#059669", "#047857");

    connect(statsBtn, &QPushButton::clicked, this, &FrigoWindow::onShowStatistics);
    connect(classBtn, &QPushButton::clicked, this, &FrigoWindow::onShowClassification);
    connect(smsBtn,   &QPushButton::clicked, this, &FrigoWindow::onSendSMS);
    connect(pdfBtn,   &QPushButton::clicked, this, &FrigoWindow::onGeneratePDF);

    lay->addWidget(statsBtn);
    lay->addWidget(classBtn);
    lay->addWidget(smsBtn);
    lay->addWidget(pdfBtn);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Capacité ↑", "Capacité ↓", "Température ↑", "Température ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FrigoWindow::onSort);
    lay->addWidget(sortCombo);

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
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({"Référence", "Capacité", "Poisson", "Statut", "Date Rés.", "Température", "Occupation", "Téléphone", "Actions"});

    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);

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

    table->setColumnWidth(0, 140);   // Référence
    table->setColumnWidth(1, 120);   // Capacité
    table->setColumnWidth(2, 120);   // Poisson
    table->setColumnWidth(3, 120);   // Statut
    table->setColumnWidth(4, 120);   // Date Rés.
    table->setColumnWidth(5, 120);   // Température
    table->setColumnWidth(6, 120);   // Occupation
    table->setColumnWidth(7, 140);   // Téléphone
}

void FrigoWindow::populateTable(const QString& filterText)
{
    static bool sessionAlertShown = false;
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    QSqlQueryModel* model;
    if (!filterText.isEmpty()) model = frigoModel.rechercher(filterText);
    else {
        int si = sortCombo ? sortCombo->currentIndex() : 0;
        if (si == 0) model = frigoModel.afficher();
        else {
            QString crit = "REFERENCE", ord = "ASC";
            if (si == 1) { crit = "CAPACITE"; ord = "ASC"; }
            else if (si == 2) { crit = "CAPACITE"; ord = "DESC"; }
            else if (si == 3) { crit = "TEMPERATURE"; ord = "ASC"; }
            else if (si == 4) { crit = "TEMPERATURE"; ord = "DESC"; }
            model = frigoModel.trier(crit, ord);
        }
    }

    // Épuiser le curseur pour libérer le driver ODBC et éviter l'erreur S1010
    while (model->canFetchMore()) {
        model->fetchMore();
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 60);

        // Map: ID(0), Ref(1), Cap(2), Type(3), Stat(4), Date(5), Temp(6), Occ(7)
        QString dbId = model->record(i).value(0).toString();

        auto addItem = [&](int col, QString val, bool isBold = false) {
            QTableWidgetItem* item = new QTableWidgetItem(val);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFont(isBold ? QFont("Segoe UI", 11, QFont::Bold) : cellFont);
            if(isBold) item->setForeground(QBrush(QColor("#5D9CEC")));
            item->setData(Qt::UserRole, dbId);
            table->setItem(r, col, item);
        };

        addItem(0, model->record(i).value("REFERENCE").toString(), true); // Reference
        addItem(1, model->record(i).value("CAPACITE").toString() + " Kg"); // Capacité
        addItem(2, model->record(i).value("TYPE_POISSON").toString()); // Type Poisson

        // Statut Badge
        QString statusText = model->record(i).value("STATUT").toString();
        table->setItem(r, 3, new QTableWidgetItem(statusText)); // Set item for programmatic access
        table->setCellWidget(r, 3, createStatusBadge(statusText));

        QVariant dat = model->record(i).value("DATE_RESERVATION");
        QString dateStr = dat.typeId() == QMetaType::QDate || dat.typeId() == QMetaType::QDateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);
        addItem(4, dateStr); // Date Réservation

        // Classification sans nouvelle colonne — Coloration de la cellule Température (QLabel Badge pour fiabilité)
        QString fishType = model->record(i).value("TYPE_POISSON").toString();
        double currentTemp = model->record(i).value("TEMPERATURE").toDouble();

        auto getQualityInfo = [](const QString& type, double temp) -> QPair<QString, QString> {
            if (type == "Sans") return {"transparent", "#1f2937"};

            QMap<QString, QPair<double, double>> ranges;
            ranges["Sardine"] = {0.0, 4.0};
            ranges["Thon"]    = {-1.0, 2.0};
            ranges["Merlan"]  = {0.0, 3.0};
            ranges["Crevette"]= {-2.0, 1.0};
            ranges["Saumon"]  = {0.0, 2.0};

            if (!ranges.contains(type)) return {"#D1FAE5", "#065F46"};

            double min = ranges[type].first;
            double max = ranges[type].second;

            if (temp >= min && temp <= max) return {"#D1FAE5", "#065F46"}; // Vert
            if (temp < min - 2 || temp > max + 2) return {"#FEE2E2", "#991B1B"}; // Rouge
            return {"#FEF3C7", "#92400E"}; // Orange
        };

        QPair<QString, QString> colors = getQualityInfo(fishType, currentTemp);

        QWidget* tempContainer = new QWidget();
        QHBoxLayout* tempLayout = new QHBoxLayout(tempContainer);
        tempLayout->setContentsMargins(5, 5, 5, 5);

        QLabel* tempBadge = new QLabel(model->record(i).value(6).toString() + " °C");
        tempBadge->setAlignment(Qt::AlignCenter);
        tempBadge->setMinimumHeight(35);
        tempBadge->setStyleSheet(QString(
                                     "background-color: %1; "
                                     "color: %2; "
                                     "border-radius: 8px; "
                                     "font-weight: bold; "
                                     "padding: 5px;"
                                     ).arg(colors.first, colors.second));

        tempLayout->addWidget(tempBadge);
        table->setItem(r, 5, new QTableWidgetItem(model->record(i).value(6).toString() + " °C")); // Set item for programmatic access
        table->setCellWidget(r, 5, tempContainer);

        double cap_kg = model->record(i).value("CAPACITE").toDouble();
        double occ_kg = model->record(i).value("OCCUPATION").toDouble();

        double percent = (cap_kg > 0) ? (occ_kg / cap_kg) * 100.0 : 0.0;

        // Cap between 0 and 100 for display
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;

        QTableWidgetItem* occItem = new QTableWidgetItem(QString::number(percent, 'f', 1) + " %");
        occItem->setTextAlignment(Qt::AlignCenter);
        occItem->setFont(cellFont);
        if (percent >= 90) {
            occItem->setForeground(QBrush(QColor("#EF4444"))); // Rouge
            occItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
        } else if (percent >= 70) {
            occItem->setForeground(QBrush(QColor("#F59E0B"))); // Orange
        } else {
            occItem->setForeground(QBrush(QColor("#10B981"))); // Vert
        }
        occItem->setData(Qt::UserRole, dbId);
        table->setItem(r, 6, occItem);
        addItem(7, model->record(i).value(8).toString()); // Téléphone

        // Actions — passer l'ID de la BD directement dans le widget
        table->setCellWidget(r, 8, createActionButtons(r, dbId));

        // [ALERTE STOCKAGE] Check storage duration for specific fish types
        QMap<QString, int> thresholds;
        thresholds["Sardine"] = 2;
        thresholds["Thon"] = 3;
        thresholds["Merlan"] = 2;
        thresholds["Crevette"] = 1;
        thresholds["Saumon"] = 2;

        if (thresholds.contains(fishType)) {
            QDate resDate = dat.toDate();
            int daysStored = resDate.daysTo(QDate::currentDate());
            int limit = thresholds[fishType];

            if (daysStored > limit) {
                // Professional Highlight for overdue items
                QTableWidgetItem* dateItem = table->item(r, 4);
                dateItem->setForeground(QBrush(QColor("#B91C1C"))); // Darker red for text
                dateItem->setBackground(QBrush(QColor("#FEE2E2"))); // Soft red background
                dateItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
                dateItem->setToolTip(QString("DÉPASSEMENT : %1 jours (Limite : %2)").arg(daysStored).arg(limit));

                // Trigger a design alert (only for the first one found to avoid spamming)
                if (!sessionAlertShown) {
                    StorageAlert* alert = new StorageAlert(model->record(i).value(1).toString(), fishType, daysStored, this);
                    Arduino::triggerSoundEvent(Arduino::SoundEvent::Error);
                    alert->show();
                    sessionAlertShown = true;
                }
            }
        }
    }

    // Détruire proprement la requête pour libérer totalement ODBC
    model->clear();
    delete model;
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

QWidget* FrigoWindow::createActionButtons(int row, const QString& dbId)
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

    // Delete button — utilise l'ID de la BD directement (fix bug suppression)
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
    connect(deleteBtn, &QPushButton::clicked, [this, dbId]() {
        onDeleteFrigoById(dbId);
    });
    layout->addWidget(deleteBtn);

    return widget;
}

QString FrigoWindow::generateFrigoId()
{
    // IDFRIGO est de type NUMBER dans la base de données.
    // On génère un ID purement numérique basé sur le timestamp pour éviter ORA-01722.
    return QString::number(QDateTime::currentMSecsSinceEpoch()).right(5);
}

void FrigoWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void FrigoWindow::onSort(int index)
{
    Q_UNUSED(index);
    populateTable(searchInput->text());
}

void FrigoWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}

void FrigoWindow::onShowStatistics()
{
    QMap<QString, double> typeCount, typeCapacity, statusCount;
    double totalOccupation = 0;

    QSqlQuery query("SELECT TYPE_POISSON, CAPACITE, STATUT, OCCUPATION FROM FRIGOS");
    while (query.next()) {
        QString type = query.value(0).toString();
        double cap = query.value(1).toDouble();
        QString stat = query.value(2).toString();
        double occ = query.value(3).toDouble();

        typeCount[type]++;
        typeCapacity[type] += cap;
        statusCount[stat]++;
        totalOccupation += occ;
    }

    FrigoStatisticsDialog dlg(typeCount, typeCapacity, statusCount, totalOccupation, this);
    dlg.exec();
}

void FrigoWindow::onGeneratePDF()
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Avertissement", "Veuillez sélectionner un frigo dans le tableau pour générer son reçu.");
        return;
    }

    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;

    QString dbId = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION, TELEPHONE FROM FRIGOS WHERE IDFRIGO = :id");
    query.bindValue(":id", dbId);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer les données du frigo.");
        return;
    }

    QString ref = query.value("REFERENCE").toString();
    QString cap = query.value("CAPACITE").toString();
    QString temp = query.value("TEMPERATURE").toString();
    QString stat = query.value("STATUT").toString();

    QVariant dat = query.value("DATE_RESERVATION");
    QString date = dat.typeId() == QMetaType::QDate || dat.typeId() == QMetaType::QDateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);

    QString fish = query.value("TYPE_POISSON").toString();
    QString occ = query.value("OCCUPATION").toString();

    QString path = QFileDialog::getSaveFileName(this, "Exporter Reçu PDF", "recu_" + ref + ".pdf", "PDF (*.pdf)");
    if(path.isEmpty()) return;

    QPdfWriter w(path);
    w.setPageSize(QPageSize(QPageSize::A4));
    w.setPageMargins(QMarginsF(15,15,15,15), QPageLayout::Millimeter);
    w.setResolution(96);

    QPainter p(&w);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = w.width();
    int y = 40;

    p.setPen(QColor("#1e40af"));
    p.setFont(QFont("Segoe UI", 24, QFont::Bold));
    p.drawText(0, y, W, 50, Qt::AlignHCenter, "Reçu de Paiement - Frigo");
    y += 60;

    p.setPen(QColor("#6b7280"));
    p.setFont(QFont("Segoe UI", 12));
    p.drawText(0, y, W, 20, Qt::AlignHCenter, "Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    y += 40;

    p.setPen(QPen(QColor("#3b82f6"), 3));
    p.drawLine(0, y, W, y);
    y += 40;

    p.setPen(QColor("#1f2937"));
    p.setFont(QFont("Segoe UI", 12));

    p.drawText(20, y, "Référence du Frigo : " + ref); y += 40;
    p.drawText(20, y, "Date de réservation : " + date); y += 40;
    p.drawText(20, y, "Capacité : " + cap + " Kg"); y += 40;
    p.drawText(20, y, "Température : " + temp + " °C"); y += 40;
    p.drawText(20, y, "Occupation : " + occ + " %"); y += 40;
    p.drawText(20, y, "Type de poisson : " + fish); y += 40;
    p.drawText(20, y, "Téléphone Employé : " + query.value("TELEPHONE").toString()); y += 40;
    p.drawText(20, y, "Statut actuel : " + stat); y += 40;

    // Simulation d'un prix de stockage
    double pricePerKg = 2.5;
    double total = cap.toDouble() * pricePerKg;

    y += 20;
    p.setPen(QPen(QColor("#e5e7eb"), 2));
    p.drawLine(0, y, W, y);
    y += 40;

    p.setPen(QColor("#166534"));
    p.setFont(QFont("Segoe UI", 14, QFont::Bold));
    p.drawText(20, y, W, 30, Qt::AlignRight, "Montant Total : " + QString::number(total, 'f', 2) + " DT   ");

    p.end();

    QMessageBox::information(this, "PDF généré", "Le reçu PDF a été généré avec succès :\n" + path);
}

void FrigoWindow::onAddFrigo()
{
    AddFrigoDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        FrigoModel newFrigoData = dialog.getData();

        QString newId = dialog.getData().getId(); // Should be generated if empty
        if (newId.isEmpty() || newId == "0") newId = generateFrigoId();

        FrigoModel toSave(newId, newFrigoData.getRef(), newFrigoData.getCap(),
                          newFrigoData.getType(), newFrigoData.getStat(),
                          newFrigoData.getDateRes(), newFrigoData.getTemp(), newFrigoData.getOcc(),
                          newFrigoData.getTelephone());

        if (toSave.ajouter()) {
            populateTable();
            QMessageBox::information(this, "Succès", "Frigo ajouté avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur", "L'ajout a échoué.\nErreur de la base de données :\n" + toSave.getLastError());
        }
    }
}

void FrigoWindow::onEditFrigo(int row)
{
    if (row < 0) return;
    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;
    QString id = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM FRIGOS WHERE IDFRIGO = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {

        QVariant dat = query.value("DATE_RESERVATION");
        QString dateResStr = dat.typeId() == QMetaType::QDate || dat.typeId() == QMetaType::QDateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);

        FrigoModel current(
            query.value("IDFRIGO").toString(),
            query.value("REFERENCE").toString(),
            query.value("CAPACITE").toDouble(),
            query.value("TYPE_POISSON").toString(),
            query.value("STATUT").toString(),
            dateResStr,
            query.value("TEMPERATURE").toDouble(),
            query.value("OCCUPATION").toDouble(),
            query.value("TELEPHONE").toString()
            );

        AddFrigoDialog dialog(this, &current);
        if (dialog.exec() == QDialog::Accepted) {
            FrigoModel updated = dialog.getData();
            if (updated.modifier(id)) {
                populateTable();
                QMessageBox::information(this, "Succès", "Frigo mis à jour.");
            } else {
                QMessageBox::critical(this, "Erreur", "La modification a échoué.");
            }
        }
    }
}

void FrigoWindow::onDeleteFrigo(int row)
{
    if (row < 0) return;
    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;
    QString id = item->data(Qt::UserRole).toString();
    onDeleteFrigoById(id);
}

void FrigoWindow::onDeleteFrigoById(const QString& id)
{
    if (id.isEmpty()) {
        QMessageBox::critical(this, "Erreur", "Identifiant du frigo introuvable.");
        return;
    }

    // Récupérer la référence pour afficher dans la confirmation
    QSqlQuery refQuery;
    refQuery.prepare("SELECT REFERENCE FROM FRIGOS WHERE IDFRIGO = :id");
    refQuery.bindValue(":id", id);
    QString ref = id;
    if (refQuery.exec() && refQuery.next()) {
        ref = refQuery.value(0).toString();
    }
    refQuery.finish();

    if (showStyledActionDialog(this, "Suppression Frigo", 
                               QString("Voulez-vous vraiment supprimer le frigo <b>%1</b> ?").arg(ref),
                               "#EF4444", "#B91C1C", "🗑️")) {
        FrigoModel f;
        if (f.supprimer(id)) {
            showStyledActionDialog(this, "Succès", "Le frigo a été supprimé avec succès.", "#10B981", "#059669", "✅", false);
            populateTable();
        } else {
            QMessageBox::critical(this, "Erreur", "La suppression a échoué.\n" + f.getLastError());
        }
    }
}

void FrigoWindow::onSendSMS()
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
                             "Veuillez sélectionner un frigo.");
        return;
    }

    QTableWidgetItem* refItem = table->item(row, 0);
    QTableWidgetItem* tempItem = table->item(row, 5);
    QTableWidgetItem* phoneItem = table->item(row, 7);

    if (!refItem || !tempItem || !phoneItem) {
        QMessageBox::critical(this, "Erreur", "Données du tableau inaccessibles.");
        return;
    }

    QString ref = refItem->text();
    QString temp = tempItem->text();
    QString storedPhone = phoneItem->text().trimmed();
    QString defaultPhone = storedPhone;

    if (!storedPhone.isEmpty() && !storedPhone.startsWith("216")) {
        defaultPhone = "216" + storedPhone;
    } else if (storedPhone.isEmpty()) {
        defaultPhone = "216";
    }

    bool okNum;
    QString phone = QInputDialog::getText(
        this,
        "Destinataire",
        "Numéro (216XXXXXXXX) :",
        QLineEdit::Normal,
        defaultPhone,
        &okNum
        );

    if (!okNum || phone.isEmpty())
        return;

    bool okMsg;
    QString defaultMsg =
        QString("Alerte Stockage : Frigo %1 température %2.")
            .arg(ref)
            .arg(temp);

    QString message =
        QInputDialog::getMultiLineText(
            this,
            "Message",
            "Contenu du SMS :",
            defaultMsg,
            &okMsg
            );

    if (!okMsg || message.isEmpty())
        return;


    // ================= CONFIG API =================

    QString token =
        "33q4B0cRDNcFxawMP5phTJe4fZJ=YmBdD9I8liBHLux3!DCHKgMt81DDxD8c0TbLhxVK65cy5cXgQpFRfefl6nFTDQjFqojllRFwui55";

    QString sender =
        "TunSMS Test"; // ⚠️ remplacer par sender MyStudents

    QString apiUrl =
        "https://mystudents.tunisiesms.tn/api/sms";


    // ================= ENVOI JSON =================

    QNetworkAccessManager *manager =
        new QNetworkAccessManager(this);

    QNetworkRequest request{QUrl(apiUrl)};

    request.setRawHeader(
        "Authorization",
        "Bearer " + token.toUtf8()
        );

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );


    // JSON principal

    QJsonObject mainObject;

    mainObject["type"] = "55";
    mainObject["sender"] = sender;


    // tableau sms

    QJsonArray smsArray;

    QJsonObject smsObject;

    smsObject["mobile"] = phone;
    smsObject["sms"] = message;

    smsArray.append(smsObject);

    mainObject["sms"] = smsArray;

    QJsonDocument doc(mainObject);

    QNetworkReply *reply =
        manager->post(request, doc.toJson());

    connect(reply,
            &QNetworkReply::finished,
            this,
            [=]()
            {
                if(reply->error() ==
                    QNetworkReply::NoError)
                {
                    QString response =
                        reply->readAll();

                    QMessageBox::information(
                        this,
                        "Succès",
                        "SMS envoyé avec succès\n"
                            + response
                        );
                }
                else
                {
                    QMessageBox::critical(
                        this,
                        "Erreur SMS",
                        reply->errorString()
                        );
                }

                reply->deleteLater();
                manager->deleteLater();
            });
}

void FrigoWindow::onShowClassification()
{
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Guide de Classification - PortFlow");
    dlg->setMinimumSize(550, 550);
    // On force le fond blanc et le texte sombre pour tout le dialogue
    dlg->setStyleSheet("QDialog { background-color: white; } QLabel { color: #1e293b; }");

    QVBoxLayout* mainLay = new QVBoxLayout(dlg);
    mainLay->setContentsMargins(30,30,30,30);
    mainLay->setSpacing(15);

    QLabel* title = new QLabel(QString::fromUtf8("🌡️ Températures de Conservation"));
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setStyleSheet("color: #1e3a5f;"); // Bleu foncé pour le titre
    mainLay->addWidget(title);

    auto addSpecies = [&](const QString& name, const QString& range) {
        QFrame* rowFrame = new QFrame();
        rowFrame->setStyleSheet("background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px;");
        QHBoxLayout* row = new QHBoxLayout(rowFrame);

        QLabel* n = new QLabel(QString::fromUtf8(name.toUtf8()));
        n->setFont(QFont("Segoe UI", 12, QFont::Bold));
        n->setStyleSheet("color: #1e293b; border: none;");

        QLabel* r = new QLabel(QString::fromUtf8(range.toUtf8()));
        r->setFont(QFont("Segoe UI", 11, QFont::Bold));
        r->setStyleSheet("color: #2563eb; background: #eff6ff; padding: 5px; border: none;");

        row->addWidget(n);
        row->addStretch();
        row->addWidget(r);
        mainLay->addWidget(rowFrame);
    };

    addSpecies("🐟 Sardine", "0.0°C  à  4.0°C");
    addSpecies("🍣 Thon", "-1.0°C  à  2.0°C");
    addSpecies("🐟 Merlan", "0.0°C  à  3.0°C");
    addSpecies("🍤 Crevette", "-2.0°C  à  1.0°C");
    addSpecies("🐟 Saumon", "0.0°C  à  2.0°C");

    // Légende
    QLabel* leg = new QLabel(QString::fromUtf8("Légende (Couleurs dans le tableau) :"));
    leg->setFont(QFont("Segoe UI", 10, QFont::Bold));
    mainLay->addWidget(leg);

    QHBoxLayout* legend = new QHBoxLayout();
    auto makeLeg = [&](const QString& col, const QString& txt, const QString& tCol) {
        QLabel* l = new QLabel(txt);
        l->setFixedHeight(35);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet(QString("background: %1; color: %2; border-radius: 5px; font-weight: bold;").arg(col, tCol));
        legend->addWidget(l, 1);
    };
    makeLeg("#D1FAE5", "Optimal", "#065F46");
    makeLeg("#FEF3C7", "Alerte", "#92400E");
    makeLeg("#FEE2E2", "Danger", "#991B1B");
    mainLay->addLayout(legend);

    mainLay->addStretch();
    QPushButton* close = new QPushButton("Fermer");
    close->setFixedHeight(45);
    close->setStyleSheet("background: #2563eb; color: white; border-radius: 10px; font-weight: bold;");
    connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
    mainLay->addWidget(close);

    dlg->exec();
}

QFrame* FrigoWindow::createCalendarCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 20px;
            border: 1.5px solid #CBD5E1;
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    QLabel* title = new QLabel("🗓️  Planning Flotte");
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setStyleSheet("color: #1e3a5f; border: none;");
    layout->addWidget(title);

    calendar = new QCalendarWidget();
    calendar->setGridVisible(false);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setStyleSheet(R"(
        QCalendarWidget {
            background-color: white;
            border: none;
            border-radius: 12px;
        }
        /* Navigation Bar */
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e3a5f, stop:1 #3b82f6);
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
            min-height: 45px;
        }
        QCalendarWidget QToolButton {
            color: white;
            font-weight: bold;
            font-size: 11pt;
            background: transparent;
            border: none;
            border-radius: 5px;
            margin: 3px;
        }
        QCalendarWidget QToolButton:hover { background: rgba(255, 255, 255, 0.15); }

        /* Day Numbers */
        QCalendarWidget QAbstractItemView:enabled {
            color: #1e293b;
            selection-background-color: #3b82f6;
            selection-color: white;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: #cbd5e1;
        }

        /* Weekdays Header */
        QCalendarWidget QHeaderView::section {
            color: #ffffffff;
            font-weight: bold;
            font-size: 9pt;
            background-color: #f1f5f9;
            border: none;
            padding: 5px;
        }

        /* Menus and Spinboxes */
        QCalendarWidget QMenu { background-color: white; color: #1e3a5f; }
        QCalendarWidget QSpinBox { color: white; background-color: transparent; }
    )");
    connect(calendar, &QCalendarWidget::clicked, this, &FrigoWindow::updateCalendarDetails);
    layout->addWidget(calendar);

    QFrame* line = new QFrame();
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #e2e8f0;");
    layout->addWidget(line);

    calendarDetails = new QListWidget();
    calendarDetails->setStyleSheet(R"(
        QListWidget {
            border: none;
            background-color: #f8fafc;
            border-radius: 10px;
            padding: 5px;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 0.5px solid #e2e8f0;
            font-size: 10pt;
        }
    )");
    calendarDetails->setMinimumHeight(150);
    layout->addWidget(calendarDetails);

    // Apply correct palette immediately based on current theme
    setDarkMode(isDarkMode);

    return card;
}

void FrigoWindow::setDarkMode(bool dark)
{
    isDarkMode = dark;

    // Re-apply calendar stylesheet with theme-correct colors
    QString dayColor        = dark ? "#E2E8F0" : "#1e293b";
    QString dayDisabled     = dark ? "#4A5568" : "#cbd5e1";
    QString calBg           = dark ? "#2D3748" : "white";
    QString headerBg        = dark ? "#1A202C" : "#f1f5f9";
    QString headerColor     = dark ? "#CBD5E0" : "#1e3a5f";
    QString selectedBg      = dark ? "#3B82F6" : "#3b82f6";

    calendar->setStyleSheet(QString(R"(
        QCalendarWidget {
            background-color: %1;
            border: none;
            border-radius: 12px;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e3a5f, stop:1 #3b82f6);
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
            min-height: 45px;
        }
        QCalendarWidget QToolButton {
            color: white;
            font-weight: bold;
            font-size: 11pt;
            background: transparent;
            border: none;
            border-radius: 5px;
            margin: 3px;
        }
        QCalendarWidget QToolButton:hover { background: rgba(255, 255, 255, 0.15); }
        QCalendarWidget QAbstractItemView {
            background-color: %1;
            color: %2;
        }
        QCalendarWidget QAbstractItemView:enabled {
            color: %2;
            selection-background-color: %5;
            selection-color: white;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: %3;
        }
        QCalendarWidget QHeaderView::section {
            color: %4;
            font-weight: bold;
            font-size: 9pt;
            background-color: %6;
            border: none;
            padding: 5px;
        }
        QCalendarWidget QMenu { background-color: %1; color: %2; }
        QCalendarWidget QSpinBox { color: white; background-color: transparent; }
    )").arg(calBg, dayColor, dayDisabled, headerColor, selectedBg, headerBg));

    // QPalette is the ONLY reliable way to change day-number text color in Qt's calendar
    QPalette calPal = calendar->palette();
    if (dark) {
        calPal.setColor(QPalette::Text,            QColor("#E2E8F0"));
        calPal.setColor(QPalette::Base,            QColor("#2D3748"));
        calPal.setColor(QPalette::Window,          QColor("#2D3748"));
        calPal.setColor(QPalette::AlternateBase,   QColor("#374151"));
        calPal.setColor(QPalette::ButtonText,      QColor("#E2E8F0"));
        calPal.setColor(QPalette::Button,          QColor("#1A202C"));
        calPal.setColor(QPalette::Highlight,       QColor("#3B82F6"));
        calPal.setColor(QPalette::HighlightedText, QColor("white"));
    } else {
        calPal.setColor(QPalette::Text,            QColor("#1e293b"));
        calPal.setColor(QPalette::Base,            QColor("white"));
        calPal.setColor(QPalette::Window,          QColor("white"));
        calPal.setColor(QPalette::AlternateBase,   QColor("#f8fafc"));
        calPal.setColor(QPalette::ButtonText,      QColor("#1e3a5f"));
        calPal.setColor(QPalette::Button,          QColor("#f1f5f9"));
        calPal.setColor(QPalette::Highlight,       QColor("#3b82f6"));
        calPal.setColor(QPalette::HighlightedText, QColor("white"));
    }
    calendar->setPalette(calPal);

    // Also set it on the internal QAbstractItemView that renders the day cells
    QAbstractItemView* calView = calendar->findChild<QAbstractItemView*>();
    if (calView) {
        calView->setPalette(calPal);
    }

    loadCalendarData();
    updateCalendarDetails(calendar->selectedDate());
}

void FrigoWindow::loadCalendarData()
{
    maintenanceDates.clear();
    deliveryDates.clear();

    // Clear previous formatting
    calendar->setDateTextFormat(QDate(), QTextCharFormat());

    // 1. Fetch Maintenance Dates (Last and Next)
    QSqlQuery maintQuery("SELECT NOMBATEAU, DATE_DERNIERE_MAINTENANCE, DATE_PROCHAINE_MAINTENANCE FROM BATEAUX");
    while (maintQuery.next()) {
        QString name = maintQuery.value(0).toString();
        QDate lastMaint = maintQuery.value(1).toDate();
        QDate nextMaint = maintQuery.value(2).toDate();

        if (lastMaint.isValid()) {
            maintenanceDates[lastMaint].append("📅 Derniére maintenance du bateau: " + name);
            QTextCharFormat fmt;
            if (isDarkMode) {
                fmt.setBackground(QBrush(QColor("#064E3B"))); // Dark Emerald
                fmt.setForeground(QBrush(QColor("#6EE7B7"))); // Light Emerald
            } else {
                fmt.setBackground(QBrush(QColor("#ECFDF5")));
                fmt.setForeground(QBrush(QColor("#047857")));
            }
            fmt.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(lastMaint, fmt);
        }
        if (nextMaint.isValid()) {
            maintenanceDates[nextMaint].append("🛠️ Future: " + name);
            QTextCharFormat fmt;
            if (isDarkMode) {
                fmt.setBackground(QBrush(QColor("#4C0519"))); // Dark Rose
                fmt.setForeground(QBrush(QColor("#FDA4AF"))); // Light Rose
            } else {
                fmt.setBackground(QBrush(QColor("#FFF1F2")));
                fmt.setForeground(QBrush(QColor("#BE123C")));
            }
            fmt.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(nextMaint, fmt);
        }
    }

    // 2. Fetch Delivery Dates
    QSqlQuery deliveryQuery("SELECT ADRESSELIVRAISON, DATELIVRAISON FROM LIVRAISONS");
    while (deliveryQuery.next()) {
        QString addr = deliveryQuery.value(0).toString();
        QDate date = deliveryQuery.value(1).toDate();
        if (date.isValid()) {
            deliveryDates[date].append("🚚 Distination: " + addr);

            QTextCharFormat cur = calendar->dateTextFormat(date);
            QTextCharFormat fmt;
            if (isDarkMode) {
                if (cur.background().style() != Qt::NoBrush) {
                    fmt.setBackground(QBrush(QColor("#082F49"))); // Dark Cyan (Mix)
                    fmt.setForeground(QBrush(QColor("#7DD3FC")));
                } else {
                    fmt.setBackground(QBrush(QColor("#172554"))); // Dark Blue
                    fmt.setForeground(QBrush(QColor("#93C5FD")));
                }
            } else {
                if (cur.background().style() != Qt::NoBrush) {
                    fmt.setBackground(QBrush(QColor("#F0F9FF"))); // Bleu ciel (Mix)
                    fmt.setForeground(QBrush(QColor("#0369A1")));
                } else {
                    fmt.setBackground(QBrush(QColor("#EFF6FF"))); // Bleu clair
                    fmt.setForeground(QBrush(QColor("#1D4ED8")));
                }
            }
            fmt.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(date, fmt);
        }
    }
}

void FrigoWindow::updateCalendarDetails(const QDate &date)
{
    calendarDetails->clear();
    bool found = false;

    if (maintenanceDates.contains(date)) {
        for (const QString &s : maintenanceDates[date]) {
            QListWidgetItem *item = new QListWidgetItem(s);
            item->setForeground(isDarkMode ? QColor("#F87171") : QColor("#B91C1C"));
            calendarDetails->addItem(item);
            found = true;
        }
    }

    if (deliveryDates.contains(date)) {
        for (const QString &s : deliveryDates[date]) {
            QListWidgetItem *item = new QListWidgetItem(s);
            item->setForeground(isDarkMode ? QColor("#60A5FA") : QColor("#1E40AF"));
            calendarDetails->addItem(item);
            found = true;
        }
    }

    if (!found) {
        QListWidgetItem *item = new QListWidgetItem("Aucun événement");
        item->setForeground(isDarkMode ? QColor("#94A3B8") : QColor("#94a3b8")); // Same or slightly adjusted
        calendarDetails->addItem(item);
    }
}

void FrigoWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    populateTable(searchInput->text());
    loadCalendarData();
    updateCalendarDetails(calendar->selectedDate());
}
