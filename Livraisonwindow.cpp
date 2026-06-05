#define LIVRAISONWINDOW_CPP
#include "Livraisonwindow.h"
#include "LivraisonTrackingDialog.h"
#include <algorithm>
#include "AddLivraisonDialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QDate>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QFile>
#include <QSqlRecord>
#include <QSqlError>
#include <QPainter>
#include "LivraisonStatisticsDialog.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

// ==================== UI HELPERS ====================
class MoveFilter : public QObject {
    QDialog* d; bool m=false; QPoint o;
public:
    MoveFilter(QDialog* dlg) : QObject(dlg), d(dlg) {}
    bool eventFilter(QObject* /*obj*/, QEvent* e) override {
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

// --- LiveProgressIndicator Implementation ---
LiveProgressIndicator::LiveProgressIndicator(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(30);
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this](){
        m_animationOffset += 0.05;
        if (m_animationOffset > 10.0) m_animationOffset = 0;
        update();
    });
    m_animationTimer->start(50);
}

void LiveProgressIndicator::setProgress(double progress) {
    m_progress = qBound(0.0, progress, 1.0);
    update();
}

void LiveProgressIndicator::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int size = 24;
    QRectF rect(width()/2.0 - size/2.0, height()/2.0 - size/2.0, size, size);

    // Dynamic color logic
    QColor color;
    if (m_progress < 0.33) color = QColor("#EF4444");      // Red
    else if (m_progress < 0.66) color = QColor("#F59E0B"); // Yellow
    else color = QColor("#10B981");                         // Green

    // 1. Draw Background Track (Gray Ring)
    painter.setPen(QPen(QColor("#E2E8F0"), 4, Qt::SolidLine, Qt::RoundCap));
    painter.drawEllipse(rect);

    // 2. Draw Progress Arc (Filling relative to m_progress)
    int spanAngle = (int)(-m_progress * 360 * 16);
    int startAngle = 90 * 16; // Start at 12 o'clock
    
    QPen progressPen(color, 4, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);
    painter.drawArc(rect, startAngle, spanAngle);

    // 3. Central Pulse / Symbol
    if (m_progress > 0 && m_progress < 1.0) {
        double pulse = std::abs(std::sin(m_animationOffset));
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        double dotSize = 4 + (pulse * 4);
        painter.drawEllipse(QRectF(width()/2.0 - dotSize/2.0, height()/2.0 - dotSize/2.0, dotSize, dotSize));
    } else if (m_progress >= 1.0) {
        painter.setPen(QPen(color, 3));
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter.drawText(rect, Qt::AlignCenter, "OK");
    }
}
// --------------------------------------------


LivraisonWindow::LivraisonWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Initial load from database
    populateTable();

    updateStats();
    // populateTable(); // REMOVED redundant call
    loadStyleSheet();

    liveUpdateTimer = new QTimer(this);
    connect(liveUpdateTimer, &QTimer::timeout, this, &LivraisonWindow::onLiveUpdate);
    liveUpdateTimer->start(5000); // Update every 5 seconds
}

void LivraisonWindow::loadStyleSheet()
{
    QFile file(":/style/livraison.css");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString style = stream.readAll();
        if (centralWidget()) {
            centralWidget()->setStyleSheet(style);
        }
        this->setStyleSheet(style); // Also keep it on window for direct usage
        qDebug() << "Successfully loaded CSS from resources";
    } else {
        qDebug() << "Failed to load CSS from resources:" << file.errorString();
    }
}




LivraisonWindow::~LivraisonWindow()
{
}

void LivraisonWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Livraisons");
    
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Content area (since this widget is used as a page in MainWindow's StackedWidget)
    QWidget* contentArea = createContentArea();
    mainLayout->addWidget(contentArea);
}

