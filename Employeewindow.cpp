#include "employeewindow.h"
#include "employeedialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QPixmap>

EmployeeWindow::EmployeeWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Données initiales de test
    Employee e1{"EMP001", "Ahmed", "Khalil", "Marin", "1200 DT", "12/03/2023", "Actif"};
    Employee e2{"EMP002", "Fatima", "Ben Salem", "RH", "1500 DT", "20/06/2022", "Congé"};
    Employee e3{"EMP003", "Mohamed", "Jebali", "Technicien", "1400 DT", "05/01/2024", "Inactif"};
    Employee e4{"EMP004", "Leila", "Trabelsi", "Sécurité", "1300 DT", "15/09/2021", "Actif"};

    employees.append(e1);
    employees.append(e2);
    employees.append(e3);
    employees.append(e4);

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
    QFrame* header = createHeader();
    layout->addWidget(header);

    // Table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* EmployeeWindow::createHeader()
{
    QFrame* header = new QFrame();
    header->setStyleSheet("background: transparent;");
    header->setFixedHeight(120);

    QVBoxLayout* layout = new QVBoxLayout(header);
    layout->setSpacing(15);

    // Title
    QLabel* title = new QLabel("Gestion des Employés");
    QFont titleFont("Segoe UI", 28, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2C3E50;");
    layout->addWidget(title);

    // Search and Add button row
    QHBoxLayout* actionRow = new QHBoxLayout();

    // Search
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher un employé...");
    QFont searchFont("Segoe UI", 12);
    searchInput->setFont(searchFont);
    searchInput->setFixedHeight(50);
    searchInput->setFixedWidth(400);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 12px;
            padding: 12px 20px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #5D9CEC;
        }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &EmployeeWindow::onSearch);
    actionRow->addWidget(searchInput);

    actionRow->addStretch();

    // Add button
    QPushButton* addBtn = new QPushButton("➕  Ajouter un employé");
    QFont btnFont("Segoe UI", 13, QFont::Bold);
    addBtn->setFont(btnFont);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(50);
    addBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 12px 30px;
        }
        QPushButton:hover {
            background-color: #4A89DC;
        }
        QPushButton:pressed {
            background-color: #3B77C4;
        }
    )");
    connect(addBtn, &QPushButton::clicked, this, &EmployeeWindow::onAddEmployee);
    actionRow->addWidget(addBtn);

    layout->addLayout(actionRow);

    return header;
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
    table->setHorizontalHeaderLabels({"ID", "Prénom", "Nom", "Position", "Salaire", "Date", "Statut", "Actions"});

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

    table->setColumnWidth(0, 90);   // ID
    table->setColumnWidth(1, 130);  // Prénom
    table->setColumnWidth(2, 130);  // Nom
    table->setColumnWidth(3, 120);  // Position
    table->setColumnWidth(4, 110);  // Salaire
    table->setColumnWidth(5, 140);  // Date
    table->setColumnWidth(6, 100);  // Statut
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

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(emp.id);
        idItem->setForeground(QBrush(QColor("#5D9CEC")));
        QFont idFont("Segoe UI", 11, QFont::Bold);
        idItem->setFont(idFont);
        idItem->setData(Qt::UserRole, i);
        table->setItem(row, 0, idItem);

        // Prénom
        QTableWidgetItem* prenomItem = new QTableWidgetItem(emp.firstName);
        prenomItem->setFont(cellFont);
        table->setItem(row, 1, prenomItem);

        // Nom
        QTableWidgetItem* nomItem = new QTableWidgetItem(emp.lastName);
        nomItem->setFont(cellFont);
        table->setItem(row, 2, nomItem);

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
        qDebug() << "Employé supprimé:" << employees[row].firstName << employees[row].lastName;
        employees.removeAt(row);
        populateTable(searchInput->text());
    }
}

void EmployeeWindow::onLogout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Quitter",
                                  "Voulez-vous vraiment quitter l'application ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Fermeture de l'application";
        this->close();
    }
}
