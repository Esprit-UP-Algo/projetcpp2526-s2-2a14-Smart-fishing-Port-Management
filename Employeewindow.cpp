#include "employeewindow.h"
#include "employeedialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>
#include <QInputDialog>
#include <QTextEdit>
#include <QDate>
#include <QPrinter>
#include <QFileDialog>
#include <QScrollBar>
#include "EmployeeStatsWindow.h"

EmployeeWindow::EmployeeWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    populateTable();
}

EmployeeWindow::~EmployeeWindow()
{
}

void EmployeeWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion des Employés");
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

QFrame* EmployeeWindow::createSidebar()
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
        logoLabel->setText("👥");
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
    navLayout->addWidget(createNavButton("👥", "Employés", true));
    navLayout->addWidget(createNavButton("🧊", "Frigos"));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();

    // Quit button
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);

    return sidebar;
}

QPushButton* EmployeeWindow::createNavButton(const QString& icon, const QString& text, bool isActive, bool isLogout)
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
        connect(btn, &QPushButton::clicked, this, &EmployeeWindow::onLogout);
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

QWidget* EmployeeWindow::createContentArea()
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

    // Horizontal layout for Table and Actions
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(20);

    // Table card (Left side)
    QFrame* tableCard = createTableCard();
    bodyLayout->addWidget(tableCard, 8); // Increase to 80% space

    // Actions Panel (Right side)
    QFrame* actionPanel = createSideActionsPanel();
    bodyLayout->addWidget(actionPanel, 2); // Decrease to 20% space

    layout->addLayout(bodyLayout, 1);

    return content;
}

QFrame* EmployeeWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");

    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    /* Title + subtitle */
    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Employés");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Administration du personnel et suivi des rôles");
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

    QPushButton* addBtn   = makeBtn("➕  Nouvel Employé", "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &EmployeeWindow::onAddEmployee);

    lay->addWidget(addBtn);
    return hdr;
}

QFrame* EmployeeWindow::createToolbar()
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
    searchInput->setPlaceholderText("🔍  Rechercher un employé par nom, poste...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &EmployeeWindow::onSearch);
    lay->addWidget(searchInput);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Nom (A→Z)", "Nom (Z→A)", "Salaire ↑", "Salaire ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmployeeWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

QFrame* EmployeeWindow::createTableCard()
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

void EmployeeWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({"ID", "CIN", "Salaire", "Date Recrutement", "Statut", "Position", "Prénom", "Nom", "Actions"});

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

    table->setColumnWidth(0, 100); // ID
    table->setColumnWidth(1, 100); // CIN
    table->setColumnWidth(2, 100); // Salaire
    table->setColumnWidth(3, 140); // Date
    table->setColumnWidth(4, 110); // Statut
    table->setColumnWidth(5, 130); // Position
    table->setColumnWidth(6, 120); // Prénom
    table->setColumnWidth(7, 120); // Nom
}

void EmployeeWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);
    QFont cellFont("Segoe UI", 11);

    QSqlQueryModel* model;
    if (!filterText.isEmpty()) model = employeeModel.rechercher(filterText);
    else {
        int si = sortCombo ? sortCombo->currentIndex() : 0;
        if (si == 0) model = employeeModel.afficher();
        else {
            QString crit = "NOM", ord = "ASC";
            if (si == 1) { crit = "NOM"; ord = "ASC"; }
            else if (si == 2) { crit = "NOM"; ord = "DESC"; }
            else if (si == 3) { crit = "SALAIRE"; ord = "ASC"; }
            else if (si == 4) { crit = "SALAIRE"; ord = "DESC"; }
            model = employeeModel.trier(crit, ord);
        }
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 60);

        // Map from Model: ID(0), CIN(1), Salaire(2), Date(3), Statut(4), Position(5), Prenom(6), Nom(7)
        // Table Columns: ID(0), CIN(1), Sal(2), Date(3), Stat(4), Pos(5), Pre(6), Nom(7), Actions(8)
        
        QString idArr = model->record(i).value(0).toString();
        
        auto addItem = [&](int col, QString val, bool isBold = false) {
            QTableWidgetItem* item = new QTableWidgetItem(val);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFont(isBold ? QFont("Segoe UI", 11, QFont::Bold) : cellFont);
            if(isBold) item->setForeground(QBrush(QColor("#5D9CEC")));
            item->setData(Qt::UserRole, idArr); 
            table->setItem(r, col, item);
        };

        addItem(0, idArr, true); // ID
        addItem(1, model->record(i).value(1).toString()); // CIN
        
        QString sal = model->record(i).value(2).toString();
        if(!sal.isEmpty()) sal += " DT";
        addItem(2, sal); // Salaire
        
        addItem(3, model->record(i).value(3).toDate().toString("dd/MM/yyyy")); // Date
        
        // Statut Badge
        table->setCellWidget(r, 4, createStatusBadge(model->record(i).value(4).toString()));
        
        addItem(5, model->record(i).value(5).toString()); // Position
        addItem(6, model->record(i).value(6).toString()); // Prénom
        addItem(7, model->record(i).value(7).toString()); // Nom
        
        // Actions
        table->setCellWidget(r, 8, createActionButtons(i)); 
    }
    delete model;
}

QWidget* EmployeeWindow::createStatusBadge(const QString& status)
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
    if (status == "Actif") {
        styleSheet = R"(
            QLabel {
                background-color: #D1FAE5;
                color: #065F46;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else if (status == "Congé") {
        styleSheet = R"(
            QLabel {
                background-color: #FEF3C7;
                color: #92400E;
                border-radius: 8px;
                padding: 6px 16px;
            }
        )";
    } else { // Inactif
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

QWidget* EmployeeWindow::createActionButtons(int row)
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
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditEmployee(row); });
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
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteEmployee(row); });
    layout->addWidget(deleteBtn);

    return widget;
}

QFrame* EmployeeWindow::createSideActionsPanel()
{
    QFrame* panel = new QFrame();
    panel->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 20px;
            border: 1.5px solid #e2e8f0;
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 30, 20, 30);
    layout->setSpacing(20);

    QLabel* panelTitle = new QLabel("Administration");
    panelTitle->setFont(QFont("Segoe UI", 16, QFont::Bold));
    panelTitle->setStyleSheet("color: #1e3a5f; border: none;");
    layout->addWidget(panelTitle);

    auto makeLargeBtn = [&](const QString& icon, const QString& text, const QString& color) {
        QPushButton* btn = new QPushButton(icon + "  " + text);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(85);
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                color: white;
                border: none;
                border-radius: 12px;
                font-size: 13px;
                font-weight: bold;
                text-align: center;
                padding: 5px;
            }
            QPushButton:hover {
                background-color: %2;
            }
        )").arg(color, QColor(color).lighter(110).name()));
        return btn;
    };

    QPushButton* regBtn = makeLargeBtn("📜", "Règlement\nIntérieur", "#2B5EA6");
    QPushButton* leaveBtn = makeLargeBtn("📅", "Demande\nde Congé", "#059669");
    QPushButton* certBtn = makeLargeBtn("📄", "Attestation\nde Travail", "#7C3AED");
    QPushButton* statsBtn = makeLargeBtn("📊", "Statistiques\nEmployés", "#F59E0B"); // New button

    connect(regBtn, &QPushButton::clicked, this, &EmployeeWindow::onReglementInterieur);
    connect(leaveBtn, &QPushButton::clicked, this, &EmployeeWindow::onDemandeConge);
    connect(certBtn, &QPushButton::clicked, this, &EmployeeWindow::onAttestationTravail);
    connect(statsBtn, &QPushButton::clicked, this, &EmployeeWindow::onViewStats); // Connect new button

    layout->addWidget(regBtn);
    layout->addWidget(leaveBtn);
    layout->addWidget(certBtn);
    layout->addWidget(statsBtn); // Add new button to layout
    layout->addStretch();

    return panel;
}

void EmployeeWindow::onViewStats()
{
    EmployeeStatsWindow stats(this);
    stats.exec();
}