QWidget* LivraisonWindow::createContentArea()
{
    QWidget* content = new QWidget();
    content->setObjectName("livraisonContentArea");

    QVBoxLayout* layout = new QVBoxLayout(content);

    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    // Header
    QFrame* header = createHeader();
    layout->addWidget(header);

    // Filter Toolbar (restored)
    layout->addWidget(createToolbar());

    // Stats Area
    QFrame* statsArea = createStatsArea();
    layout->addWidget(statsArea);

    // Table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* LivraisonWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Livraisons");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Coordination des transports et logistique");
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

    QPushButton* addBtn   = makeBtn("➕  Nouvelle Livraison", "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &LivraisonWindow::onAddLivraison);

    lay->addWidget(addBtn);
    return hdr;
}

QFrame* LivraisonWindow::createToolbar()
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
    searchInput->setPlaceholderText("🔍  Rechercher par date, adresse ou statut...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#F0F4F8; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#2C3E50;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &LivraisonWindow::onSearch);
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
    connect(statsBtn, &QPushButton::clicked, this, &LivraisonWindow::onShowStatistics);
    lay->addWidget(statsBtn);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Date (Récent)", "Date (Ancien)", "Prix ↑", "Prix ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#F0F4F8; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#2C3E50;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LivraisonWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

QFrame* LivraisonWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setObjectName("livraisonTableCard");

    QVBoxLayout* layout = new QVBoxLayout(card);

    layout->setContentsMargins(3, 3, 3, 3);

    // Container blanc pour le tableau
    QFrame* whiteContainer = new QFrame();
    whiteContainer->setObjectName("livraisonWhiteContainer");

    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);

    containerLayout->setContentsMargins(25, 25, 25, 25);

    setupTable();
    containerLayout->addWidget(table);

    layout->addWidget(whiteContainer);

    return card;
}

QFrame* LivraisonWindow::createStatsArea()
{
    QFrame* statsArea = new QFrame();
    statsArea->setObjectName("statsArea");
    
    QHBoxLayout* layout = new QHBoxLayout(statsArea);
    layout->setSpacing(20);
    layout->setContentsMargins(0, 0, 0, 0);

    // Total Deliveries Card
    QFrame* totalCard = new QFrame();
    totalCard->setProperty("class", "stats-card");
    totalCard->setFixedHeight(120);
    
    QHBoxLayout* totalLayout = new QHBoxLayout(totalCard);
    totalLayout->setContentsMargins(25, 15, 25, 15);
    
    QVBoxLayout* totalLabels = new QVBoxLayout();
    QLabel* totalTitle = new QLabel("Total Livraisons");
    totalTitle->setProperty("class", "stats-label");
    totalTitle->setStyleSheet("border: none; background: transparent;");
    
    totalDeliveriesLabel = new QLabel("0");
    totalDeliveriesLabel->setProperty("class", "stats-value");
    totalDeliveriesLabel->setStyleSheet("border: none; background: transparent;");
    
    totalLabels->addWidget(totalTitle);
    totalLabels->addWidget(totalDeliveriesLabel);
    
    totalLayout->addLayout(totalLabels);
    totalLayout->addStretch();
    
    // Efficiency Card
    QFrame* efficiencyCard = new QFrame();
    efficiencyCard->setProperty("class", "stats-card");
    efficiencyCard->setFixedHeight(120);
    
    QHBoxLayout* effLayout = new QHBoxLayout(efficiencyCard);
    effLayout->setContentsMargins(25, 15, 25, 15);
    
    QVBoxLayout* effLabels = new QVBoxLayout();
    QLabel* effTitle = new QLabel("Efficacité (Livrées)");
    effTitle->setProperty("class", "stats-label");
    effTitle->setStyleSheet("border: none; background: transparent;");
    
    efficiencyLabel = new QLabel("0%");
    efficiencyLabel->setProperty("class", "stats-value");
    efficiencyLabel->setStyleSheet("border: none; background: transparent;");
    
    effLabels->addWidget(effTitle);
    effLabels->addWidget(efficiencyLabel);
    
    effLayout->addLayout(effLabels);
    effLayout->addStretch();

    layout->addWidget(totalCard);
    layout->addWidget(efficiencyCard);

    return statsArea;
}

void LivraisonWindow::updateStats()
{
    int total = 0;
    int delivered = 0;
    
    // Scoping the query ensures the handle is released immediately
    {
        QSqlQuery query("SELECT STATUT FROM LIVRAISONS");
        while (query.next()) {
            total++;
            if (query.value(0).toString() == "Livré" || query.value(0).toString() == "Arrivé") {
                delivered++;
            }
        }
    }
    
    double efficiency = (total > 0) ? (static_cast<double>(delivered) / total * 100.0) : 0.0;
    
    totalDeliveriesLabel->setText(QString::number(total));
    efficiencyLabel->setText(QString::number(efficiency, 'f', 1) + "%");
}

void LivraisonWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({"Référence", "Date", "Destination", "Livreur", "Statut", "Prix", "ETA", "Avancement", "Actions"});

    /* Responsive columns (matching Bateau styling) */
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Adresse stretch

    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);
    table->setFrameShape(QFrame::NoFrame);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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
        QHeaderView::section:first {
            border-top-left-radius: 14px;
        }
        QHeaderView::section:last {
            border-top-right-radius: 14px;
        }
    )");

    table->setObjectName("livraisonTable");

    table->setColumnWidth(0, 110);   // Référence
    table->setColumnWidth(1, 110);   // Date
    table->setColumnWidth(2, 250);   // Destination (Stretch)
    table->setColumnWidth(3, 130);   // Livreur
    table->setColumnWidth(4, 120);   // Statut
    table->setColumnWidth(5, 100);   // Prix
    table->setColumnWidth(6, 100);   // ETA
    table->setColumnWidth(7, 100);   // Avancement
    table->setColumnWidth(8, 200);   // Actions (Increased from 160)

    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

void LivraisonWindow::populateTable(const QString& filterText, const QString& sortCritere, const QString& sortOrdre)
{
    table->setRowCount(0);
    QSqlQueryModel* model;
    
    if (!sortCritere.isEmpty()) {
        model = livraisons.trier(sortCritere, sortOrdre);
    } else if (filterText.isEmpty()) {
        model = livraisons.afficher();
    } else {
        model = livraisons.rechercher(filterText);
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 60);

        int id = model->record(i).value("ID").toInt();
        QString reference = model->record(i).value("REFERENCE").toString();
        QString date = model->record(i).value("DATELIV").toDate().toString("dd/MM/yyyy");
        QString adresse = model->record(i).value("ADRESSE").toString();
        QString chauffeur = model->record(i).value("ID_EMPLOYE").toString();
        QString statut = model->record(i).value("STATUT").toString();
        
        // Format price to avoid scientific notation/weird displays
        double prixVal = model->record(i).value("PRIX").toDouble();
        QString prix = QString::number(prixVal, 'f', 2) + " DT";

        table->setItem(row, 0, new QTableWidgetItem(reference));
        table->setItem(row, 1, new QTableWidgetItem(date));
        table->setItem(row, 2, new QTableWidgetItem(adresse));
        table->setItem(row, 3, new QTableWidgetItem(chauffeur));
        table->setCellWidget(row, 4, createStatusBadge(statut));
        table->setItem(row, 5, new QTableWidgetItem(prix));

        // ETA Column (6) - Using the helper from the model
        Livraison tempLiv;
        tempLiv.setID(id);
        table->setItem(row, 6, new QTableWidgetItem(tempLiv.getETA()));
        table->item(row, 6)->setTextAlignment(Qt::AlignCenter);

        // Live Progress Indicator (Column 7)
        if (statut == "En chemin") {
            QDateTime start = model->record(i).value("DATE_DEPART").toDateTime();
            double duration = model->record(i).value("DUREE_ESTIMEE").toDouble();
            
            LiveProgressIndicator* indicator = new LiveProgressIndicator();
            if (start.isValid() && duration > 0) {
                double elapsed = start.secsTo(QDateTime::currentDateTime());
                indicator->setProgress(elapsed / duration);
            }
            // Store data in the widget for timer updates
            indicator->setProperty("startTime", start);
            indicator->setProperty("duration", duration);
            table->setCellWidget(row, 7, indicator);
        } else {
            table->setItem(row, 7, new QTableWidgetItem("---"));
            table->item(row, 7)->setTextAlignment(Qt::AlignCenter);
        }

        // Action Buttons (Column 8)
        table->setCellWidget(row, 8, createActionButtons(row)); 
        table->item(row, 0)->setData(Qt::UserRole, id); 
    }
    delete model;
    updateStats();
}

QWidget* LivraisonWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* badge = new QLabel(status);
    badge->setFixedHeight(30);
    badge->setAlignment(Qt::AlignCenter);

    badge->setProperty("class", "status-badge");
    if (status == "Livré" || status == "Arrivé") {
        badge->setProperty("class", "status-badge status-livre");
    } else if (status == "En cours") {
        badge->setProperty("class", "status-badge status-en-cours");
    } else if (status == "Annulé" || status == "Canceled") {
        badge->setProperty("class", "status-badge status-annule");
    } else {
        badge->setProperty("class", "status-badge status-en-attente");
    }
    // Re-polish to apply styles from the new properties
    badge->style()->unpolish(badge);
    badge->style()->polish(badge);

    
    layout->addWidget(badge);
    return widget;
}

