#include "dashboardwindow.h"
#include "addfrigodialog.h"
#include "employeewindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>

DashboardWindow::DashboardWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1400,800);
    setWindowTitle("PortFlow Dashboard");

    QWidget *central = new QWidget();
    central->setStyleSheet("background:#000000;");

    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // ================= SIDEBAR =================
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(230);
    sidebar->setStyleSheet("background:#3b82f6;border-radius:22px;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);

    QLabel *logo = new QLabel();

    // Use a relative path or resource if possible, but keeping original absolute for now or a placeholder
    // If the image fails it will just be empty as per original code
    QPixmap pix("C:/Users/Lenovo/Downloads/619768662_1784874298868838_2749366332197481779_n-removebg-preview.png"); 

    logo->setPixmap(pix.scaled(200,200,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);

    logo->setStyleSheet("background:#d1d5db;border-radius:22px;");
    logo->setFixedHeight(100);

    sideLayout->addWidget(logo);

    // Navigation Buttons
    QPushButton* btnDashboard = createMenuBtn("🏠 Dashboard");
    connect(btnDashboard, &QPushButton::clicked, this, [=](){ pages->setCurrentWidget(frigoPage); });
    sideLayout->addWidget(btnDashboard);

    sideLayout->addWidget(createMenuBtn("🌤️ Weather"));
    sideLayout->addWidget(createMenuBtn("⛵ Bateaux"));
    sideLayout->addWidget(createMenuBtn("🐟 Pêche"));

    QPushButton* btnEmployees = createMenuBtn("👥 Employés");
    // Connect to switch to employee page instead of opening new window
    // We will initialize employeePage later
    sideLayout->addWidget(btnEmployees);

    QPushButton* btnFrigos = createMenuBtn("🧊 Frigos");
    connect(btnFrigos, &QPushButton::clicked, this, [=](){ pages->setCurrentWidget(frigoPage); });
    sideLayout->addWidget(btnFrigos);

    sideLayout->addWidget(createMenuBtn("⚙️ Paramètres"));

    sideLayout->addStretch();


    // ================= CONTENT =================
    pages = new QStackedWidget();

    frigoPage = new QWidget();
    setupFrigoPage(frigoPage);
    pages->addWidget(frigoPage);

    // Add Employee Page
    employeePage = new EmployeeWindow(this);
    pages->addWidget(employeePage);

    // Connect Employee button now that page exists
    connect(btnEmployees, &QPushButton::clicked, this, [=](){ pages->setCurrentWidget(employeePage); });

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pages);

    setCentralWidget(central);
}

void DashboardWindow::setupFrigoPage(QWidget *frigoPage) {
    QVBoxLayout *frigoLayout = new QVBoxLayout(frigoPage);

    // ================= TOP BAR =================
    QHBoxLayout *topBar = new QHBoxLayout();

    QWidget *titleFrame = new QWidget();
    titleFrame->setStyleSheet("background:#d1d5db;border-radius:10px;");
    titleFrame->setFixedHeight(55);
    titleFrame->setFixedWidth(600);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleFrame);

    QLabel *title = new QLabel("Gestion des Frigos");
    title->setStyleSheet("font-size:22px;font-weight:bold;color:black;");

    titleLayout->addWidget(title);

    topBar->addWidget(titleFrame);
    topBar->addStretch();


    // ===== Barre de recherche =====
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Rechercher...");
    searchEdit->setFixedSize(220,35);

    searchEdit->setStyleSheet(
        "QLineEdit{background:white;  color:black;border-radius:8px;padding-left:10px;font-size:14px;}"
        );

    topBar->addWidget(searchEdit);
    topBar->addSpacing(10);


    // ===== Bouton Ajouter =====
    QPushButton *btnAdd = new QPushButton("➕ Ajouter Frigo");

    btnAdd->setStyleSheet(
        "background:#22c55e;color:white;height:35px;border-radius:8px;padding:0 15px;"
        );

    topBar->addWidget(btnAdd);

    frigoLayout->addLayout(topBar);


    // ================= TABLE FRAME =================
    QWidget *tableFrame = new QWidget();

    tableFrame->setStyleSheet("background:#3b82f6;border-radius:15px;");

    QVBoxLayout *frameLayout = new QVBoxLayout(tableFrame);
    frameLayout->setContentsMargins(20,20,20,20);


    // ================= TABLE =================
    frigoTable = new QTableWidget();

    frigoTable->setColumnCount(7);

    frigoTable->setHorizontalHeaderLabels({
        "ID","Capacité (Kg)","Humidité (%)","Temp (°C)",
        "Statut","Poisson","Actions"
    });

    frigoTable->horizontalHeader()->setStretchLastSection(true);
    frigoTable->verticalHeader()->setVisible(false);

    frigoTable->setEditTriggers(QTableWidget::NoEditTriggers);
    frigoTable->setSelectionBehavior(QTableWidget::SelectRows);

    frigoTable->setStyleSheet(

        "QTableWidget{background:white;color:black;border:none;border-radius:12px;}"

        "QTableWidget::item{background:white;}"

        "QTableWidget QWidget{background:white;}"

        "QHeaderView::section{"
        "background:#d1d5db;color:black;font-weight:bold;padding:6px;border:none;}"

        "QHeaderView::section:first{border-top-left-radius:12px;}"

        "QHeaderView::section:last{border-top-right-radius:12px;}"
        );


    frameLayout->addWidget(frigoTable);
    frigoLayout->addWidget(tableFrame);


    // ================= DATA =================
    addRow("FR01","800","65","-4","Disponible","Sardine");
    addRow("FR02","1200","72","-6","Occupé","Thon");
    addRow("FR03","600","60","-3","Disponible","Crevette");


    // ================= RECHERCHE =================
    connect(searchEdit, &QLineEdit::textChanged, this,
            [=](const QString &text){

                for(int i=0;i<frigoTable->rowCount();i++){

                    bool found = false;

                    for(int j=0;j<6;j++){

                        QTableWidgetItem *item = frigoTable->item(i,j);

                        if(item && item->text().contains(text,Qt::CaseInsensitive)){
                            found = true;
                            break;
                        }
                    }

                    frigoTable->setRowHidden(i,!found);
                }
            });


    // ================= ADD =================
    connect(btnAdd,&QPushButton::clicked,this,[=](){

        AddFrigoDialog dialog(this);

        if(dialog.exec()==QDialog::Accepted){

            addRow(
                dialog.getId(),
                dialog.getCap(),
                dialog.getHum(),
                dialog.getTemp(),
                dialog.getStatus(),
                dialog.getFish()
                );
        }
    });

}

