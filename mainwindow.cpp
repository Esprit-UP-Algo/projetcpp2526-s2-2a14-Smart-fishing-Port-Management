#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addfrigodialog.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Visual Setup
    setupLogo();
    setupShadows();
    setupTable();

    // Data
    loadSampleData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupLogo()
{
    QPixmap logo(":/images/logo.png");

    if (!logo.isNull()) {
        int logoWidth = ui->logoLabel->width();
        int logoHeight = ui->logoLabel->height();

        logo = logo.scaled(logoWidth, logoHeight,
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

        ui->logoLabel->setPixmap(logo);
        ui->logoLabel->setAlignment(Qt::AlignCenter);
        ui->logoLabel->setScaledContents(false);
    } else {
         ui->logoLabel->setText("PORTFLOW");
         // Style set in QSS
    }
}

void MainWindow::setupShadows()
{
    // Lighter shadows for modern SaaS look
    
    // Logo shadow ? Removed to be flat or very subtle
    // ui->logoLabel->setGraphicsEffect(nullptr); 

    // Title shadow - removed for clean look
    // ui->titleFrame->setGraphicsEffect(nullptr);

    // If we want a card shadow for the table, we'd apply it to the table's container.
    // Since the table is a widget, we can try applying it directly, but QSS radius + shadow can glitch.
    
    QGraphicsDropShadowEffect *tableShadow = new QGraphicsDropShadowEffect();
    tableShadow->setBlurRadius(30);
    tableShadow->setXOffset(0);
    tableShadow->setYOffset(10);
    tableShadow->setColor(QColor(0, 0, 0, 20)); // Soft shadow
    ui->tableCard->setGraphicsEffect(tableShadow);
}

void MainWindow::setupTable()
{
    ui->frigoTable->setColumnCount(7); 
    QStringList headers = {"ID", "Date", "Address", "Status", "Transport Type", "Price", "Actions"};
    ui->frigoTable->setHorizontalHeaderLabels(headers);

    ui->frigoTable->horizontalHeader()->setStretchLastSection(true);
    ui->frigoTable->verticalHeader()->setVisible(false);
    ui->frigoTable->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->frigoTable->setSelectionBehavior(QTableWidget::SelectRows);
    
    // Set column widths
    ui->frigoTable->setColumnWidth(0, 100); 
    ui->frigoTable->setColumnWidth(1, 120); 
    ui->frigoTable->setColumnWidth(2, 200); 
    ui->frigoTable->setColumnWidth(3, 100); 
    ui->frigoTable->setColumnWidth(4, 150); 
    ui->frigoTable->setColumnWidth(5, 100); 
    
    ui->frigoTable->verticalHeader()->setDefaultSectionSize(60); 
}

void MainWindow::loadSampleData()
{
    addFrigoData("LIV001", "2024-02-01", "123 Port St, Tunis", "In Progress", "Truck", "150 USD");
    addFrigoData("LIV002", "2024-02-02", "Industrial Zone, Sfax", "Delivered", "Boat", "5000 USD");
    addFrigoData("LIV003", "2024-02-02", "Central Market, Sousse", "Pending", "Refrig. Truck", "300 USD");
}

void MainWindow::addFrigoData(const QString &id, const QString &date,
                              const QString &address, const QString &status,
                              const QString &type, const QString &price)
{
    int row = ui->frigoTable->rowCount();
    ui->frigoTable->insertRow(row);

    QTableWidgetItem *idItem = new QTableWidgetItem(id);
    QTableWidgetItem *dateItem = new QTableWidgetItem(date);
    QTableWidgetItem *addrItem = new QTableWidgetItem(address);
    QTableWidgetItem *statItem = new QTableWidgetItem(status);
    QTableWidgetItem *typeItem = new QTableWidgetItem(type);
    QTableWidgetItem *priceItem = new QTableWidgetItem(price);

    // Alignments
    idItem->setTextAlignment(Qt::AlignCenter);
    dateItem->setTextAlignment(Qt::AlignCenter);
    // Address, Type, Price -> Default (Left) for better readability & alignment with header
    // addrItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Default is left
    // statItem is replaced by widget
    // typeItem -> Left
    // priceItem -> Left

    ui->frigoTable->setItem(row, 0, idItem);
    ui->frigoTable->setItem(row, 1, dateItem);
    ui->frigoTable->setItem(row, 2, addrItem);
    ui->frigoTable->setItem(row, 3, statItem);
    ui->frigoTable->setItem(row, 4, typeItem);
    ui->frigoTable->setItem(row, 5, priceItem);

    // --- ACTIONS COLUMN ---
    QWidget *actionWidget = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(5, 5, 5, 5); // Balanced padding
    actionLayout->setSpacing(10); // More space between buttons

    QPushButton *btnEdit = new QPushButton("Edit");
    QPushButton *btnDelete = new QPushButton("Delete");

    // Assign IDs for CSS styling
    btnEdit->setObjectName("btnTableEdit");
    btnDelete->setObjectName("btnTableDelete");

    // Tooltips
    btnEdit->setToolTip("Edit Delivery");
    btnDelete->setToolTip("Delete Delivery");

    btnEdit->setToolTip("Edit Delivery");
    btnDelete->setToolTip("Delete Delivery");

    // Connect signals
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onUpdateRow);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteRow);

    actionLayout->addWidget(btnEdit);
    actionLayout->addWidget(btnDelete);
    actionLayout->addStretch(); // Push to left or center? Let's keep them centered/left.

    ui->frigoTable->setCellWidget(row, 6, actionWidget);
}

