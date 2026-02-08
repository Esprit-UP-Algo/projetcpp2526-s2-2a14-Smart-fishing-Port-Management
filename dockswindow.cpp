#include "dockswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QHeaderView>
#include <QPushButton>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QDebug>

DocksWindow::DocksWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1400, 800);
    setWindowTitle("Smart Fishing Port - Docks");

    setupUI();
    setupDockTable();
    populateTable();
}

void DocksWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    // ===== Sidebar =====
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

    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0,0,0,0);
    sidebarLayout->setSpacing(0);

    // Sidebar logo
    QFrame* logoFrame = new QFrame();
    logoFrame->setFixedHeight(160);
    QVBoxLayout* logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setAlignment(Qt::AlignCenter);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix("C:/Users/yoser/OneDrive/Bureau/logoportflow.png");
    if(!logoPix.isNull())
        logoLabel->setPixmap(logoPix.scaled(140,140,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logoLabel->setText("🛥️");
    logoLabel->setAlignment(Qt::AlignCenter);

    logoLabel->setStyleSheet(R"(
        QLabel {
            background-color: white;
            border-radius: 20px;
            padding: 10px;
        }
    )");

    logoLayout->addWidget(logoLabel);
    sidebarLayout->addWidget(logoFrame);

    auto createNavBtn = [](const QString& text, bool active=false) {
        QPushButton* btn = new QPushButton(text);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(55);

        if(active) {
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: rgba(255,255,255,0.25);
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
                    color: rgba(255,255,255,0.9);
                    border: none;
                    border-radius: 12px;
                    text-align: left;
                    padding-left: 20px;
                }
                QPushButton:hover {
                    background-color: rgba(255,255,255,0.15);
                }
            )");
        }

        return btn;
    };

    sidebarLayout->addWidget(createNavBtn("🏠 Dashboard"));
    sidebarLayout->addWidget(createNavBtn("⛵ Bateaux"));
    sidebarLayout->addWidget(createNavBtn("🐟 Pêche"));
    sidebarLayout->addWidget(createNavBtn("👥 Employés"));
    sidebarLayout->addWidget(createNavBtn("⚓ Docks", true));
    sidebarLayout->addWidget(createNavBtn("⚙️ Paramètres"));
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(createNavBtn("🚪 Quitter"));

    mainLayout->addWidget(sidebar);

    // ===== Right Content Area =====
    QWidget* contentArea = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0,0,0,0);

    // Top Navbar
    QFrame* topNavbar = new QFrame();
    topNavbar->setFixedHeight(70);
    QHBoxLayout* navbarLayout = new QHBoxLayout(topNavbar);
    navbarLayout->setContentsMargins(25,0,25,0);

    QLabel* appTitle = new QLabel("Smart Fishing Port");
    appTitle->setStyleSheet("font-size: 20px; font-weight:bold;");
    navbarLayout->addWidget(appTitle);
    navbarLayout->addStretch();

    QPushButton* profileBtn = new QPushButton(" Profile");
    profileBtn->setCursor(Qt::PointingHandCursor);
    navbarLayout->addWidget(profileBtn);

    contentLayout->addWidget(topNavbar);

    // Scrollable main content
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(30,30,30,30);
    scrollLayout->setSpacing(25);

    QLabel* pageTitle = new QLabel("Dock Management");
    pageTitle->setStyleSheet("font-size: 24px; font-weight:bold;");
    scrollLayout->addWidget(pageTitle);

    // Table section
    QFrame* tableContainer = new QFrame();
    tableContainer->setStyleSheet(R"(
        QFrame {
            background-color: #ffffff;
            border-radius: 20px;
            padding: 15px;
        }
    )");
    QVBoxLayout* tableContainerLayout = new QVBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(0,0,0,0);

    dockTable = new QTableWidget();
    tableContainerLayout->addWidget(dockTable);
    scrollLayout->addWidget(tableContainer);

    scrollArea->setWidget(scrollContent);
    contentLayout->addWidget(scrollArea);

    mainLayout->addWidget(contentArea, 1);
}
void DocksWindow::setupDockTable()
{
    dockTable->setColumnCount(8);
    dockTable->setHorizontalHeaderLabels({"ID","Dock Name","Capacity","Max Size","Status","Rate","Current Client","Actions"});
    dockTable->verticalHeader()->setVisible(false);
    dockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dockTable->setAlternatingRowColors(false);
    dockTable->setShowGrid(false);
    dockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    dockTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #ffffff;
            border-radius: 15px;
            gridline-color: #E5E7EB;
            font-family: 'Segoe UI', 'Arial', sans-serif;
            font-size: 11pt;
            color: #111827;
        }
        QHeaderView::section {
            background-color: #F3F4F6;
            color: #111827;
            border: none;
            padding: 10px;
            font-weight: bold;
            font-size: 12pt;
            font-family: 'Segoe UI', 'Arial', sans-serif;
        }
        QTableWidget::item:selected {
            background-color: #DDEFFE;
            color: #2563EB;
        }
    )");

    dockTable->setRowCount(12);
    for(int row=0; row<dockTable->rowCount(); ++row)
        dockTable->setRowHeight(row, 55);
}

