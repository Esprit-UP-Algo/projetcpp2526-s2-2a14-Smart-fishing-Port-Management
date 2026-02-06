#include "employeewindow.h"
#include "employeedialog.h"
#include "loginwindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>

EmployeeWindow::EmployeeWindow(QWidget *parent)
    : QWidget(parent)
{
    // Initialize sample data with firstName and lastName
    employees.append({"EMP001", "Ahmed", "Khalil", "Marin", "1200 DT", "12/03/2023", "Actif"});
    employees.append({"EMP002", "Fatima", "Ben Salem", "RH", "1500 DT", "20/06/2022", "Congé"});
    employees.append({"EMP003", "Mohamed", "Jebali", "Technicien", "1400 DT", "05/01/2024", "Inactif"});
    employees.append({"EMP004", "Leila", "Trabelsi", "Sécurité", "1300 DT", "15/09/2021", "Actif"});

    setupUi();
}

EmployeeWindow::~EmployeeWindow()
{
}

QString EmployeeWindow::generateEmployeeId()
{
    int maxId = 0;

    // Find the highest existing ID number
    for (const Employee& emp : employees) {
        if (emp.id.startsWith("EMP")) {
            QString numberPart = emp.id.mid(3); // Get everything after "EMP"
            bool ok;
            int currentId = numberPart.toInt(&ok);
            if (ok && currentId > maxId) {
                maxId = currentId;
            }
        }
    }

    // Generate new ID with proper zero-padding
    return QString("EMP%1").arg(maxId + 1, 3, 10, QChar('0'));
}

void EmployeeWindow::setupUi()
{
    setWindowTitle("PortFlow - Gestion Employés");
    setGeometry(100, 100, 1400, 800);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #FFFFFF;
        }
    )");

    // Create central widget
    // QWidget* centralWidget = new QWidget(this); // not needed if this is the widget
    // setCentralWidget(centralWidget); // not needed

    // Main layout
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    // Mimic FrigoPage default layout or minimal margins
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    mainLayout->setSpacing(0);

    // Sidebar - REMOVED
    // QFrame* sidebar = createSidebar();
    // mainLayout->addWidget(sidebar);

    // Content area
    QWidget* content = createContentArea();
    mainLayout->addWidget(content);
}


QWidget* EmployeeWindow::createContentArea()
{
    QFrame* content = new QFrame();
    content->setStyleSheet("background-color: transparent;");

    QVBoxLayout* layout = new QVBoxLayout(content);
    // Use default margins similar to QVBoxLayout default (~11px) used in FrigoPage
    // or set explicitly to something small if Frigo layout looked tighter.
    // Dashboard code: `QVBoxLayout *frigoLayout = new QVBoxLayout(frigoPage);` -> uses defaults.
    // So we should probably remove the explicit 20px margins or reduce them significantly.
    // Let's rely on default by not analyzing, or set to standard ~11.
    // However, user wants "same size". 
    // Let's use 11 which is Qt default roughly.
    layout->setContentsMargins(11, 11, 11, 11);
    layout->setSpacing(10);

    // Header
    QFrame* header = createHeader();
    layout->addWidget(header);

    // Add Employee Button Row
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("➕ Ajouter Employé");
    QFont btnFont("Segoe UI", 11);
    addBtn->setFont(btnFont);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            height: 35px;
            border-radius: 8px;
            padding: 0 15px;
            border: none;
        }
        QPushButton:hover {
            background-color: #5D9CEC;
        }
    )");
    connect(addBtn, &QPushButton::clicked, this, &EmployeeWindow::onAddEmployee);
    
    btnLayout->addWidget(addBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    // Employee table card
    QFrame* tableCard = createTableCard();
    layout->addWidget(tableCard, 1);

    return content;
}

