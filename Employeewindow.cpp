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
#include <QMenu>
#include <QAction>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>

EmployeeWindow::EmployeeWindow(QWidget *parent)
    : QMainWindow(parent)
{
    isDarkMode = false;
    setupUi();

    loadEmployeesFromDb();

    populateTable();
    applyTheme();
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
    sidebar = createSidebar();
    mainLayout->addWidget(sidebar);

    // Content area
    contentArea = createContentArea();
    mainLayout->addWidget(contentArea, 1);
}

QFrame* EmployeeWindow::createSidebar()
{
    QFrame* side = new QFrame();
    side->setFixedWidth(280);
    side->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #2B5EA6,
                stop:1 #5D9CEC
            );
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(side);
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
    
    QPushButton* settingsBtn = createNavButton("⚙️", "Paramètres");
    connect(settingsBtn, &QPushButton::clicked, this, &EmployeeWindow::onSettingsClicked);
    navLayout->addWidget(settingsBtn);

    navLayout->addStretch();

    // Quit button
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);

    return side;
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
    QWidget* area = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(area);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    // Header
    header = createHeader();
    layout->addWidget(header);

    // Toolbar
    toolbar = createToolbar();
    layout->addWidget(toolbar);

    // Table card (Full width)
    tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return area;
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
    titleLabel = new QLabel("Gestion des Employés");
    titleLabel->setFont(QFont("Segoe UI", 26, QFont::Bold));
    subtitleLabel = new QLabel("Administration du personnel et suivi des rôles");
    subtitleLabel->setFont(QFont("Segoe UI", 10));
    titleCol->addWidget(titleLabel);
    titleCol->addWidget(subtitleLabel);
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
    searchInput->setFixedHeight(40);
    searchInput->setFixedWidth(280);
    searchInput->setStyleSheet(R"(
        QLineEdit{ background:#ffffff; border:2px solid #e2e8f0; border-radius:12px; padding:4px 16px; color:#1f2937; }
        QLineEdit:focus{ border:2px solid #2563EB; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &EmployeeWindow::onSearch);
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

    QPushButton* adminBtn = makeToolBtn("📜  Administration", "#2B5EA6", "#1D4ED8");
    adminMenu = new QMenu(this);
    
    adminMenu->setStyleSheet(R"(
        QMenu {
            background-color: white; border: 1.5px solid #d1d5db; border-radius: 14px; padding: 10px 0px;
        }
        QMenu::item {
            padding: 12px 30px; font-family: 'Segoe UI'; font-size: 13px; font-weight: 500; color: #1e3a5f; border-bottom: 0.5px solid #f1f5f9;
        }
        QMenu::item:selected {
            background-color: #eff6ff; color: #2563EB;
        }
        QMenu::item:last { border-bottom: none; }
    )");

    QAction* reglementAct = new QAction("📜  Règlement Intérieur", this);
    QAction* congeAct = new QAction("📅  Demande de Congé", this);
    QAction* attestationAct = new QAction("📄  Attestation de Travail", this);
    QAction* presenceAct = new QAction("📋  Rapport Présence", this);
    QAction* registerFaceAct = new QAction("📸  Register Face ID", this);
    
    adminMenu->addAction(reglementAct);
    adminMenu->addAction(congeAct);
    adminMenu->addAction(attestationAct);
    adminMenu->addSeparator();
    adminMenu->addAction(presenceAct);
    adminMenu->addSeparator();
    adminMenu->addAction(registerFaceAct);
    
    adminBtn->setMenu(adminMenu);
    
    connect(reglementAct, &QAction::triggered, this, &EmployeeWindow::onReglementInterieur);
    connect(congeAct, &QAction::triggered, this, &EmployeeWindow::onDemandeConge);
    connect(attestationAct, &QAction::triggered, this, &EmployeeWindow::onAttestationTravail);
    connect(presenceAct, &QAction::triggered, this, &EmployeeWindow::onPresenceReport);
    connect(registerFaceAct, &QAction::triggered, this, &EmployeeWindow::onRegisterFaceID);
    
    QPushButton* statsBtn = makeToolBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    connect(statsBtn, &QPushButton::clicked, this, &EmployeeWindow::onViewStats);

    QPushButton* aiBtn = makeToolBtn("✨  Rapport IA", "#8B5CF6", "#6D28D9");
    connect(aiBtn, &QPushButton::clicked, this, &EmployeeWindow::onSmartAIReport);

    lay->addWidget(adminBtn);
    lay->addWidget(statsBtn);
    lay->addWidget(aiBtn);

    lay->addStretch();

    /* Divider */
    QFrame* div = new QFrame(); div->setFrameShape(QFrame::VLine);
    div->setStyleSheet("color:#e2e8f0;"); div->setFixedWidth(1);
    lay->addWidget(div);

    /* Sort label */
    sortLabel = new QLabel("Trier par :");
    sortLabel->setFont(QFont("Segoe UI", 10, QFont::Medium));
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
    tableContainer = new QFrame();
    
    QVBoxLayout* containerLayout = new QVBoxLayout(tableContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    setupTable();
    containerLayout->addWidget(table);

    layout->addWidget(tableContainer);

    return card;
}

void EmployeeWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({"Prénom", "Nom", "CIN", "Téléphone", "Email", "Position", "Salaire", "Date", "Statut", "Actions"});

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
    table->setColumnWidth(3, 100);  // Phone
    table->setColumnWidth(4, 150);  // Email
    table->setColumnWidth(5, 110);  // Position
    table->setColumnWidth(6, 90);   // Salaire
    table->setColumnWidth(7, 100);  // Date
    table->setColumnWidth(8, 90);   // Statut
}

void EmployeeWindow::loadEmployeesFromDb()
{
    employees.clear();
    QSqlQuery query;
    bool success = query.exec("SELECT ID_EMPLOYE, PRENOM, NOM, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, CIN, TELEPHONE, EMAIL, RFID_UID FROM EMPLOYEES");
    if (!success) {
        query.exec("SELECT ID_EMPLOYE, PRENOM, NOM, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, CIN, TELEPHONE, EMAIL, RFID_UID FROM EMPLOYEE");
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
        if (dateVar.typeId() == QMetaType::QDate || dateVar.typeId() == QMetaType::QDateTime) {
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
        e.phone = query.value(8).toString();
        e.email = query.value(9).toString();
        e.rfid_uid = query.value(10).toString();
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
                !emp.position.toLower().contains(searchLower) &&
                !emp.phone.toLower().contains(searchLower) &&
                !emp.email.toLower().contains(searchLower)) {
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

        // Téléphone
        QTableWidgetItem* phoneItem = new QTableWidgetItem(emp.phone);
        phoneItem->setFont(cellFont);
        table->setItem(row, 3, phoneItem);

        // Email
        QTableWidgetItem* emailItem = new QTableWidgetItem(emp.email);
        emailItem->setFont(cellFont);
        table->setItem(row, 4, emailItem);

        // Position
        QTableWidgetItem* positionItem = new QTableWidgetItem(emp.position);
        positionItem->setFont(cellFont);
        table->setItem(row, 5, positionItem);

        // Salaire
        QTableWidgetItem* salaireItem = new QTableWidgetItem(emp.salary);
        salaireItem->setFont(cellFont);
        table->setItem(row, 6, salaireItem);

        // Date
        QTableWidgetItem* dateItem = new QTableWidgetItem(emp.date);
        dateItem->setFont(cellFont);
        table->setItem(row, 7, dateItem);

        // Statut
        table->setCellWidget(row, 8, createStatusBadge(emp.status));

        // Actions (Modifier et Supprimer)
        table->setCellWidget(row, 9, createActionButtons(i));
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
    // Keeping this function for compatibility but it's not used in the final full-width layout
    QFrame* panel = new QFrame();
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
        QString numStr = e.id;
        numStr.remove(QRegularExpression("[^0-9]"));
        int num = numStr.toInt();
        if (num > maxId) {
            maxId = num;
        }
    }
    return QString::number(maxId + 1);
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

        QSqlQuery query1;
        query1.prepare("INSERT INTO EMPLOYEES (ID_EMPLOYE, PRENOM, NOM, CIN, TELEPHONE, EMAIL, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, RFID_UID) VALUES (:id, :pre, :nom, :cin, :phone, :email, :pos, :sal, :date, :stat, :rfid)");
        query1.bindValue(":id", newEmployee.id.toInt());
        query1.bindValue(":pre", newEmployee.firstName);
        query1.bindValue(":nom", newEmployee.lastName);
        query1.bindValue(":cin", newEmployee.cin);
        query1.bindValue(":phone", newEmployee.phone);
        query1.bindValue(":email", newEmployee.email);
        query1.bindValue(":pos", newEmployee.position);
        
        QString cleanSal = newEmployee.salary;
        cleanSal.remove("DT", Qt::CaseInsensitive).remove(" ").remove(",");
        query1.bindValue(":sal", cleanSal.toDouble());

        query1.bindValue(":date", QDate::fromString(newEmployee.date, "dd/MM/yyyy"));
        query1.bindValue(":stat", newEmployee.status);
        query1.bindValue(":rfid", newEmployee.rfid_uid);
        
        if(!query1.exec()) {
            QString err1 = query1.lastError().text();
            QSqlQuery query2;
            query2.prepare("INSERT INTO EMPLOYEE (ID_EMPLOYE, PRENOM, NOM, CIN, TELEPHONE, EMAIL, \"POSITION\", SALAIRE, DATE_RECRUTEMENT, STATUT, RFID_UID) VALUES (:id, :pre, :nom, :cin, :phone, :email, :pos, :sal, :date, :stat, :rfid)");
            query2.bindValue(":id", newEmployee.id.toInt());
            query2.bindValue(":pre", newEmployee.firstName);
            query2.bindValue(":nom", newEmployee.lastName);
            query2.bindValue(":cin", newEmployee.cin);
            query2.bindValue(":phone", newEmployee.phone);
            query2.bindValue(":email", newEmployee.email);
            query2.bindValue(":pos", newEmployee.position);
            query2.bindValue(":sal", cleanSal.toDouble());
            query2.bindValue(":date", QDate::fromString(newEmployee.date, "dd/MM/yyyy"));
            query2.bindValue(":stat", newEmployee.status);
            query2.bindValue(":rfid", newEmployee.rfid_uid);
            
            if (!query2.exec()) {
                QMessageBox::critical(this, "Erreur Base de données", 
                    "Erreur EMPLOYEES :\n" + err1 + "\n\nErreur EMPLOYEE:\n" + query2.lastError().text());
                return;
            }
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

        QSqlQuery query1;
        query1.prepare("UPDATE EMPLOYEES SET PRENOM=:pre, NOM=:nom, CIN=:cin, TELEPHONE=:phone, EMAIL=:email, \"POSITION\"=:pos, SALAIRE=:sal, DATE_RECRUTEMENT=:date, STATUT=:stat, RFID_UID=:rfid WHERE ID_EMPLOYE=:id");
        query1.bindValue(":id", updatedEmployee.id.toInt());
        query1.bindValue(":pre", updatedEmployee.firstName);
        query1.bindValue(":nom", updatedEmployee.lastName);
        query1.bindValue(":cin", updatedEmployee.cin);
        query1.bindValue(":phone", updatedEmployee.phone);
        query1.bindValue(":email", updatedEmployee.email);
        query1.bindValue(":pos", updatedEmployee.position);
        
        QString cleanSalUpd = updatedEmployee.salary;
        cleanSalUpd.remove("DT", Qt::CaseInsensitive).remove(" ").remove(",");
        query1.bindValue(":sal", cleanSalUpd.toDouble());

        query1.bindValue(":date", QDate::fromString(updatedEmployee.date, "dd/MM/yyyy"));
        query1.bindValue(":stat", updatedEmployee.status);
        query1.bindValue(":rfid", updatedEmployee.rfid_uid);
        
        if(!query1.exec()) {
            QString err1 = query1.lastError().text();
            QSqlQuery query2;
            query2.prepare("UPDATE EMPLOYEE SET PRENOM=:pre, NOM=:nom, CIN=:cin, TELEPHONE=:phone, EMAIL=:email, \"POSITION\"=:pos, SALAIRE=:sal, DATE_RECRUTEMENT=:date, STATUT=:stat, RFID_UID=:rfid WHERE ID_EMPLOYE=:id");
            query2.bindValue(":id", updatedEmployee.id.toInt());
            query2.bindValue(":pre", updatedEmployee.firstName);
            query2.bindValue(":nom", updatedEmployee.lastName);
            query2.bindValue(":cin", updatedEmployee.cin);
            query2.bindValue(":phone", updatedEmployee.phone);
            query2.bindValue(":email", updatedEmployee.email);
            query2.bindValue(":pos", updatedEmployee.position);
            query2.bindValue(":sal", cleanSalUpd.toDouble());
            query2.bindValue(":date", QDate::fromString(updatedEmployee.date, "dd/MM/yyyy"));
            query2.bindValue(":stat", updatedEmployee.status);
            query2.bindValue(":rfid", updatedEmployee.rfid_uid);
            
            if (!query2.exec()) {
                QMessageBox::critical(this, "Erreur Base de données", 
                    "Erreur EMPLOYEES :\n" + err1 + "\n\nErreur EMPLOYEE:\n" + query2.lastError().text());
                return;
            }
        }
        
        employees[row] = updatedEmployee;
        populateTable(searchInput->text());
        qDebug() << "Employé modifié:" << updatedEmployee.firstName << updatedEmployee.lastName;
    }
}

void EmployeeWindow::onDeleteEmployee(int row)
{
    if (row < 0 || row >= employees.size()) return;

    if (showStyledActionDialog(this, "Suppression Employé",
                               QString("Voulez-vous vraiment supprimer l'employé <b>%1 %2</b> ?").arg(employees[row].firstName, employees[row].lastName),
                               "#EF4444", "#B91C1C", "🗑️")) {
        
        QString idToDelete = employees[row].id;
        QSqlQuery query;
        query.prepare("DELETE FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
        query.bindValue(":id", idToDelete.toInt());
        
        if(!query.exec()) {
            // Affichage de l'erreur via le dialogue stylisé
            showStyledActionDialog(this, "Erreur Base de données", 
                                 "Impossible de supprimer l'employé car il est lié à des bateaux ou livraisons.<br><br><b>Détail:</b> " + query.lastError().text(),
                                 "#991B1B", "#EF4444", "!", false, "Compris");
            return;
        }

        qDebug() << "Employé supprimé:" << employees[row].firstName << employees[row].lastName;
        employees.removeAt(row);
        populateTable(searchInput->text());
        
        showStyledActionDialog(this, "Succès", "L'employé a été supprimé avec succès.", "#10B981", "#059669", "✅", false);
    }
}

void EmployeeWindow::onLogout()
{
    if (showStyledActionDialog(this, "Quitter", "Voulez-vous vraiment fermer la gestion des employés ?", "#1E3A5F", "#3B82F6", "🚪", true, "Oui, Quitter", "Annuler")) {
        this->close();
    }
}

void EmployeeWindow::onSort(int index)
{
    if (index == 0) return;

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

void EmployeeWindow::toggleDarkMode()
{
    isDarkMode = !isDarkMode;
    applyTheme();
}

void EmployeeWindow::onSettingsClicked()
{
    QDialog* settingsDlg = new QDialog(this);
    settingsDlg->setWindowTitle("Paramètres - PortFlow");
    settingsDlg->setFixedSize(350, 180);
    settingsDlg->setStyleSheet(isDarkMode ? "background-color: #2D3748; color: white;" : "background-color: #F0F4F8; color: #1e3a5f;");

    QVBoxLayout* layout = new QVBoxLayout(settingsDlg);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel("Préférences");
    title->setFont(QFont("Segoe UI", 16, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QPushButton* darkBtnDlg = new QPushButton(isDarkMode ? "☀️  Mode: Clair" : "🌙  Mode: Sombre");

    auto btnStyle = R"(
        QPushButton {
            background-color: #2B5EA6;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 15px;
            font-weight: bold;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #1e3a5f;
        }
    )";
    darkBtnDlg->setStyleSheet(btnStyle);

    connect(darkBtnDlg, &QPushButton::clicked, [this, darkBtnDlg, settingsDlg]() {
        toggleDarkMode();
        darkBtnDlg->setText(isDarkMode ? "☀️  Mode: Clair" : "🌙  Mode: Sombre");
        settingsDlg->setStyleSheet(isDarkMode ? "background-color: #2D3748; color: white;" : "background-color: #F0F4F8; color: #1e3a5f;");
    });

    layout->addWidget(darkBtnDlg);

    QPushButton* closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("background-color: #6c757d; color: white; border-radius: 10px; padding: 10px; font-weight: bold; margin-top: 10px;");
    connect(closeBtn, &QPushButton::clicked, settingsDlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    settingsDlg->exec();
}

void EmployeeWindow::applyTheme()
{
    QString bgColor = isDarkMode ? "#1A202C" : "#F0F4F8";
    QString containerBg = isDarkMode ? "#2D3748" : "white";
    QString textPrimary = isDarkMode ? "#F7FAFC" : "#1e3a5f";
    QString textSecondary = isDarkMode ? "#A0AEC0" : "#6b7280";
    QString borderCol = isDarkMode ? "#4A5568" : "#e2e8f0";
    QString tableGrid = isDarkMode ? "#4A5568" : "#d1d5db";
    QString sidebarBgStr = isDarkMode ? "QFrame { border-right: 1px solid #4A5568; background-color: #2D3748; }" : "QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2B5EA6, stop:1 #5D9CEC); }";

    setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(bgColor));
    contentArea->setStyleSheet(QString("background-color: %1;").arg(bgColor));
    
    sidebar->setStyleSheet(sidebarBgStr);

    toolbar->setStyleSheet(QString("QFrame { background: %1; border-radius: 14px; border: 1.5px solid %2; }").arg(containerBg, borderCol));
    
    sortLabel->setStyleSheet(QString("color: %1; margin-left: 10px;").arg(textSecondary));

    sortCombo->setStyleSheet(QString(R"(
        QComboBox{ background:transparent; border:none; padding:4px 12px; color:%1; font-weight:600; }
        QComboBox:hover { color:#3B82F6; }
        QComboBox::drop-down{ border:none; width:30px; }
        QComboBox QAbstractItemView{
            background:%2; border:1px solid %3; border-radius:12px;
            selection-background-color:#4A5568; selection-color:white; outline:none; padding:8px;
            color:%1;
        }
    )").arg(textPrimary, containerBg, borderCol));
    
    searchInput->setStyleSheet(QString(R"(
        QLineEdit{ background:%1; border:2px solid %2; border-radius:12px; padding:4px 16px; color:%3; }
        QLineEdit:focus{ border:2px solid #3B82F6; background:%1; }
    )").arg(containerBg, borderCol, textPrimary));
    
    titleLabel->setStyleSheet(QString("color: %1;").arg(textPrimary));
    subtitleLabel->setStyleSheet(QString("color: %1;").arg(textSecondary));
    
    tableCard->setStyleSheet(QString("QFrame { background-color: #3B82F6; border-radius: 20px; padding: 3px; }"));
    tableContainer->setStyleSheet(QString("QFrame { background-color: %1; border-radius: 17px; }").arg(containerBg));
    
    table->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 16px;
            gridline-color: %2;
        }
        QTableWidget::item {
            padding: 12px;
            border-right: 1px solid %2;
            border-bottom: 1px solid %2;
            color: %3;
            background-color: %1;
            font-family: 'Segoe UI';
            font-size: 11pt;
        }
        QTableWidget::item:selected {
            background-color: #2B6CB0;
            color: white;
        }
        QHeaderView::section {
            background-color: %2;
            color: %3;
            padding: 12px;
            border: none;
            font-weight: 600;
            font-family: 'Segoe UI';
            font-size: 11pt;
        }
        QHeaderView::section:first { border-top-left-radius: 14px; }
        QHeaderView::section:last { border-top-right-radius: 14px; }
    )").arg(containerBg, tableGrid, textPrimary));
    
    if (adminMenu) {
        adminMenu->setStyleSheet(QString(R"(
            QMenu {
                background-color: %1;
                border: 1.5px solid %2;
                border-radius: 14px;
                padding: 10px 0px;
            }
            QMenu::item {
                padding: 12px 30px;
                font-family: 'Segoe UI';
                font-size: 13px;
                font-weight: 500;
                color: %3;
                border-bottom: 0.5px solid %4;
            }
            QMenu::item:selected {
                background-color: %5;
                color: #2563EB;
            }
            QMenu::item:last {
                border-bottom: none;
            }
        )").arg(containerBg, borderCol, textPrimary, borderCol, (isDarkMode ? "#374151" : "#eff6ff")));
    }
    
    populateTable(searchInput->text());
}
void EmployeeWindow::onRegisterFaceID()
{
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select an employee to register their face.");
        return;
    }

    QTableWidgetItem* item = table->item(currentRow, 0);
    int empIndex = item->data(Qt::UserRole).toInt();
    const Employee& emp = employees[empIndex];

    QMessageBox::information(this, "Face Registration", 
        "Starting face registration for: " + emp.firstName + " " + emp.lastName + 
        "\n\nPlease look directly at the camera. The system will automatically scan and register your face securely in the background.");

    QProcess *process = new QProcess(this);
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    process->setProcessEnvironment(env);
    
    // Robust search for the python script
    QString scriptPath = "face_id/face_auth.py";
    QDir dir(QCoreApplication::applicationDirPath());
    bool found = false;
    while (!dir.isRoot()) {
        if (dir.exists(scriptPath)) {
            scriptPath = dir.absoluteFilePath(scriptPath);
            found = true;
            break;
        }
        dir.cdUp();
    }
    if (!found) {
        scriptPath = "C:/Users/manne/Downloads/projetcpp2526-s2-2a14-Smart-fishing-Port-Management-gestion-des-peches-v2/projetcpp2526-s2-2a14-Smart-fishing-Port-Management-gestion-des-peches-v2/face_id/face_auth.py";
    }

    QStringList arguments;
    arguments << scriptPath << "register" << (emp.firstName + "_" + emp.lastName) << emp.position;

    QString* outputBuffer = new QString();
    QString* errorBuffer = new QString();

    connect(process, &QProcess::readyReadStandardOutput, this, [process, outputBuffer]() {
        *outputBuffer += process->readAllStandardOutput();
    });

    connect(process, &QProcess::readyReadStandardError, this, [process, errorBuffer]() {
        *errorBuffer += process->readAllStandardError();
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process, emp, outputBuffer, errorBuffer](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
        qDebug() << "Python STDOUT:" << *outputBuffer;
        qDebug() << "Python STDERR:" << *errorBuffer;

        if (outputBuffer->contains("SUCCESS")) {
            QMessageBox::information(this, "Success", "Face registered successfully for " + emp.firstName);
        } else {
            QString fullError = *outputBuffer + "\n" + *errorBuffer;
            QMessageBox::warning(this, "Failed", "Registration failed or cancelled.\n\nDetails:\n" + (fullError.trimmed().isEmpty() ? "No face found after 20 seconds." : fullError));
        }
        
        delete outputBuffer;
        delete errorBuffer;
        process->deleteLater();
    });

    process->start("python", arguments);
    
    if (!process->waitForStarted(500)) {
        process->start("py", arguments);
        if (!process->waitForStarted(500)) {
            QString userProfile = QDir::homePath();
            QString fallbackPath = userProfile + "/AppData/Local/Programs/Python/Python312/python.exe";
            process->start(fallbackPath, arguments);
            if (!process->waitForStarted(500)) {
                QMessageBox::critical(this, "Error", "Could not start Python. Please ensure you checked 'Add to PATH' when installing Python 3.12.");
                delete outputBuffer;
                delete errorBuffer;
                process->deleteLater();
            }
        }
    }
}

void EmployeeWindow::onPresenceReport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter Rapport de Présence",
                                                     "Rapport_Presence_" + QDate::currentDate().toString("dd_MM_yyyy") + ".pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf")) fileName += ".pdf";

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    QString html = R"(
        <div style='margin: 30px; font-family: sans-serif;'>
            <h1 style='text-align: center; color: #1e3a5f;'>RAPPORT DE PRÉSENCE – PORTFLOW</h1>
            <p style='text-align: center;'>Généré le )" + QDate::currentDate().toString("dd/MM/yyyy") + R"(</p>
            <hr>
            <table width='100%' border='1' cellspacing='0' cellpadding='8' style='border-collapse: collapse; margin-top: 20px;'>
                <tr style='background-color: #f1f5f9;'>
                    <th>Employé</th>
                    <th>Poste</th>
                    <th>Date</th>
                    <th>Arrivée</th>
                    <th>Départ</th>
                </tr>
    )";

    QSqlQuery query;
    query.prepare("SELECT E.PRENOM, E.NOM, E.\"POSITION\", P.DATE_PRESENCE, P.HEURE_ARRIVEE, P.HEURE_DEPART "
                  "FROM PRESENCE P JOIN EMPLOYEES E ON P.ID_EMPLOYE = E.ID_EMPLOYE "
                  "ORDER BY P.DATE_PRESENCE DESC, P.HEURE_ARRIVEE DESC");
    
    bool foundData = false;
    if (query.exec()) {
        while (query.next()) {
            foundData = true;
            QString name = query.value(0).toString() + " " + query.value(1).toString();
            QString pos = query.value(2).toString();
            QString date = query.value(3).toDate().toString("dd/MM/yyyy");
            QString arrival = query.value(4).toDateTime().toString("HH:mm:ss");
            QString departure = query.value(5).toDateTime().toString("HH:mm:ss");
            if (query.value(5).isNull()) departure = "<span style='color: #2563EB;'>En cours</span>";

            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                    .arg(name, pos, date, arrival, departure);
        }
    }
    
    if (!foundData) {
        // Fallback for EMPLOYEE table
        QSqlQuery query2;
        query2.prepare("SELECT E.PRENOM, E.NOM, E.\"POSITION\", P.DATE_PRESENCE, P.HEURE_ARRIVEE, P.HEURE_DEPART "
                      "FROM PRESENCE P JOIN EMPLOYEE E ON P.ID_EMPLOYE = E.ID_EMPLOYE "
                      "ORDER BY P.DATE_PRESENCE DESC, P.HEURE_ARRIVEE DESC");
        if (query2.exec()) {
             while (query2.next()) {
                QString name = query2.value(0).toString() + " " + query2.value(1).toString();
                QString pos = query2.value(2).toString();
                QString date = query2.value(3).toDate().toString("dd/MM/yyyy");
                QString arrival = query2.value(4).toDateTime().toString("HH:mm:ss");
                QString departure = query2.value(5).toDateTime().toString("HH:mm:ss");
                if (query2.value(5).isNull()) departure = "<span style='color: #2563EB;'>En cours</span>";

                html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                        .arg(name, pos, date, arrival, departure);
            }
        }
    }

    html += "</table></div>";
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Succès", "Le rapport de présence a été exporté avec succès.");
}

#include "aiintegration.h"
#include <QDateTime>

void EmployeeWindow::onSmartAIReport()
{
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Sélection Requise", "Veuillez sélectionner un employé pour générer son rapport IA.");
        return;
    }

    QTableWidgetItem* item = table->item(currentRow, 0);
    int empIndex = item->data(Qt::UserRole).toInt();
    const Employee& emp = employees[empIndex];

    // Show loading dialog or message
    QMessageBox* loading = new QMessageBox(this);
    loading->setWindowTitle("Analyse IA en cours");
    loading->setText("Génération du rapport intelligent pour " + emp.firstName + " " + emp.lastName + "...");
    loading->setStandardButtons(QMessageBox::NoButton);
    loading->show();
    QCoreApplication::processEvents();

    // Gather statistics from database
    EmployeeAnalysisContext context;
    context.name = emp.firstName + " " + emp.lastName;
    context.position = emp.position;
    context.salary = QString(emp.salary).remove(QRegularExpression("[^0-9]")).toDouble();
    
    QSqlQuery query;
    query.prepare("SELECT HEURE_ARRIVEE, HEURE_DEPART FROM PRESENCE WHERE ID_EMPLOYE = :id AND HEURE_DEPART IS NOT NULL");
    query.bindValue(":id", emp.id.toInt());
    
    qint64 totalSeconds = 0;
    int sessions = 0;
    if (query.exec()) {
        while (query.next()) {
            QDateTime arrival = query.value(0).toDateTime();
            QDateTime departure = query.value(1).toDateTime();
            totalSeconds += arrival.secsTo(departure);
            sessions++;
        }
    }
    context.totalHours = static_cast<int>(totalSeconds / 3600);
    context.sessionsCount = sessions;
    context.averageSessionHours = sessions > 0 ? (totalSeconds / 3600.0) / sessions : 0;

    // Call AI Service
    EmployeeAnalysisReport report = EmployeeIntelligenceService::generateSmartReport(context);
    
    loading->close();
    delete loading;

    if (report.success) {
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("Rapport Intelligent IA - PortFlow");
        dialog->setMinimumWidth(550);
        
        QString bgColor = isDarkMode ? "#1A202C" : "#F8FAFC";
        QString cardBg = isDarkMode ? "#2D3748" : "#F3F4F6";
        QString textColor = isDarkMode ? "#F7FAFC" : "#1E293B";
        QString accentColor = "#8B5CF6";
        
        dialog->setStyleSheet(QString("QDialog { background-color: %1; }").arg(bgColor));
        
        QVBoxLayout* layout = new QVBoxLayout(dialog);
        layout->setSpacing(20);
        layout->setContentsMargins(30, 30, 30, 30);
        
        QLabel* title = new QLabel("✨ Analyse Prédictive & Performance");
        title->setStyleSheet(QString("font-size: 22px; font-weight: bold; color: %1;").arg(accentColor));
        layout->addWidget(title);
        
        // Summary Card
        QFrame* summaryCard = new QFrame();
        summaryCard->setStyleSheet(QString("background-color: %1; border-radius: 12px; padding: 15px;").arg(cardBg));
        QVBoxLayout* summaryLay = new QVBoxLayout(summaryCard);
        
        QLabel* nameLabel = new QLabel("Employé: " + context.name);
        nameLabel->setStyleSheet(QString("font-weight: bold; font-size: 14px; color: %1;").arg(textColor));
        summaryLay->addWidget(nameLabel);
        
        QLabel* statsLabel = new QLabel(QString("Heures totales: %1h | Sessions: %2 | Score de Productivité: %3%")
                                        .arg(context.totalHours).arg(context.sessionsCount).arg(report.productivityScore, 0, 'f', 1));
        statsLabel->setStyleSheet(isDarkMode ? "color: #A0AEC0;" : "color: #4B5563;");
        summaryLay->addWidget(statsLabel);
        layout->addWidget(summaryCard);
        
        // Analysis
        QLabel* analysisTitle = new QLabel("📝 Analyse de l'IA (Google Gemini)");
        analysisTitle->setStyleSheet(QString("font-weight: bold; color: %1; margin-top: 10px;").arg(isDarkMode ? "#63B3ED" : "#1E3A8A"));
        layout->addWidget(analysisTitle);
        
        QLabel* analysisText = new QLabel(report.analysis);
        analysisText->setWordWrap(true);
        analysisText->setStyleSheet(QString(R"(
            QLabel { 
                line-height: 1.5; color: %1; background: %2; padding: 12px; 
                border: 1px solid %3; border-radius: 10px; 
            }
        )").arg(textColor, isDarkMode ? "#2D3748" : "white", isDarkMode ? "#4A5568" : "#E5E7EB"));
        layout->addWidget(analysisText);
        
        // Recommendation
        QLabel* recoTitle = new QLabel("💡 Recommandations Stratégiques");
        recoTitle->setStyleSheet(QString("font-weight: bold; color: %1; margin-top: 10px;").arg(isDarkMode ? "#68D391" : "#059669"));
        layout->addWidget(recoTitle);
        
        QLabel* recoText = new QLabel(report.recommendation);
        recoText->setWordWrap(true);
        recoText->setStyleSheet(QString(R"(
            QLabel { 
                line-height: 1.5; color: %1; background: %2; padding: 12px; 
                border: 1px solid %3; border-radius: 10px; 
            }
        )").arg(textColor, isDarkMode ? "#1A365D" : "#ECFDF5", isDarkMode ? "#2B6CB0" : "#A7F3D0"));
        layout->addWidget(recoText);
        
        QPushButton* closeBtn = new QPushButton("Fermer");
        closeBtn->setFixedHeight(45);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet(QString(R"(
            QPushButton { 
                background-color: %1; color: white; font-weight: bold; 
                border-radius: 10px; margin-top: 10px; 
            }
            QPushButton:hover { background-color: #7C3AED; }
        )").arg(accentColor));
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        layout->addWidget(closeBtn);
        
        dialog->exec();
    } else {
        QMessageBox::critical(this, "Erreur AI", "Impossible de générer le rapport IA pour le moment.");
    }
}
