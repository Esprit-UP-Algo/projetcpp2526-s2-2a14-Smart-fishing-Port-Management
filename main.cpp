#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>

// ================= POPUP AJOUT / MODIFIER =================
class AddFrigoDialog : public QDialog
{
    Q_OBJECT

public:
    AddFrigoDialog(QWidget *parent=nullptr):QDialog(parent)
    {
        setWindowTitle("Frigo");
        setFixedSize(350,400);

        QVBoxLayout *layout = new QVBoxLayout(this);

        idEdit   = new QLineEdit();
        capEdit  = new QLineEdit();
        humEdit  = new QLineEdit();
        tempEdit = new QLineEdit();

        statusBox = new QComboBox();
        fishBox   = new QComboBox();

        statusBox->addItems({"Disponible","Occupé"});
        fishBox->addItems({"Sardine","Thon","Merlan","Crevette","Saumon"});

        layout->addWidget(new QLabel("ID Frigo"));
        layout->addWidget(idEdit);

        layout->addWidget(new QLabel("Capacité (Kg)"));
        layout->addWidget(capEdit);

        layout->addWidget(new QLabel("Humidité (%)"));
        layout->addWidget(humEdit);

        layout->addWidget(new QLabel("Température (°C)"));
        layout->addWidget(tempEdit);

        layout->addWidget(new QLabel("Statut"));
        layout->addWidget(statusBox);

        layout->addWidget(new QLabel("Poisson"));
        layout->addWidget(fishBox);

        QPushButton *btnAdd = new QPushButton("Valider");
        QPushButton *btnCancel = new QPushButton("Annuler");

        btnAdd->setStyleSheet("background:#22c55e;color:white;height:35px;border-radius:6px;");
        btnCancel->setStyleSheet("background:#ef4444;color:white;height:35px;border-radius:6px;");

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addWidget(btnAdd);
        btnLayout->addWidget(btnCancel);

        layout->addLayout(btnLayout);

        connect(btnAdd,&QPushButton::clicked,this,&QDialog::accept);
        connect(btnCancel,&QPushButton::clicked,this,&QDialog::reject);
    }

    void setData(QString id, QString cap, QString hum,
                 QString temp, QString stat, QString fish)
    {
        idEdit->setText(id);
        capEdit->setText(cap);
        humEdit->setText(hum);
        tempEdit->setText(temp);

        statusBox->setCurrentText(stat);
        fishBox->setCurrentText(fish);
    }

    QString getId(){ return idEdit->text(); }
    QString getCap(){ return capEdit->text(); }
    QString getHum(){ return humEdit->text(); }
    QString getTemp(){ return tempEdit->text(); }
    QString getStatus(){ return statusBox->currentText(); }
    QString getFish(){ return fishBox->currentText(); }

private:
    QLineEdit *idEdit,*capEdit,*humEdit,*tempEdit;
    QComboBox *statusBox,*fishBox;
};


// ================= DASHBOARD =================
class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    DashboardWindow(QWidget *parent=nullptr):QMainWindow(parent)
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

        QPixmap pix("C:/Users/Lenovo/Downloads/619768662_1784874298868838_2749366332197481779_n-removebg-preview.png"); // change si besoin

        logo->setPixmap(pix.scaled(200,200,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        logo->setAlignment(Qt::AlignCenter);

        logo->setStyleSheet("background:#d1d5db;border-radius:22px;");
        logo->setFixedHeight(100);

        sideLayout->addWidget(logo);

        sideLayout->addWidget(createMenuBtn("🏠 Dashboard"));
        sideLayout->addWidget(createMenuBtn("🌤️ Weather"));
        sideLayout->addWidget(createMenuBtn("⛵ Bateaux"));
        sideLayout->addWidget(createMenuBtn("🐟 Pêche"));
        sideLayout->addWidget(createMenuBtn("👥 Employés"));
        sideLayout->addWidget(createMenuBtn("🧊 Frigos"));
        sideLayout->addWidget(createMenuBtn("⚙️ Paramètres"));

        sideLayout->addStretch();


        // ================= CONTENT =================
        QStackedWidget *pages = new QStackedWidget();

        QWidget *frigoPage = new QWidget();
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


        pages->addWidget(frigoPage);

        mainLayout->addWidget(sidebar);
        mainLayout->addWidget(pages);

        setCentralWidget(central);
    }


private:

    QTableWidget *frigoTable;


    QPushButton* createMenuBtn(QString text)
    {
        QPushButton *btn = new QPushButton(text);

        btn->setFixedHeight(50);

        btn->setStyleSheet(
            "QPushButton{color:#000000;border:none;text-align:left;padding-left:25px;font-size:15px;background:#60a5fa;border-radius:12px;}"
            "QPushButton:hover{background:#1e3f8f;color:white;}"
            );

        return btn;
    }


    // ================= ADD ROW =================
    void addRow(QString id, QString cap, QString hum,
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
};


// ================= MAIN =================
int main(int argc,char *argv[])
{
    QApplication app(argc,argv);

    DashboardWindow win;
    win.show();

    return app.exec();
}

#include "main.moc"