QFrame* EmployeeWindow::createHeader()
{
    QFrame* header = new QFrame();
    // header->setFixedHeight(100); // Removed fixed height
    header->setStyleSheet("background-color: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(header);
    // Reduced margins to match Frigo (almost 0)
    layout->setContentsMargins(0, 0, 0, 10); 

    // Gray frame for title -> Blue Frame
    QWidget* titleFrame = new QWidget();
    titleFrame->setStyleSheet(R"(
        background-color: #2B5EA6;
        border-radius: 10px;
    )");
    titleFrame->setFixedHeight(55);
    titleFrame->setFixedWidth(1200);

    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(15, 8, 15, 8);

    QLabel* title = new QLabel("Gestion des Employés");
    QFont titleFont("Segoe UI", 22, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #FFFFFF;");
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    // Search Bar
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Rechercher...");
    searchEdit->setFixedSize(220, 35);
    searchEdit->setStyleSheet("QLineEdit{background:white; color:black; border-radius:8px; padding-left:10px; font-size:14px;}");
    connect(searchEdit, &QLineEdit::textChanged, this, &EmployeeWindow::onSearch);
    titleLayout->addWidget(searchEdit);

    // Shadow for title frame
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(12);
    shadow->setXOffset(5);
    shadow->setYOffset(3);
    shadow->setColor(QColor(59, 130, 246, 60));
    titleFrame->setGraphicsEffect(shadow);

    layout->addWidget(titleFrame);
    layout->addStretch();

    return header;
}

QFrame* EmployeeWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet("background-color: transparent;");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(15);
    // Removed extra 20px margins to maximize table size
    layout->setContentsMargins(0, 0, 0, 0);

    // Blue frame container for table
    QWidget* tableFrame = new QWidget();
    tableFrame->setStyleSheet("background:#2B5EA6;border-radius:15px;");

    QVBoxLayout* frameLayout = new QVBoxLayout(tableFrame);
    frameLayout->setContentsMargins(20, 20, 20, 20);

    // Table
    table = new QTableWidget();
    setupTable();
    populateTable();
    frameLayout->addWidget(table);

    layout->addWidget(tableFrame);

    return card;
}

void EmployeeWindow::setupTable()
{
    // Updated to 9 columns: ID, First Name, Last Name, Position, Salary, Date, Status, Modifier, Supprimer
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({
        "ID", "Prénom", "Nom", "Position", "Salaire", "Date recrutement", "Statut", "Modifier", "Supprimer"
    });

    // Table configuration
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    table->setSelectionBehavior(QTableWidget::SelectRows);

    // Table style with white background and gray rounded header
    table->setStyleSheet(
        "QTableWidget{background:white;color:black;border:none;border-radius:12px;}"
        "QTableWidget::item{background:white;}"
        "QTableWidget QWidget{background:white;}"
        "QHeaderView::section{"
        "background:#d1d5db;color:black;font-weight:bold;padding:6px;border:none;}"
        "QHeaderView::section:first{border-top-left-radius:12px;}"
        "QHeaderView::section:last{border-top-right-radius:12px;}"
        );

    QFont tableFont("Segoe UI", 10);
    table->setFont(tableFont);

    // Set column widths
    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);   // ID
    header->setSectionResizeMode(1, QHeaderView::Stretch); // First Name
    header->setSectionResizeMode(2, QHeaderView::Stretch); // Last Name
    header->setSectionResizeMode(3, QHeaderView::Fixed);   // Position
    header->setSectionResizeMode(4, QHeaderView::Fixed);   // Salary
    header->setSectionResizeMode(5, QHeaderView::Fixed);   // Date
    header->setSectionResizeMode(6, QHeaderView::Fixed);   // Status
    header->setSectionResizeMode(7, QHeaderView::Fixed);   // Modifier
    header->setSectionResizeMode(8, QHeaderView::Fixed);   // Supprimer

    table->setColumnWidth(0, 90);   // ID
    table->setColumnWidth(3, 120);  // Position
    table->setColumnWidth(4, 110);  // Salary
    table->setColumnWidth(5, 140);  // Date
    table->setColumnWidth(6, 100);  // Status
    table->setColumnWidth(7, 90);   // Modifier
    table->setColumnWidth(8, 90);   // Supprimer
}