void EmployeeWindow::onReglementInterieur()
{
    QDialog* regDialog = new QDialog(this);
    regDialog->setWindowTitle("Règlement Intérieur du Port");
    regDialog->resize(800, 600);
    regDialog->setStyleSheet("background-color: white;");

    QVBoxLayout* layout = new QVBoxLayout(regDialog);
    
    QLabel* title = new QLabel("RÈGLEMENT INTÉRIEUR - PORTFLOW");
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #1e3a5f; margin-bottom: 20px;");
    layout->addWidget(title);

    QTextEdit* textDisplay = new QTextEdit();
    textDisplay->setReadOnly(true);
    textDisplay->setFont(QFont("Segoe UI", 11));
    textDisplay->setHtml(R"(
        <h1 style='color: #1e3a5f; text-align: center;'>📜 RÈGLEMENT INTÉRIEUR DU PORT – PORTFLOW</h1>
        
        <h2 style='color: #2b5ea6;'>Article 1 : Objet</h2>
        <p>Le présent règlement définit les règles de conduite, de sécurité et d’organisation applicables à tous les employés du port.</p>
        
        <h2 style='color: #2b5ea6;'>Article 2 : Horaires de travail</h2>
        <ul>
            <li>Les employés doivent respecter les horaires définis par l’administration.</li>
            <li>Tout retard doit être signalé au responsable hiérarchique.</li>
            <li>Les heures supplémentaires doivent être validées par la direction.</li>
        </ul>
        
        <h2 style='color: #2b5ea6;'>Article 3 : Sécurité</h2>
        <ul>
            <li>Le port des équipements de protection (casque, gilet réfléchissant, chaussures de sécurité) est obligatoire.</li>
            <li>L’accès aux zones techniques est limité au personnel autorisé.</li>
            <li>Tout incident doit être déclaré immédiatement.</li>
        </ul>
        
        <h2 style='color: #2b5ea6;'>Article 4 : Discipline</h2>
        <ul>
            <li>Le respect entre employés est obligatoire.</li>
            <li>Toute négligence mettant en danger la sécurité du port est sanctionnée.</li>
            <li>L’usage d’alcool ou de substances interdites est strictement prohibé.</li>
        </ul>
        
        <h2 style='color: #2b5ea6;'>Article 5 : Gestion des opérations</h2>
        <ul>
            <li>Les employés doivent suivre les procédures de chargement et déchargement.</li>
            <li>Les données saisies dans le système PortFlow doivent être exactes.</li>
            <li>Toute falsification de données est passible de sanction.</li>
        </ul>
        
        <h2 style='color: #2b5ea6;'>Article 6 : Sanctions</h2>
        <p>Le non-respect du présent règlement peut entraîner :</p>
        <ul>
            <li>Avertissement écrit</li>
            <li>Suspension temporaire</li>
            <li>Licenciement en cas de faute grave</li>
        </ul>
        
        <h2 style='color: #2b5ea6;'>Article 7 : Entrée en vigueur</h2>
        <p>Ce règlement prend effet à compter de sa date de publication.</p>
        
        <br><p style='text-align: right;'><i>Fait le )" + QDate::currentDate().toString("dd/MM/yyyy") + R"(<br>La Direction de PortFlow</i></p>
    )");
    layout->addWidget(textDisplay);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* printBtn = new QPushButton("📄 Exporter PDF");
    QPushButton* closeBtn = new QPushButton("Fermer");
    
    QString btnStyle = "QPushButton{ background:%1; color:white; border-radius:8px; padding:10px 20px; font-weight:bold; }";
    printBtn->setStyleSheet(btnStyle.arg("#059669"));
    closeBtn->setStyleSheet(btnStyle.arg("#6b7280"));

    connect(printBtn, &QPushButton::clicked, [this, regDialog, textDisplay]() {
        QString fileName = QFileDialog::getSaveFileName(regDialog, "Exporter en PDF", "", "PDF Files (*.pdf)");
        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".pdf")) fileName += ".pdf";
            
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setPageSize(QPageSize(QPageSize::A4));
            printer.setOutputFileName(fileName);
            
            textDisplay->document()->print(&printer);
            
            QMessageBox::information(regDialog, "Succès", "Le règlement a été exporté avec succès vers :\n" + fileName);
        }
    });
    connect(closeBtn, &QPushButton::clicked, regDialog, &QDialog::accept);

    btnLayout->addStretch();
    btnLayout->addWidget(printBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    regDialog->exec();
}

