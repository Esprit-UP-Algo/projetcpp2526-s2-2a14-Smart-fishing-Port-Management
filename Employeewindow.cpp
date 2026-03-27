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
#include <algorithm>

EmployeeWindow::EmployeeWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    loadEmployeesFromDb();

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
        QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }
    )");
    bar->setFixedHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    /* Search Input */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Rechercher un employé par nom, poste...");
    searchInput->setFont(QFont("Segoe UI", 11));
    searchInput->setFixedHeight(45);
    searchInput->setStyleSheet(R"(
        QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; }
        QLineEdit:focus{ border:2px solid #2563EB; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &EmployeeWindow::onSearch);
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

    /* Sort combo (Improved Premium Style) */
    sortCombo = new QComboBox();
    sortCombo->setFont(QFont("Segoe UI", 10));
    sortCombo->setFixedHeight(45);
    sortCombo->setMinimumWidth(200);
    sortCombo->addItems({"Défaut", "Nom (A→Z)", "Nom (Z→A)", "Salaire ↑", "Salaire ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox{ background:transparent; border:none; padding:4px 12px; color:#1f2937; font-weight:600; }
        QComboBox:hover { color:#2563EB; }
        QComboBox::drop-down{ border:none; width:30px; }
        QComboBox QAbstractItemView{
            background:white; border:1px solid #e2e8f0; border-radius:12px;
            selection-background-color:#eff6ff; selection-color:#2563EB; outline:none; padding:8px;
        }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmployeeWindow::onSort);
    lay->addWidget(sortCombo, 2);

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
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Prénom", "Nom", "CIN", "Position", "Salaire", "Date", "Statut", "Actions"});

    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(true);
    table->setAlternatingRowColors(false);

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

    table->setColumnWidth(0, 110);  // Prénom
    table->setColumnWidth(1, 110);  // Nom
    table->setColumnWidth(2, 100);  // CIN
    table->setColumnWidth(3, 110);  // Position
    table->setColumnWidth(4, 90);   // Salaire
    table->setColumnWidth(5, 100);  // Date
    table->setColumnWidth(6, 90);   // Statut
}

void EmployeeWindow::loadEmployeesFromDb()
{
    employees.clear();
    QSqlQuery query;
    bool success = query.exec("SELECT ID_EMPLOYE, PRENOM, NOM, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, CIN FROM EMPLOYEE");
    if (!success) {
        query.exec("SELECT ID_EMPLOYE, PRENOM, NOM, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, CIN FROM EMPLOYEES");
    }

    while (query.next()) {
        Employee e;
        e.id = query.value(0).toString();
        e.firstName = query.value(1).toString();
        e.lastName = query.value(2).toString();
        e.position = query.value(3).toString();

        e.salary = query.value(4).toString();
        if (!e.salary.endsWith("DT") && !e.salary.isEmpty()) {
            e.salary += " DT";
        }

        QVariant dateVar = query.value(5);
        if (dateVar.type() == QVariant::Date || dateVar.type() == QVariant::DateTime) {
            e.date = dateVar.toDate().toString("dd/MM/yyyy");
        } else {
            // Attempt to parse string or fallback
            QString dStr = dateVar.toString();
            QDate parsed = QDate::fromString(dStr, Qt::ISODate);
            if (parsed.isValid()) {
                e.date = parsed.toString("dd/MM/yyyy");
            } else {
                e.date = dStr;
            }
        }

        e.status = query.value(6).toString();
        e.cin = query.value(7).toString();
        employees.append(e);
    }
}

void EmployeeWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);

    QFont cellFont("Segoe UI", 11);

    for (int i = 0; i < employees.size(); ++i) {
        const Employee& emp = employees[i];

        // Filter
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!emp.id.toLower().contains(searchLower) &&
                !emp.firstName.toLower().contains(searchLower) &&
                !emp.lastName.toLower().contains(searchLower) &&
                !emp.position.toLower().contains(searchLower)) {
                continue;
            }
        }

        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 65);

        // Prénom
        QTableWidgetItem* prenomItem = new QTableWidgetItem(emp.firstName);
        prenomItem->setFont(cellFont);
        prenomItem->setData(Qt::UserRole, i); // Store row index for reference if needed
        table->setItem(row, 0, prenomItem);

        // Nom
        QTableWidgetItem* nomItem = new QTableWidgetItem(emp.lastName);
        nomItem->setFont(cellFont);
        table->setItem(row, 1, nomItem);

        // CIN
        QTableWidgetItem* cinItem = new QTableWidgetItem(emp.cin);
        cinItem->setFont(cellFont);
        table->setItem(row, 2, cinItem);

        // Position
        QTableWidgetItem* positionItem = new QTableWidgetItem(emp.position);
        positionItem->setFont(cellFont);
        table->setItem(row, 3, positionItem);

        // Salaire
        QTableWidgetItem* salaireItem = new QTableWidgetItem(emp.salary);
        salaireItem->setFont(cellFont);
        table->setItem(row, 4, salaireItem);

        // Date
        QTableWidgetItem* dateItem = new QTableWidgetItem(emp.date);
        dateItem->setFont(cellFont);
        table->setItem(row, 5, dateItem);

        // Statut
        table->setCellWidget(row, 6, createStatusBadge(emp.status));

        // Actions (Modifier et Supprimer)
        table->setCellWidget(row, 7, createActionButtons(i));
    }
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
    EmployeeStatsWindow stats(employees, this);
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
    int empIndex = item->data(Qt::UserRole).toInt();
    const Employee& emp = employees[empIndex];

    bool ok;
    int days = QInputDialog::getInt(this, "Durée du Congé",
                                    "Nombre de jours pour " + emp.firstName + " " + emp.lastName + " :",
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
                                         ).arg(emp.firstName, emp.lastName, emp.position).arg(days).arg(QDate::currentDate().toString("dd/MM/yyyy")));

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
    int empIndex = item->data(Qt::UserRole).toInt();
    const Employee& emp = employees[empIndex];

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter Attestation de Travail",
                                                    "Attestation_" + emp.lastName + ".pdf", "PDF Files (*.pdf)");
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
                            emp.firstName, emp.lastName, emp.position, emp.id, emp.date);

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Succès", "L'attestation de travail a été exportée avec succès.");
}