void EmployeeWindow::populateTable(const QString& filterText)
{
    table->setRowCount(0);

    for (int i = 0; i < employees.size(); ++i) {
        const Employee& emp = employees[i];

        // Filter - include firstName and lastName in search
        if (!filterText.isEmpty()) {
            QString searchStr = emp.id + emp.firstName + emp.lastName + emp.position + emp.salary + emp.date + emp.status;
            if (!searchStr.contains(filterText, Qt::CaseInsensitive)) {
                continue;
            }
        }

        int row = table->rowCount();
        table->insertRow(row);

        // Store the actual employee index in the table row data
        QTableWidgetItem* idItem = new QTableWidgetItem(emp.id);
        idItem->setData(Qt::UserRole, i); // Store the actual index
        table->setItem(row, 0, idItem);

        // First Name
        table->setItem(row, 1, new QTableWidgetItem(emp.firstName));

        // Last Name
        table->setItem(row, 2, new QTableWidgetItem(emp.lastName));

        // Position
        table->setItem(row, 3, new QTableWidgetItem(emp.position));

        // Salary
        table->setItem(row, 4, new QTableWidgetItem(emp.salary));

        // Date
        table->setItem(row, 5, new QTableWidgetItem(emp.date));

        // Status
        QWidget* statusWidget = createStatusBadge(emp.status);
        table->setCellWidget(row, 6, statusWidget);

        // Modifier button (column 7)
        QWidget* editButton = createActionButtons(i);
        table->setCellWidget(row, 7, editButton);

        // Supprimer button (column 8)
        QWidget* deleteButton = createDeleteButton(i);
        table->setCellWidget(row, 8, deleteButton);

        // Set row height
        table->setRowHeight(row, 45);
    }
}

QWidget* EmployeeWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    widget->setStyleSheet("background: transparent;");
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 0, 0, 0);

    QLabel* badge = new QLabel(status);
    QFont badgeFont("Segoe UI", 10, QFont::Bold);
    badge->setFont(badgeFont);
    badge->setAlignment(Qt::AlignCenter);

    QString color;
    if (status == "Actif") {
        color = "#22c55e";  // Green
    } else if (status == "Congé") {
        color = "#f59e0b";  // Orange
    } else {  // Inactif
        color = "#ef4444";  // Red
    }

    badge->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));

    layout->addWidget(badge);
    return widget;
}

QWidget* EmployeeWindow::createActionButtons(int row)
{
    // Create Edit button (yellow)
    QPushButton* editBtn = new QPushButton("✏️");
    editBtn->setStyleSheet(R"(
        QPushButton {
            background: #facc15;
            color: black;
            border-radius: 6px;
            height: 28px;
            border: none;
        }
        QPushButton:hover {
            background: #eab308;
        }
    )");
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditEmployee(row); });

    return editBtn;
}

QWidget* EmployeeWindow::createDeleteButton(int row)
{
    // Create Delete button (red)
    QPushButton* deleteBtn = new QPushButton("🗑️");
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            background: #ef4444;
            color: white;
            border-radius: 6px;
            height: 28px;
            border: none;
        }
        QPushButton:hover {
            background: #dc2626;
        }
    )");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteEmployee(row); });

    return deleteBtn;
}

void EmployeeWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void EmployeeWindow::onAddEmployee()
{
    EmployeeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Employee emp = dialog.getData();
        emp.id = generateEmployeeId();  // Auto-generate ID
        employees.append(emp);
        populateTable();
    }
}

void EmployeeWindow::onEditEmployee(int row)
{
    if (row >= 0 && row < employees.size()) {
        EmployeeDialog dialog(this, &employees[row]);
        if (dialog.exec() == QDialog::Accepted) {
            Employee updatedEmp = dialog.getData();
            updatedEmp.id = employees[row].id;  // Keep the same ID when editing
            employees[row] = updatedEmp;
            populateTable();
        }
    }
}

void EmployeeWindow::onDeleteEmployee(int row)
{
    if (row >= 0 && row < employees.size()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Confirmer la suppression",
            QString("Êtes-vous sûr de vouloir supprimer l'employé %1 %2?").arg(employees[row].firstName, employees[row].lastName),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            employees.removeAt(row);
            populateTable();
        }
    }
}