QWidget* LivraisonWindow::createActionButtons(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 0, 10, 0); // Added margins
    layout->setSpacing(12); // Increased spacing
    layout->setAlignment(Qt::AlignCenter);

    QPushButton* editBtn = new QPushButton("✏️");
    QPushButton* deleteBtn = new QPushButton("🗑️");
    QPushButton* pdfBtn = new QPushButton("📄");
    QPushButton* trackBtn = new QPushButton("📍");

    editBtn->setFixedSize(36, 36);
    deleteBtn->setFixedSize(36, 36);
    pdfBtn->setFixedSize(36, 36);
    trackBtn->setFixedSize(36, 36);
    
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    pdfBtn->setCursor(Qt::PointingHandCursor);
    trackBtn->setCursor(Qt::PointingHandCursor);
    
    editBtn->setProperty("class", "action-btn edit-btn");
    deleteBtn->setProperty("class", "action-btn delete-btn");
    pdfBtn->setProperty("class", "action-btn pdf-btn");
    trackBtn->setProperty("class", "action-btn track-btn");

    // Initial styling for track button
    trackBtn->setStyleSheet("QPushButton { background-color: #E0F2FE; border: none; border-radius: 8px; font-size: 16px; } QPushButton:hover { background-color: #BAE6FD; }");

    // Force style refresh for dynamic buttons
    for (QPushButton* btn : {editBtn, deleteBtn, pdfBtn, trackBtn}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }


    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditLivraison(row); });
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteLivraison(row); });
    connect(pdfBtn, &QPushButton::clicked, [this, row]() { onExportPDF(row); });
    connect(trackBtn, &QPushButton::clicked, [this, row]() { onTrackDelivery(row); });

    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    layout->addWidget(pdfBtn);
    layout->addWidget(trackBtn);

    return widget;
}

void LivraisonWindow::onTrackDelivery(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();
    QString address = table->item(row, 2)->text(); // Destination est en colonne 2

    LivraisonTrackingDialog* dlg = new LivraisonTrackingDialog(QString::number(id), address, this);
    // Refresh table when done to see updated status
    connect(dlg, &QDialog::finished, this, [this]() { populateTable(searchInput->text()); });
    dlg->show();
}

void LivraisonWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void LivraisonWindow::onAddLivraison()
{
    AddLivraisonDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison data = dialog.getData(); 
        // ID is securely gathered from dialog input directly
        
        if (data.ajouter()) {
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Impossible d'ajouter la livraison.\n\nErreur Base de Données: " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onEditLivraison(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    // We need to fetch current data to pass to dialog
    QSqlQuery query;
    query.prepare("SELECT * FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", id);
    
    if (!query.exec() || !query.next()) return;

    ::Livraison currentData; // The struct used by dialog
    currentData.setID(id);
    currentData.setDate(query.value("DATELIVRAISON").toDate().toString("dd/MM/yyyy"));
    currentData.setAdresse(query.value("ADRESSELIVRAISON").toString());
    currentData.setStatut(query.value("STATUT").toString());
    currentData.setTransport(query.value("TYPETRANSPORT").toString());
    currentData.setVehicule(query.value("VEHICULE").toString());
    currentData.setPrix(query.value("PRIXLIVRAISON").toString());
    currentData.setDuree(query.value("DUREE").toInt());

    AddLivraisonDialog dialog(this, &currentData);
    if (dialog.exec() == QDialog::Accepted) {
        Livraison updatedData = dialog.getData();
        updatedData.setID(id);
        
        if (updatedData.modifier(id)) {
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Impossible de modifier la livraison.\n\nErreur Base de Données: " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onDeleteLivraison(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    if (showStyledActionDialog(this, "Suppression Livraison", 
                               "Voulez-vous vraiment supprimer cette livraison ? Cette action est irréversible.",
                               "#EF4444", "#B91C1C", "🗑️")) {
        if (livraisons.supprimer(id)) {
            showStyledActionDialog(this, "Succès", "La livraison a été supprimée.", "#10B981", "#059669", "✅", false);
            populateTable(searchInput->text());
        } else {
            QString errorMsg = "Echec de suppression : " + Livraison::getLastError();
            QMessageBox::critical(this, "Erreur", errorMsg);
        }
    }
}

void LivraisonWindow::onSort(int index)
{
    QString critere = "";
    QString ordre = "ASC";

    if (index == 1) { // Date (Récent)
        critere = "DATELIV"; ordre = "DESC";
    } else if (index == 2) { // Date (Ancien)
        critere = "DATELIV"; ordre = "ASC";
    } else if (index == 3) { // Prix ↑
        critere = "PRIX"; ordre = "ASC";
    } else if (index == 4) { // Prix ↓
        critere = "PRIX"; ordre = "DESC";
    }

    populateTable(searchInput->text(), critere, ordre);
}

void LivraisonWindow::onExportPDF(int row)
{
    if (row < 0 || row >= table->rowCount()) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", id);
    
    if (!query.exec() || !query.next()) return;

    Livraison liv;
    liv.setID(id);
    liv.setReference(query.value("REFERENCE").toString());
    liv.setDate(query.value("DATELIVRAISON").toDate().toString("dd/MM/yyyy"));
    liv.setAdresse(query.value("ADRESSELIVRAISON").toString());
    liv.setStatut(query.value("STATUT").toString());
    liv.setTransport(query.value("TYPETRANSPORT").toString());
    liv.setVehicule(query.value("VEHICULE").toString());
    liv.setPrix(query.value("PRIXLIVRAISON").toString());
    liv.setDuree(query.value("DUREE").toInt());

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", 
                                                    QString("Livraison_%1.pdf").arg(liv.getID()),
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15)); // Smaller margins for better fit

    QPainter painter(&writer);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = writer.width();
    int h = writer.height();

    // Header - Professional Blue Theme
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1E3A8A")); // Deep Blue
    painter.drawRect(0, 0, w, h * 0.12);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.drawText(QRect(0, h * 0.01, w, h * 0.03), Qt::AlignCenter, "PORTFLOW MANAGEMENT SYSTEM");

    painter.setFont(QFont("Segoe UI", 26, QFont::Bold));
    painter.drawText(QRect(0, h * 0.04, w, h * 0.06), Qt::AlignCenter, "REÇU DE LIVRAISON OFFICIEL");
    
    painter.setFont(QFont("Segoe UI", 11));
    painter.drawText(QRect(0, h * 0.09, w, h * 0.03), Qt::AlignCenter, "Logistique & Distribution Portuaire de Mahdia");

    // Content area
    int y = h * 0.16;
    auto drawField = [&](const QString& label, const QString& value, bool highlight = false) {
        painter.setPen(QColor("#1E40AF")); // Medium Blue for labels
        painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
        painter.drawText(QRect(0, y, w, h * 0.03), Qt::AlignCenter, label);
        
        y += h * 0.03;
        if (highlight) {
            painter.setPen(QColor("#1E3A8A"));
            painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
        } else {
            painter.setPen(Qt::black);
            painter.setFont(QFont("Segoe UI", 13, QFont::DemiBold));
        }
        painter.drawText(QRect(0, y, w, h * 0.04), Qt::AlignCenter, value);
        y += h * 0.05;
        
        painter.setPen(QPen(QColor("#3B82F6"), 1, Qt::DotLine)); // Blue dotted line
        painter.drawLine(w * 0.2, y - h * 0.01, w * 0.8, y - h * 0.01);
        y += h * 0.01;
    };

    drawField("RÉFÉRENCE DE LIVRAISON", liv.getReference());
    drawField("DATE D'EXPÉDITION", liv.getDate());
    drawField("ADRESSE DE DESTINATION", liv.getAdresse());
    drawField("VÉHICULE & TRANSPORT", liv.getTransport() + " | " + liv.getVehicule());
    
    // Price with big highlight
    double pVal = liv.getPrix().replace("DT", "").replace("$", "").replace("€", "").trimmed().toDouble();
    drawField("MONTANT TOTAL À RÉGLER", QString::number(pVal, 'f', 2) + " DT", true);

    // Signature section at the bottom
    y = h * 0.8;
    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(QRect(w * 0.6, y, w * 0.3, h * 0.05), Qt::AlignLeft, "SIGNATURE & CACHET");
    
    painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
    painter.drawLine(w * 0.6, y + h * 0.06, w * 0.9, y + h * 0.06);
    
    painter.setFont(QFont("Segoe UI", 8, QFont::StyleItalic));
    painter.setPen(QColor("#64748B"));
    painter.drawText(QRect(0, h * 0.95, w, h * 0.03), Qt::AlignCenter, "Généré automatiquement par PortFlow Management System");

    painter.end();

    QMessageBox::information(this, "Export PDF", "Le reçu de livraison a été exporté avec succès !");
}

