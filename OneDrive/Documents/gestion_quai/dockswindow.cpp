#include "dockswindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QDebug>

docksWindow::docksWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1400, 800);
    setWindowTitle("Smart Fishing Port");

    QWidget *central = new QWidget();
    central->setStyleSheet("background:#F8FAFC;");
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // ===== Sidebar =====
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet("background:#DBEAFE;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(15,30,15,15);
    sideLayout->setSpacing(8);

    // Logo
    QLabel *logo = new QLabel();
    QPixmap logoPix("C:/Users/yoser/Downloads/logo_portflow-removebg-preview.png");
    if(!logoPix.isNull())
        logo->setPixmap(logoPix.scaled(180,180,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logo->setText("🚢");
    logo->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(logo);

    // Sidebar Buttons
    QList<QString> btnNames = {"Docks","Cold Storage","Delivery","Fishbatch","Boat Management"};
    QList<QPushButton*> buttons;
    for(const auto &name : btnNames){
        QPushButton* b = new QPushButton(" " + name);
        b->setCursor(Qt::PointingHandCursor);
        b->setObjectName("sidebarBtn");
        sideLayout->addWidget(b);
        buttons.append(b);
    }
    sideLayout->addStretch();

    QPushButton* logoutBtn = new QPushButton(" Logout");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setObjectName("sidebarBtn");
    logoutBtn->setStyleSheet("text-align:left; padding-left:18px;");
    sideLayout->addWidget(logoutBtn);

    mainLayout->addWidget(sidebar);

    // ===== Main Content =====
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *contentWidget = new QWidget();
    scrollArea->setWidget(contentWidget);

    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30,30,30,30);
    contentLayout->setSpacing(25);

    // Page Title
    QLabel *pageTitle = new QLabel("Dashboard Overview");
    pageTitle->setObjectName("pageTitle");
    contentLayout->addWidget(pageTitle);

    // ===== Dashboard Cards =====
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    auto createCard = [](QString title, QString value, QString color){
        QFrame* card = new QFrame();
        card->setStyleSheet(QString("background:white; border-radius:12px; border-left:4px solid %1;").arg(color));

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(15); shadow->setXOffset(0); shadow->setYOffset(3); shadow->setColor(QColor(0,0,0,30));
        card->setGraphicsEffect(shadow);

        QVBoxLayout* layout = new QVBoxLayout(card);
        layout->setContentsMargins(20,18,20,18);
        QLabel* val = new QLabel(value); val->setStyleSheet("font-size:32px; font-weight:700;");
        QLabel* tit = new QLabel(title); tit->setStyleSheet("font-size:13px; color:#64748B; text-transform:uppercase;");
        layout->addWidget(val);
        layout->addWidget(tit);
        layout->addStretch();
        return card;
    };

    cardsLayout->addWidget(createCard("Total Docks","12","#3B82F6"));
    cardsLayout->addWidget(createCard("Available","5","#10B981"));
    cardsLayout->addWidget(createCard("Occupied","6","#F59E0B"));
    cardsLayout->addWidget(createCard("Today's Revenue","€450","#8B5CF6"));
    contentLayout->addLayout(cardsLayout);

    // ===== Dock Table =====
    QLabel* tableTitle = new QLabel("Dock Management");
    tableTitle->setObjectName("sectionTitle");
    contentLayout->addWidget(tableTitle);

    table = new QTableWidget();
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"ID","Dock Name","Capacity","Max Size","Status","Rate","Current Client","Actions"});
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    table->setRowCount(12);
    for(int row=0; row<12; ++row)
        table->setRowHeight(row,60);

    contentLayout->addWidget(table);
    mainLayout->addWidget(scrollArea,1);

    // ===== Sidebar Button Logic =====
    for(auto b: buttons){
        QObject::connect(b,&QPushButton::clicked,[=](){
            for(auto btn: buttons) btn->setProperty("active",false), btn->style()->unpolish(btn), btn->style()->polish(btn);
            b->setProperty("active",true);
            b->style()->unpolish(b); b->style()->polish(b);
        });
    }

    // ===== Styles =====
    QString style = R"(
        QPushButton#sidebarBtn{ background-color:transparent; color:#1E40AF; text-align:left; padding:14px 18px; font-size:15px; border-radius:10px; }
        QPushButton#sidebarBtn:hover{ background-color:rgba(59,130,246,0.2); }
        QPushButton#sidebarBtn[active="true"]{ background-color:#3B82F6; color:white; }
        QLabel#pageTitle{ font-size:28px; font-weight:700; color:#0F172A; }
        QLabel#sectionTitle{ font-size:20px; font-weight:600; color:#1E293B; }
    )";
    setStyleSheet(style);
}