void EmployeeWindow::onDemandeConge()
{
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Sélection Requise", "Veuillez sélectionner un employé dans le tableau.");
        return;
    }

    // Récupérer les données depuis le tableau ou le vecteur
    // On peut utiliser le UserData stocké dans la première colonne
    QTableWidgetItem* item = table->item(currentRow, 0);
    QString empId = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", empId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer les données de l'employé.");
        return;
    }

    QString prenom = query.value("PRENOM").toString();
    QString nom = query.value("NOM").toString();
    QString poste = query.value("POSITION").toString();

    bool ok;
    int days = QInputDialog::getInt(this, "Durée du Congé", 
                                   "Nombre de jours pour " + prenom + " " + nom + " :",
                                   1, 1, 30, 1, &ok);

    if (ok) {
        QDialog* summary = new QDialog(this);
        summary->setWindowTitle("Demande de Congé Générée");
        summary->setFixedWidth(400);

        QVBoxLayout* lay = new QVBoxLayout(summary);
        QLabel* content = new QLabel(QString(
            "<h3>Demande de Congé</h3>"
            "<p><b>Employé :</b> %1 %2</p>"
            "<p><b>Poste :</b> %3</p>"
            "<p><b>Durée :</b> %4 jours</p>"
            "<hr>"
            "<p style='color: green;'><i>Demande générée avec succès le %5</i></p>"
        ).arg(prenom, nom, poste).arg(days).arg(QDate::currentDate().toString("dd/MM/yyyy")));
        
        lay->addWidget(content);
        
        QPushButton* okBtn = new QPushButton("Terminer");
        okBtn->setStyleSheet("background: #2B5EA6; color: white; padding: 8px; border-radius: 5px;");
        connect(okBtn, &QPushButton::clicked, summary, &QDialog::accept);
        lay->addWidget(okBtn);

        summary->exec();
    }
}

void EmployeeWindow::onAttestationTravail()
{
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Sélection Requise", "Veuillez sélectionner un employé dans le tableau.");
        return;
    }

    QTableWidgetItem* item = table->item(currentRow, 0);
    QString empId = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", empId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer les données de l'employé.");
        return;
    }

    QString prenom = query.value("PRENOM").toString();
    QString nom = query.value("NOM").toString();
    QString poste = query.value("POSITION").toString();
    QString dateEmbauche = query.value("DATE_RECRUTEMENT").toDate().toString("dd/MM/yyyy");

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter Attestation de Travail", 
                                                    "Attestation_" + nom + ".pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf")) fileName += ".pdf";

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    QString html = QString(R"(
        <div style='margin: 50px; font-family: sans-serif;'>
            <table width='100%'>
                <tr>
                    <td width='60%'>
                        <p><b>PORTFLOW – Administration Portuaire</b><br>
                        Adresse : Mahdia<br>
                        Téléphone : 93782135<br>
                        Email : portflow@gmail.com</p>
                    </td>
                    <td width='40%' align='right'>
                        <p>Mahdia, le %1</p>
                    </td>
                </tr>
            </table>

            <br><br><br>
            <h1 style='text-align: center; text-decoration: underline; font-size: 24px;'>ATTESTATION DE TRAVAIL</h1>
            <br><br>

            <p style='font-size: 16px; line-height: 1.5;'>
                Je soussigné(e), Directeur(trice) du Port, atteste que :
            </p>

            <table width='100%' style='margin-left: 50px; font-size: 16px; line-height: 2.0; border: none;'>
                <tr><td width='40%'><b>Nom et prénom :</b></td><td>%2 %3</td></tr>
                <tr><td><b>Poste :</b></td><td>%4</td></tr>
                <tr><td><b>Numéro d’employé :</b></td><td>%5</td></tr>
                <tr><td><b>Date d’embauche :</b></td><td>%6</td></tr>
            </table>

            <p style='font-size: 16px; line-height: 1.5;'>
                Travaille au sein de notre établissement depuis le %6 jusqu’à ce jour.
            </p>

            <p style='font-size: 16px; line-height: 1.5;'>
                La présente attestation est délivrée à l’intéressé(e) pour servir et valoir ce que de droit.
            </p>

            <br><br><br><br>
            <table width='100%' style='font-size: 16px;'>
                <tr>
                    <td width='50%'></td>
                    <td width='50%' align='center'>
                        <b>Signature</b><br>
                        <i>(Cachet de la Direction)</i>
                    </td>
                </tr>
            </table>
        </div>
    )").arg(QDate::currentDate().toString("dd/MM/yyyy"), 
            prenom, nom, poste, empId, dateEmbauche);

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Succès", "L'attestation de travail a été exportée avec succès.");
}