void LivraisonWindow::onShowStatistics()
{
    QMap<QString, int> statusData;
    QMap<QString, int> vanTripsData;
    QMap<QString, QPair<int, int>> vanTimeMetrics; // vehicle -> {totalDuration, tripCount}

    QSqlQuery query("SELECT STATUT, VEHICULE, DUREE FROM LIVRAISONS");
    while (query.next()) {
        QString statut = query.value(0).toString();
        QString vehicule = query.value(1).toString();
        int duree = query.value(2).toInt();

        statusData[statut]++;
        
        QString van = vehicule.isEmpty() ? "Inconnu" : vehicule;
        vanTripsData[van]++;
        
        vanTimeMetrics[van].first += duree;
        vanTimeMetrics[van].second++;
    }

    QMap<QString, double> avgTimeData;
    for (auto it = vanTimeMetrics.begin(); it != vanTimeMetrics.end(); ++it) {
        if (it.value().second > 0) {
            avgTimeData[it.key()] = (double)it.value().first / it.value().second;
        }
    }

    LivraisonStatisticsDialog dialog(statusData, vanTripsData, avgTimeData, this);
    dialog.exec();
}

void LivraisonWindow::onExportAllPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter tout en PDF", 
                                                    "Rapport_Livraisons.pdf",
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));

    QPainter painter(&writer);
    painter.setRenderHint(QPainter::Antialiasing);

    // Professional Header
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1E3A8A"));
    painter.drawRect(0, 0, 9600, 800);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.drawText(QRect(400, 200, 8800, 400), Qt::AlignVCenter, "RAPPORT GLOBAL DES LIVRAISONS");

    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    int y = 1200;
    
    // Table Header Background
    painter.setBrush(QColor("#F1F5F9"));
    painter.drawRect(200, y - 100, 9200, 300);
    
    painter.setPen(QColor("#475569"));
    painter.drawText(400, y + 100, "RÉF");
    painter.drawText(1200, y + 100, "DATE");
    painter.drawText(2200, y + 100, "DESTINATION");
    painter.drawText(5500, y + 100, "STATUT");
    painter.drawText(7200, y + 100, "VÉHICULE");
    painter.drawText(8500, y + 100, "PRIX");
    
    y += 400;

    painter.setFont(QFont("Segoe UI", 9));
    QSqlQuery query("SELECT REFERENCE, DATELIVRAISON, ADRESSELIVRAISON, STATUT, VEHICULE, PRIXLIVRAISON FROM LIVRAISONS");
    while (query.next()) {
        if (y > 13000) { 
            writer.newPage();
            y = 500;
        }
        painter.setPen(Qt::black);
        painter.drawText(400, y, query.value(0).toString());
        painter.drawText(1200, y, query.value(1).toDate().toString("dd/MM/yyyy"));
        painter.drawText(2200, y, query.value(2).toString().left(40));
        painter.drawText(5500, y, query.value(3).toString());
        painter.drawText(7200, y, query.value(4).toString().left(15));
        
        double p = query.value(5).toDouble();
        painter.drawText(8500, y, QString::number(p, 'f', 2) + " DT");
        
        y += 300;
        painter.setPen(QPen(QColor("#F1F5F9"), 1));
        painter.drawLine(200, y - 150, 9400, y - 150);
    }

    painter.end();

    QMessageBox::information(this, "Export PDF", "Le rapport global a été exporté avec succès !");
}

void LivraisonWindow::onLiveUpdate()
{
    // Iterate through rows to update progress indicators and ETAs
    for (int i = 0; i < table->rowCount(); ++i) {
        // Avancement indicator is in column 7
        QWidget* widget = table->cellWidget(i, 7);
        LiveProgressIndicator* indicator = qobject_cast<LiveProgressIndicator*>(widget);
        
        if (indicator) {
            QDateTime start = indicator->property("startTime").toDateTime();
            double duration = indicator->property("duration").toDouble();
            
            if (start.isValid() && duration > 0) {
                double elapsed = start.secsTo(QDateTime::currentDateTime());
                double progress = elapsed / duration;
                indicator->setProgress(progress);
                
                // Real-time ETA update (Column 6)
                int id = table->item(i, 0)->data(Qt::UserRole).toInt();
                Livraison temp;
                temp.setID(id);
                if (table->item(i, 6)) {
                    table->item(i, 6)->setText(temp.getETA());
                }
                
                // If it just finished, forcefully update the database to ensure the UI refreshes to 'Arrivé'
                if (progress >= 1.0) {
                    QSqlQuery update;
                    update.prepare("UPDATE LIVRAISONS SET STATUT = 'Arrivé' WHERE IDLIVRAISON = :id");
                    update.bindValue(":id", id);
                    if (update.exec()) {
                        populateTable(searchInput->text());
                        return; // populateTable will refresh headers and widgets
                    }
                }
            }
        }
    }
}