void DocksWindow::populateTable()
{
    QStringList statuses = {"Available","Occupied","Available","Maintenance","Occupied","Available",
                            "Occupied","Available","Occupied","Available","Occupied","Available"};
    QStringList clients = {"-","Sea Harvest Ltd","-","-","Ocean Fresh Co","-",
                           "Blue Wave Inc","-","Coastal Catch","-","Marine Traders","-"};

    int colActions = dockTable->columnCount() - 1;

    for(int i=0;i<dockTable->rowCount();i++){
        dockTable->setItem(i,0,new QTableWidgetItem(QString::number(i+1)));
        dockTable->setItem(i,1,new QTableWidgetItem("Dock "+QString::number(i+1)));
        dockTable->setItem(i,2,new QTableWidgetItem("2-3 boats"));
        dockTable->setItem(i,3,new QTableWidgetItem("15m"));

        QTableWidgetItem* statusItem = new QTableWidgetItem(statuses[i]);
        if(statuses[i]=="Available") statusItem->setForeground(QBrush(QColor("#10B981")));
        else if(statuses[i]=="Occupied") statusItem->setForeground(QBrush(QColor("#EF4444")));
        else statusItem->setForeground(QBrush(QColor("#F59E0B")));
        dockTable->setItem(i,4,statusItem);

        dockTable->setItem(i,5,new QTableWidgetItem("€50/day"));
        dockTable->setItem(i,6,new QTableWidgetItem(clients[i]));

        QWidget* container = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(5);
        layout->setAlignment(Qt::AlignCenter);

        auto createBtn = [](const QString& text, const QString& colorStr) {
            QColor color(colorStr);
            QPushButton* btn = new QPushButton(text);
            btn->setFixedHeight(28);
            btn->setStyleSheet(QString(R"(
                QPushButton {
                    background-color: %1;
                    color: white;
                    border-radius: 6px;
                    padding: 2px 10px;
                    font-size: 9pt;
                }
                QPushButton:hover {
                    background-color: %2;
                }
            )").arg(color.name()).arg(color.lighter(120).name()));
            return btn;
        };

        QPushButton* btnEdit = createBtn("Edit","#3B82F6");
        QPushButton* btnDelete = createBtn("Delete","#EF4444");
        QPushButton* btnUpdate = createBtn("Update","#10B981");

        layout->addWidget(btnEdit);
        layout->addWidget(btnDelete);
        layout->addWidget(btnUpdate);

        dockTable->setCellWidget(i,colActions,container);

        QObject::connect(btnEdit, &QPushButton::clicked, [i](){ qDebug() << "Edit clicked for row" << i; });
        QObject::connect(btnDelete, &QPushButton::clicked, [i](){ qDebug() << "Delete clicked for row" << i; });
        QObject::connect(btnUpdate, &QPushButton::clicked, [i](){ qDebug() << "Update clicked for row" << i; });
    }
}