QString EmployeeWindow::generateEmployeeId()
{
    int maxId = 0;
    QSqlQuery query("SELECT ID_EMPLOYE FROM EMPLOYEES");
    while (query.next()) {
        QString idStr = query.value(0).toString();
        QString numStr = idStr.mid(3);
        int num = numStr.toInt();
        if (num > maxId) {
            maxId = num;
        }
    }
    query.finish();
    query.clear();
    return QString("EMP%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void EmployeeWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void EmployeeWindow::onAddEmployee()
{
    EmployeeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        EmployeeModel newEmp = dlg.getData();
        QString newId = generateEmployeeId();

        // Schema order: (id, cin, salaire, date_recrutement, statut, position, prenom, nom)
        EmployeeModel toSave(newId,
                             newEmp.getCin(),
                             newEmp.getSalaire(),
                             newEmp.getDate(),
                             newEmp.getStatut(),
                             newEmp.getPosition(),
                             newEmp.getPrenom(),
                             newEmp.getNom());

        if (toSave.ajouter()) {
            populateTable();
            QMessageBox::information(this, "Succès", "Employé ajouté avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter l'employé.\n\nErreur SQL: " + toSave.getLastError());
        }
    }
}

void EmployeeWindow::onEditEmployee(int row)
{
    if (row < 0) return;
    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;
    QString id = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        // Schema order: (id, cin, salaire, date_recrutement, statut, position, prenom, nom)
        EmployeeModel current(
            query.value("ID_EMPLOYE").toString(),
            query.value("CIN").toString(),
            query.value("SALAIRE").toDouble(),
            query.value("DATE_RECRUTEMENT").toDate(),
            query.value("STATUT").toString(),
            query.value("POSITION").toString(),
            query.value("PRENOM").toString(),
            query.value("NOM").toString()
        );

        EmployeeDialog dlg(this, &current);
        if (dlg.exec() == QDialog::Accepted) {
            EmployeeModel updated = dlg.getData();
            if (updated.modifier(id)) {
                populateTable();
                QMessageBox::information(this, "Succès", "Employé mis à jour.");
            } else {
                QMessageBox::critical(this, "Erreur", "Échec de la mise à jour.");
            }
        }
    }
}

void EmployeeWindow::onDeleteEmployee(int row)
{
    if (row < 0) return;
    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;
    QString id = item->data(Qt::UserRole).toString();
    QString name = item->text() + " " + table->item(row, 1)->text();

    if (QMessageBox::question(this, "Suppression", 
        QString("Voulez-vous vraiment supprimer l'employé %1 ?").arg(name)) == QMessageBox::Yes) {
        if (employeeModel.supprimer(id)) {
            populateTable();
            QMessageBox::information(this, "Succès", "Employé supprimé.");
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
        }
    }
}


void EmployeeWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter l'application ?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        this->close();
    }
}

void EmployeeWindow::onSort(int index)
{
    Q_UNUSED(index);
    populateTable(searchInput->text());
}