QString EmployeeWindow::generateEmployeeId()
{
    int maxId = 0;
    for (const Employee& e : employees) {
        QString numStr = e.id.mid(3);
        int num = numStr.toInt();
        if (num > maxId) {
            maxId = num;
        }
    }
    return QString("EMP%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void EmployeeWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void EmployeeWindow::onAddEmployee()
{
    EmployeeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Employee newEmployee = dialog.getData();
        newEmployee.id = generateEmployeeId();

        QSqlQuery query;
        query.prepare("INSERT INTO EMPLOYEE (ID_EMPLOYE, PRENOM, NOM, CIN, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT) VALUES (:id, :pre, :nom, :cin, :pos, :sal, TO_DATE(:date, 'DD/MM/YYYY'), :stat)");
        query.bindValue(":id", newEmployee.id);
        query.bindValue(":pre", newEmployee.firstName);
        query.bindValue(":nom", newEmployee.lastName);
        query.bindValue(":cin", newEmployee.cin);
        query.bindValue(":pos", newEmployee.position);
        query.bindValue(":sal", newEmployee.salary);
        query.bindValue(":date", newEmployee.date);
        query.bindValue(":stat", newEmployee.status);
        if(!query.exec()) {
            // fallback
            query.prepare("INSERT INTO EMPLOYEES (ID_EMPLOYE, PRENOM, NOM, CIN, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT) VALUES (:id, :pre, :nom, :cin, :pos, :sal, :date, :stat)");
            query.bindValue(":id", newEmployee.id);
            query.bindValue(":pre", newEmployee.firstName);
            query.bindValue(":nom", newEmployee.lastName);
            query.bindValue(":cin", newEmployee.cin);
            query.bindValue(":pos", newEmployee.position);
            query.bindValue(":sal", newEmployee.salary);
            query.bindValue(":date", newEmployee.date);
            query.bindValue(":stat", newEmployee.status);
            query.exec();
        }

        employees.append(newEmployee);
        populateTable(searchInput->text());
        qDebug() << "Employé ajouté:" << newEmployee.firstName << newEmployee.lastName;
    }
}

void EmployeeWindow::onEditEmployee(int row)
{
    if (row < 0 || row >= employees.size()) return;

    EmployeeDialog dialog(this, &employees[row]);
    if (dialog.exec() == QDialog::Accepted) {
        Employee updatedEmployee = dialog.getData();
        updatedEmployee.id = employees[row].id;

        QSqlQuery query;
        query.prepare("UPDATE EMPLOYEE SET PRENOM=:pre, NOM=:nom, CIN=:cin, \"POSITION\"=:pos, SALAIRE=:sal, DATE_RECRUTEMENT=TO_DATE(:date, 'DD/MM/YYYY'), STATUT=:stat WHERE ID_EMPLOYE=:id");
        query.bindValue(":id", updatedEmployee.id);
        query.bindValue(":pre", updatedEmployee.firstName);
        query.bindValue(":nom", updatedEmployee.lastName);
        query.bindValue(":cin", updatedEmployee.cin);
        query.bindValue(":pos", updatedEmployee.position);
        query.bindValue(":sal", updatedEmployee.salary);
        query.bindValue(":date", updatedEmployee.date);
        query.bindValue(":stat", updatedEmployee.status);
        if(!query.exec()) {
            // fallback
            query.prepare("UPDATE EMPLOYEES SET PRENOM=:pre, NOM=:nom, CIN=:cin, \"POSITION\"=:pos, SALAIRE=:sal, DATE_RECRUTEMENT=:date, STATUT=:stat WHERE ID_EMPLOYE=:id");
            query.bindValue(":id", updatedEmployee.id);
            query.bindValue(":pre", updatedEmployee.firstName);
            query.bindValue(":nom", updatedEmployee.lastName);
            query.bindValue(":cin", updatedEmployee.cin);
            query.bindValue(":pos", updatedEmployee.position);
            query.bindValue(":sal", updatedEmployee.salary);
            query.bindValue(":date", updatedEmployee.date);
            query.bindValue(":stat", updatedEmployee.status);
            query.exec();
        }

        employees[row] = updatedEmployee;
        populateTable(searchInput->text());
        qDebug() << "Employé modifié:" << updatedEmployee.firstName << updatedEmployee.lastName;
    }
}

void EmployeeWindow::onDeleteEmployee(int row)
{
    if (row < 0 || row >= employees.size()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir supprimer l'employé '" + employees[row].firstName + " " + employees[row].lastName + "' ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString idToDelete = employees[row].id;
        QSqlQuery query;
        query.prepare("DELETE FROM EMPLOYEE WHERE ID_EMPLOYE = :id");
        query.bindValue(":id", idToDelete);
        if(!query.exec()) {
            query.prepare("DELETE FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
            query.bindValue(":id", idToDelete);
            query.exec();
        }

        qDebug() << "Employé supprimé:" << employees[row].firstName << employees[row].lastName;
        employees.removeAt(row);
        populateTable(searchInput->text());
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
    if (index == 0) return; // Défaut (ordre d'ajout)

    // Helper to extract salary as number
    auto getSalaryValue = [](const QString& s) {
        QString clean = s;
        clean.remove("DT").remove(" ").remove(",");
        return clean.toDouble();
    };

    if (index == 1) { // Nom (A→Z)
        std::sort(employees.begin(), employees.end(), [](const Employee& a, const Employee& b) {
            if (a.lastName != b.lastName) return a.lastName < b.lastName;
            return a.firstName < b.firstName;
        });
    } else if (index == 2) { // Nom (Z→A)
        std::sort(employees.begin(), employees.end(), [](const Employee& a, const Employee& b) {
            if (a.lastName != b.lastName) return a.lastName > b.lastName;
            return a.firstName > b.firstName;
        });
    } else if (index == 3) { // Salaire ↑
        std::sort(employees.begin(), employees.end(), [&](const Employee& a, const Employee& b) {
            return getSalaryValue(a.salary) < getSalaryValue(b.salary);
        });
    } else if (index == 4) { // Salaire ↓
        std::sort(employees.begin(), employees.end(), [&](const Employee& a, const Employee& b) {
            return getSalaryValue(a.salary) > getSalaryValue(b.salary);
        });
    }

    populateTable(searchInput->text());
}