void MainWindow::on_btnAddFrigo_clicked()
{
    AddFrigoDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Correct Order: ID, Date, Address, Status, Transport Type, Price
        addFrigoData(dialog.getId(), 
                     dialog.getCap(),   // Date
                     dialog.getHum(),   // Address
                     dialog.getStatus(), // Status
                     dialog.getFish(),   // Transport Type
                     dialog.getTemp());  // Price
    }
}

void MainWindow::onDeleteRow()
{
    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (!senderBtn) return;

    // Use iteration to find the row. This is the most robust method.
    QWidget *cellWidget = senderBtn->parentWidget();
    int row = -1;
    
    for(int i = 0; i < ui->frigoTable->rowCount(); ++i) {
        if(ui->frigoTable->cellWidget(i, 6) == cellWidget) {
            row = i;
            break;
        }
    }
    
    if (row != -1) {
        ui->frigoTable->removeRow(row);
    }
}

void MainWindow::onUpdateRow()
{
    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (!senderBtn) return;

    QWidget *cellWidget = senderBtn->parentWidget();
    int r = -1;
    
    for(int i = 0; i < ui->frigoTable->rowCount(); ++i) {
        if(ui->frigoTable->cellWidget(i, 6) == cellWidget) {
            r = i;
            break;
        }
    }
    
    if (r != -1) {
        // Get existing data
        QString id = ui->frigoTable->item(r, 0)->text();
        QString date = ui->frigoTable->item(r, 1)->text();
        QString addr = ui->frigoTable->item(r, 2)->text();
        QString status = ui->frigoTable->item(r, 3)->text();
        QString type = ui->frigoTable->item(r, 4)->text();
        QString price = ui->frigoTable->item(r, 5)->text();

        AddFrigoDialog dialog(this);
        dialog.setWindowTitle("Edit Delivery");
        dialog.setData(id, date, addr, price, status, type);

        if (dialog.exec() == QDialog::Accepted) {
            ui->frigoTable->item(r, 0)->setText(dialog.getId());
            ui->frigoTable->item(r, 1)->setText(dialog.getCap()); // Date
            ui->frigoTable->item(r, 2)->setText(dialog.getHum()); // Addr
            ui->frigoTable->item(r, 3)->setText(dialog.getStatus());
            ui->frigoTable->item(r, 4)->setText(dialog.getFish()); // Type
            ui->frigoTable->item(r, 5)->setText(dialog.getTemp()); // Price
        }
    }
}