QPushButton* DashboardWindow::createMenuBtn(QString text)
{
    QPushButton *btn = new QPushButton(text);

    btn->setFixedHeight(50);

    btn->setStyleSheet(
        "QPushButton{color:#000000;border:none;text-align:left;padding-left:25px;font-size:15px;background:#60a5fa;border-radius:12px;}"
        "QPushButton:hover{background:#1e3f8f;color:white;}"
        );

    return btn;
}

void DashboardWindow::addRow(QString id, QString cap, QString hum,
            QString temp, QString stat, QString fish)
{
    int r = frigoTable->rowCount();

    frigoTable->insertRow(r);

    frigoTable->setItem(r,0,new QTableWidgetItem(id));
    frigoTable->setItem(r,1,new QTableWidgetItem(cap));
    frigoTable->setItem(r,2,new QTableWidgetItem(hum));
    frigoTable->setItem(r,3,new QTableWidgetItem(temp));
    frigoTable->setItem(r,4,new QTableWidgetItem(stat));
    frigoTable->setItem(r,5,new QTableWidgetItem(fish));


    // ===== Boutons =====
    QPushButton *btnEdit = new QPushButton("✏️");
    btnEdit->setFixedSize(50,28);
    btnEdit->setStyleSheet("background:#facc15;border-radius:6px;");

    QPushButton *btnDelete = new QPushButton("🗑️");
    btnDelete->setFixedSize(50,28);
    btnDelete->setStyleSheet("background:#ef4444;color:white;border-radius:6px;");


    QWidget *btnWidget = new QWidget();
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);

    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDelete);

    btnLayout->setAlignment(Qt::AlignCenter);
    btnLayout->setSpacing(5);
    btnLayout->setContentsMargins(0,0,0,0);

    frigoTable->setCellWidget(r,6,btnWidget);


    // ===== DELETE =====
    connect(btnDelete,&QPushButton::clicked,this,[=](){

        if(QMessageBox::question(this,"Confirmation",
                                  "Supprimer ce frigo ?")==QMessageBox::Yes)
        {
            frigoTable->removeRow(r);
        }
    });


    // ===== EDIT =====
    connect(btnEdit,&QPushButton::clicked,this,[=](){

        AddFrigoDialog dialog(this);

        dialog.setData(
            frigoTable->item(r,0)->text(),
            frigoTable->item(r,1)->text(),
            frigoTable->item(r,2)->text(),
            frigoTable->item(r,3)->text(),
            frigoTable->item(r,4)->text(),
            frigoTable->item(r,5)->text()
            );

        if(dialog.exec()==QDialog::Accepted){

            frigoTable->item(r,0)->setText(dialog.getId());
            frigoTable->item(r,1)->setText(dialog.getCap());
            frigoTable->item(r,2)->setText(dialog.getHum());
            frigoTable->item(r,3)->setText(dialog.getTemp());
            frigoTable->item(r,4)->setText(dialog.getStatus());
            frigoTable->item(r,5)->setText(dialog.getFish());
        }
    });
}

void DashboardWindow::switchPage(QWidget *page)
{
    pages->setCurrentWidget(page);
}
