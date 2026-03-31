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
    QPushButton* smsBtn   = makeBtn("📱  Envoi SMS",     "#F59E0B", "#D97706");
    QPushButton* pdfBtn   = makeBtn("📄  Exporter PDF",  "#059669", "#047857");
    QPushButton* addBtn   = makeBtn("➕  Nouveau Frigo", "#2563EB", "#1D4ED8");

    connect(statsBtn, &QPushButton::clicked, this, &FrigoWindow::onShowStatistics);
    connect(pdfBtn,   &QPushButton::clicked, this, &FrigoWindow::onGeneratePDF);
    connect(addBtn, &QPushButton::clicked, this, &FrigoWindow::onAddFrigo);

    lay->addWidget(statsBtn);
    lay->addWidget(smsBtn);
    lay->addWidget(pdfBtn);
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
    searchInput->setFixedWidth(350);
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
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Référence", "Capacité", "Poisson", "Statut", "Date Rés.", "Température", "Occupation", "Actions"});

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

    table->setColumnWidth(0, 140);   // Référence
    table->setColumnWidth(1, 120);   // Capacité
    table->setColumnWidth(2, 120);   // Poisson
    table->setColumnWidth(3, 120);   // Statut
    table->setColumnWidth(4, 120);   // Date Rés.
    table->setColumnWidth(5, 120);   // Température
    table->setColumnWidth(6, 120);   // Occupation
}

void FrigoWindow::populateTable(const QString& filterText)
{
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

        addItem(0, model->record(i).value(1).toString(), true); // Reference
        addItem(1, model->record(i).value(2).toString() + " Kg"); // Capacité
        addItem(2, model->record(i).value(3).toString()); // Type Poisson
        
        // Statut Badge
        table->setCellWidget(r, 3, createStatusBadge(model->record(i).value(4).toString()));
        
        QVariant dat = model->record(i).value(5);
        QString dateStr = dat.type() == QVariant::Date || dat.type() == QVariant::DateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);
        addItem(4, dateStr); // Date Réservation
        
        addItem(5, model->record(i).value(6).toString() + " °C"); // Température
        addItem(6, model->record(i).value(7).toString() + " %"); // Occupation
        
        // Actions
        table->setCellWidget(r, 7, createActionButtons(r)); // Use current row index for actions
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
    
    QSqlQuery query("SELECT TYPE_POISSON, CAPACITE, STATUT FROM FRIGOS");
    while (query.next()) {
        QString type = query.value(0).toString();
        double cap = query.value(1).toDouble();
        QString stat = query.value(2).toString();
        
        typeCount[type]++;
        typeCapacity[type] += cap;
        statusCount[stat]++;
    }

    FrigoStatisticsDialog dlg(typeCount, typeCapacity, statusCount, this);
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
    query.prepare("SELECT REFERENCE, CAPACITE, TYPE_POISSON, STATUT, DATE_RESERVATION, TEMPERATURE, OCCUPATION FROM FRIGOS WHERE IDFRIGO = :id");
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
    QString date = dat.type() == QVariant::Date || dat.type() == QVariant::DateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);
    
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
                          newFrigoData.getDateRes(), newFrigoData.getTemp(), newFrigoData.getOcc());

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
        QString dateResStr = dat.type() == QVariant::Date || dat.type() == QVariant::DateTime ? dat.toDate().toString("dd/MM/yyyy") : dat.toString().left(10);

        FrigoModel current(
            query.value("IDFRIGO").toString(),
            query.value("REFERENCE").toString(),
            query.value("CAPACITE").toDouble(),
            query.value("TYPE_POISSON").toString(),
            query.value("STATUT").toString(),
            dateResStr,
            query.value("TEMPERATURE").toDouble(),
            query.value("OCCUPATION").toDouble()
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
    QString ref = item->text();

    if (QMessageBox::question(this, "Confirmation",
        QString("Voulez-vous vraiment supprimer le frigo '%1' ?").arg(ref)) == QMessageBox::Yes) {
        if (frigoModel.supprimer(id)) {
            populateTable();
            QMessageBox::information(this, "Succès", "Frigo supprimé.");
        } else {
            QMessageBox::critical(this, "Erreur", "La suppression a échoué.");
        }
    }
}
