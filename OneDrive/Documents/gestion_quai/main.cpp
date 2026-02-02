#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QBrush>
#include <QColor>
#include <QScrollBar>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setFont(QFont("Times New Roman", 10));

    QMainWindow window;
    window.setWindowTitle("Smart Fishing Port");
    window.resize(1200, 700);

    QWidget* centralWidget = new QWidget();
    window.setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    // ===== Top Navbar =====
    QFrame* topNavbar = new QFrame();
    topNavbar->setFixedHeight(70);
    topNavbar->setObjectName("topNavbar");

    QHBoxLayout* navbarLayout = new QHBoxLayout(topNavbar);
    navbarLayout->setContentsMargins(25, 0, 25, 0);

    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(85, 85);
    logoContainer->setObjectName("logoContainer");
    logoContainer->setStyleSheet("QFrame#logoContainer { background-color: white; border-radius: 42px; }");

    QVBoxLayout* logoLayout = new QVBoxLayout(logoContainer);
    logoLayout->setContentsMargins(0,0,0,0);
    logoLayout->setAlignment(Qt::AlignCenter);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix("C:/Users/yoser/Downloads/logo_portflow-removebg-preview.png");
    if (!logoPix.isNull())
        logoLabel->setPixmap(logoPix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else {
        logoLabel->setText("🚢");
        logoLabel->setStyleSheet("font-size: 32px;");
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLayout->addWidget(logoLabel);
    navbarLayout->addWidget(logoContainer);

    QLabel* appTitle = new QLabel("Smart Fishing Port");
    appTitle->setObjectName("appTitle");
    navbarLayout->addWidget(appTitle);
    navbarLayout->addStretch();

    QPushButton* profileBtn = new QPushButton(" Profile");
    profileBtn->setObjectName("navBtn");
    profileBtn->setCursor(Qt::PointingHandCursor);
    navbarLayout->addWidget(profileBtn);

    mainLayout->addWidget(topNavbar);

    // ===== Main horizontal area =====
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(horizontalLayout);

    // ===== Sidebar =====
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(240);
    sidebar->setObjectName("sidebar");

    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(15,30,15,15);
    sidebarLayout->setSpacing(8);

    QPushButton* btnDock = new QPushButton(" Docks");
    QPushButton* btnLocations = new QPushButton(" Cold Storage");
    QPushButton* btnStats = new QPushButton(" Delivery");
    QPushButton* btnFishBatch = new QPushButton(" Fishbatch");
    QPushButton* btnBoatMgmt = new QPushButton(" Boat Management");

    QList<QPushButton*> buttons = {btnDock, btnLocations, btnStats, btnFishBatch, btnBoatMgmt};
    for(auto b: buttons){
        b->setCursor(Qt::PointingHandCursor);
        b->setObjectName("sidebarBtn");
        sidebarLayout->addWidget(b);
    }

    btnDock->setProperty("active", true);
    btnDock->style()->unpolish(btnDock);
    btnDock->style()->polish(btnDock);

    sidebarLayout->addStretch();

    QPushButton* logoutBtn = new QPushButton(" Logout");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setObjectName("sidebarBtn");
    logoutBtn->setStyleSheet("text-align:left; padding-left:18px;");
    sidebarLayout->addWidget(logoutBtn);

    // ===== Scrollable Main Content =====
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("scrollArea");

    QWidget* contentWidget = new QWidget();
    contentWidget->setObjectName("contentWidget");
    scrollArea->setWidget(contentWidget);

    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30,30,30,30);
    contentLayout->setSpacing(25);

    QLabel* pageTitle = new QLabel("Dashboard Overview");
    pageTitle->setObjectName("pageTitle");
    contentLayout->addWidget(pageTitle);

    // ===== Dashboard Cards =====
    QHBoxLayout* dashboardLayout = new QHBoxLayout();
    dashboardLayout->setSpacing(20);

    auto createStatCard = [](const QString& title, const QString& value, const QString& icon, const QString& color) {
        QFrame* card = new QFrame();
        card->setObjectName("statCard");
        card->setStyleSheet(QString("QFrame#statCard { background-color: white; border-radius: 12px; border-left: 4px solid %1; }").arg(color));

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(15); shadow->setXOffset(0); shadow->setYOffset(3); shadow->setColor(QColor(0,0,0,30));
        card->setGraphicsEffect(shadow);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20,18,20,18);

        QLabel* iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size:28px; background:transparent; border:none;");

        QLabel* valueLabel = new QLabel(value); valueLabel->setObjectName("statValue");
        QLabel* titleLabel = new QLabel(title); titleLabel->setObjectName("statTitle");

        cardLayout->addWidget(iconLabel); cardLayout->addWidget(valueLabel); cardLayout->addWidget(titleLabel);
        cardLayout->addStretch();

        return card;
    };

    dashboardLayout->addWidget(createStatCard("Total Docks", "12", "", "#3B82F6"));
    dashboardLayout->addWidget(createStatCard("Available", "5", "", "#10B981"));
    dashboardLayout->addWidget(createStatCard("Occupied", "6", "", "#F59E0B"));
    dashboardLayout->addWidget(createStatCard("Today's Revenue", "€450", "", "#8B5CF6"));

    contentLayout->addLayout(dashboardLayout);

    // ===== Table Section =====
    QLabel* tableTitle = new QLabel("Dock Management");
    tableTitle->setObjectName("sectionTitle");
    contentLayout->addWidget(tableTitle);

    QFrame* tableContainer = new QFrame();
    tableContainer->setObjectName("tableContainer");

    QGraphicsDropShadowEffect* tableShadow = new QGraphicsDropShadowEffect();
    tableShadow->setBlurRadius(20); tableShadow->setXOffset(0); tableShadow->setYOffset(4); tableShadow->setColor(QColor(0,0,0,25));
    tableContainer->setGraphicsEffect(tableShadow);

    QVBoxLayout* tableContainerLayout = new QVBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(0,0,0,0);

    QTableWidget* table = new QTableWidget();
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"ID","Dock Name","Capacity","Max Size","Status","Rate","Current Client","Actions"});
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setObjectName("dockTable");

    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // ===== Fix last column width for buttons =====
    int colActions = table->columnCount() - 1;
    for(int i=0; i<table->columnCount(); ++i)
        if(i == colActions) table->setColumnWidth(i, 500); // last column wider
        else table->setColumnWidth(i, 150);

    // Make table rows taller
    table->setRowCount(12);
    for(int row=0; row<table->rowCount(); ++row)
        table->setRowHeight(row, 60);

    // ===== Populate table with Edit, Delete, Update buttons =====
    for(int row=0; row<table->rowCount(); ++row) {
        QWidget* container = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(30); // space between buttons
        layout->setAlignment(Qt::AlignCenter);

        QPushButton* btnEdit = new QPushButton("Edit");
        QPushButton* btnDelete = new QPushButton("Delete");
        QPushButton* btnUpdate = new QPushButton("Update");

        btnEdit->setObjectName("editBtn");
        btnDelete->setObjectName("deleteBtn");
        btnUpdate->setObjectName("updateBtn");

        btnEdit->setFixedSize(60,28);
        btnDelete->setFixedSize(60,28);
        btnUpdate->setFixedSize(60,28);

        layout->addWidget(btnEdit);
        layout->addWidget(btnDelete);
        layout->addWidget(btnUpdate);

        table->setCellWidget(row,colActions,container);

        QObject::connect(btnEdit, &QPushButton::clicked, [row](){ qDebug() << "Edit clicked for row" << row; });
        QObject::connect(btnDelete, &QPushButton::clicked, [row](){ qDebug() << "Delete clicked for row" << row; });
        QObject::connect(btnUpdate, &QPushButton::clicked, [row](){ qDebug() << "Update clicked for row" << row; });
    }

    // Sample data
    QStringList statuses = {"Available","Occupied","Available","Maintenance","Occupied","Available",
                            "Occupied","Available","Occupied","Available","Occupied","Available"};
    QStringList clients = {"-","Sea Harvest Ltd","-","-","Ocean Fresh Co","-",
                           "Blue Wave Inc","-","Coastal Catch","-","Marine Traders","-"};
    for(int i=0;i<12;i++){
        table->setItem(i,0,new QTableWidgetItem(QString::number(i+1)));
        table->setItem(i,1,new QTableWidgetItem("Dock "+QString::number(i+1)));
        table->setItem(i,2,new QTableWidgetItem("2-3 boats"));
        table->setItem(i,3,new QTableWidgetItem("15m"));

        QTableWidgetItem* statusItem = new QTableWidgetItem(statuses[i]);
        if(statuses[i]=="Available") statusItem->setForeground(QBrush(QColor("#10B981")));
        else if(statuses[i]=="Occupied") statusItem->setForeground(QBrush(QColor("#EF4444")));
        else statusItem->setForeground(QBrush(QColor("#F59E0B")));
        table->setItem(i,4,statusItem);

        table->setItem(i,5,new QTableWidgetItem("€50/day"));
        table->setItem(i,6,new QTableWidgetItem(clients[i]));
    }

    table->setMinimumHeight(450);
    tableContainerLayout->addWidget(table);
    contentLayout->addWidget(tableContainer);

    horizontalLayout->addWidget(sidebar);
    horizontalLayout->addWidget(scrollArea,1);

    for(auto b: buttons){
        QObject::connect(b,&QPushButton::clicked,[=](){
            for(auto btn: buttons) btn->setProperty("active",false);
            b->setProperty("active",true);
            for(auto btn: buttons) btn->style()->unpolish(btn), btn->style()->polish(btn);
        });
    }

    // ===== Styles (unchanged) =====
    QString styleSheet = R"(
        *{ font-family:'Times New Roman', serif; }
        QFrame#topNavbar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3B82F6, stop:1 #60A5FA); border-bottom:1px solid #93C5FD; }
        QLabel#appTitle { font-weight:700; font-size:22px; color:white; }
        QPushButton#navBtn { background-color: rgba(255,255,255,0.9); color:#1E40AF; border:1px solid rgba(59,130,246,0.5); padding:10px 20px; font-size:14px; font-weight:500; border-radius:8px; min-width:100px; }
        QPushButton#navBtn:hover{background-color:rgba(59,130,246,0.1);}
        QPushButton#navBtn:pressed{background-color:rgba(59,130,246,0.2);}
        QFrame#sidebar{ background-color:#DBEAFE; border-right:1px solid #93C5FD; }
        QPushButton#sidebarBtn{ background-color:transparent; color:#1E40AF; text-align:left; padding:14px 18px; font-size:15px; font-weight:500; border-radius:10px; margin:2px 0; }
        QPushButton#sidebarBtn:hover{ background-color:rgba(59,130,246,0.2); color:#1E40AF; }
        QPushButton#sidebarBtn[active="true"]{ background-color:#3B82F6; color:white; }
        QWidget#contentWidget{ background-color:#F8FAFC; }
        QLabel#pageTitle{ font-size:28px; font-weight:700; color:#0F172A; padding-bottom:5px; }
        QLabel#sectionTitle{ font-size:20px; font-weight:600; color:#1E293B; padding-top:10px; }
        QLabel#statValue{ font-size:32px; font-weight:700; color:#0F172A; padding-top:8px; }
        QLabel#statTitle{ font-size:13px; font-weight:500; color:#64748B; text-transform:uppercase; letter-spacing:0.5px; }
        QFrame#tableContainer{ background-color:white; border-radius:12px; }
        QTableWidget#dockTable{ background-color:white; border:none; border-radius:12px; font-size:14px; color:#334155; }
        QHeaderView::section{ background-color:#F8FAFC; color:#475569; padding:12px 16px; border:none; border-bottom:2px solid #E2E8F0; font-weight:600; font-size:13px; text-transform:uppercase; letter-spacing:0.5px; }
        QTableWidget#dockTable::item{ padding:12px 16px; border-bottom:1px solid #F1F5F9; }
        QTableWidget#dockTable::item:selected{ background-color:#EFF6FF; color:#1E40AF; }
        QTableWidget#dockTable::item:alternate{ background-color:#FAFBFC; }
        QPushButton#editBtn{ background-color:#3B82F6; color:white; border:none; border-radius:4px; font-weight:600; font-size:11px; padding:4px 8px; }
        QPushButton#editBtn:hover{ background-color:#2563EB; }
        QPushButton#deleteBtn{ background-color:#EF4444; color:white; border:none; border-radius:4px; font-weight:600; font-size:11px; padding:4px 8px; }
        QPushButton#deleteBtn:hover{ background-color:#DC2626; }
        QPushButton#updateBtn{ background-color:#FBBF24; color:white; border:none; border-radius:4px; font-weight:600; font-size:11px; padding:4px 8px; }
        QPushButton#updateBtn:hover{ background-color:#F59E0B; }
    )";

    // ===== Smooth horizontal scroll =====
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    scrollArea->verticalScrollBar()->setStyleSheet(R"(
        QScrollBar:vertical { width:12px; background:transparent; }
        QScrollBar::handle:vertical{ background:transparent; min-height:20px; border-radius:6px; }
        QScrollBar::handle:vertical:hover{ background:#3B82F6; }
        QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page{ background:none; }
    )");

    window.setStyleSheet(styleSheet);
    window.show();
    return a.exec();
}
