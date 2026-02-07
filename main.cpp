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
#include <QDateEdit>
#include <QPixmap>

class AddBateauDialog : public QDialog
{
    Q_OBJECT
public:
    AddBateauDialog(QWidget *parent=nullptr):QDialog(parent)
    {
        setWindowTitle("Bateau");
        setFixedSize(350,600);

        QVBoxLayout *layout = new QVBoxLayout(this);

        nomEdit  = new QLineEdit();
        immatEdit= new QLineEdit();
        capEdit  = new QLineEdit();
        longueurEdit = new QLineEdit();
        proprietaireEdit = new QLineEdit();
        etatBox = new QComboBox();
        dispoBox = new QComboBox();
        dateMaint = new QDateEdit(QDate::currentDate());
        dateMaint->setCalendarPopup(true);

        etatBox->addItems({"En mer","Au port","Maintenance"});
        dispoBox->addItems({"Oui","Non"});

        layout->addWidget(new QLabel("Nom"));
        layout->addWidget(nomEdit);

        layout->addWidget(new QLabel("Immatriculation"));
        layout->addWidget(immatEdit);

        layout->addWidget(new QLabel("Capacité (tonnes)"));
        layout->addWidget(capEdit);

        layout->addWidget(new QLabel("Longueur (m)"));
        layout->addWidget(longueurEdit);

        layout->addWidget(new QLabel("Propriétaire"));
        layout->addWidget(proprietaireEdit);

        layout->addWidget(new QLabel("État"));
        layout->addWidget(etatBox);

        layout->addWidget(new QLabel("Date dernière maintenance"));
        layout->addWidget(dateMaint);

        layout->addWidget(new QLabel("Disponible"));
        layout->addWidget(dispoBox);

        QPushButton *btnOk = new QPushButton("Valider");
        QPushButton *btnCancel = new QPushButton("Annuler");

        btnOk->setStyleSheet("background:#22c55e;color:white;height:35px;border-radius:6px;");
        btnCancel->setStyleSheet("background:#ef4444;color:white;height:35px;border-radius:6px;");

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addWidget(btnOk);
        btnLayout->addWidget(btnCancel);
        layout->addLayout(btnLayout);

        connect(btnOk,&QPushButton::clicked,this,&QDialog::accept);
        connect(btnCancel,&QPushButton::clicked,this,&QDialog::reject);
    }

    void setData(QString nom,QString immat,QString cap,QString longueur,QString proprietaire,QString etat,QDate date,QString dispo)
    {
        nomEdit->setText(nom);
        immatEdit->setText(immat);
        capEdit->setText(cap);
        longueurEdit->setText(longueur);
        proprietaireEdit->setText(proprietaire);
        etatBox->setCurrentText(etat);
        dateMaint->setDate(date);
        dispoBox->setCurrentText(dispo);
    }

    QString getNom(){return nomEdit->text();}
    QString getImmat(){return immatEdit->text();}
    QString getCap(){return capEdit->text();}
    QString getLongueur(){return longueurEdit->text();}
    QString getProprietaire(){return proprietaireEdit->text();}
    QString getEtat(){return etatBox->currentText();}
    QDate getDateMaint(){return dateMaint->date();}
    QString getDispo(){return dispoBox->currentText();}

private:
    QLineEdit *nomEdit,*immatEdit,*capEdit,*longueurEdit,*proprietaireEdit;
    QComboBox *etatBox,*dispoBox;
    QDateEdit *dateMaint;
};

class DashboardWindow : public QMainWindow
{
    Q_OBJECT
public:
    DashboardWindow(QWidget *parent=nullptr):QMainWindow(parent)
    {
        resize(1400,800);
        setWindowTitle("PortFlow - Gestion des Bateaux");

        QWidget *central = new QWidget();
        // Fond noir pour toute la fenêtre
        central->setStyleSheet("background:#000000;");
        QHBoxLayout *mainLayout = new QHBoxLayout(central);

        // Sidebar
        QWidget *sidebar = new QWidget();
        sidebar->setFixedWidth(230);
        sidebar->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop:0 #2563eb, stop:0.5 #3b82f6, stop:1 #60a5fa);"
            "border-radius:22px;"
            );
        QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);

        // Logo avec image
        QLabel *logo = new QLabel();
        QPixmap pix("C:/images/portflowlogo.png");
        logo->setPixmap(pix.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

        // Content - Zone droite (fond noir)
        QWidget *contentArea = new QWidget();
        contentArea->setStyleSheet("background:#000000;");
        QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
        contentLayout->setContentsMargins(20,15,20,15);

        // Header vertical: Titre en haut, bouton en dessous (alignés à gauche)
        QWidget *headerWidget = new QWidget();
        headerWidget->setStyleSheet("background:transparent;");
        QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
        headerLayout->setContentsMargins(0,0,0,5);
        headerLayout->setSpacing(8);

        // Cadre gris autour du titre "Gestion des Bateaux"
        QWidget *titleFrame = new QWidget();
        titleFrame->setObjectName("titleFrame");
        titleFrame->setStyleSheet(
            "QWidget#titleFrame {"
            "  background:#d1d5db;"
            "  border: 2px solid #9ca3af;"
            "  border-radius: 8px;"
            "}"
            );
        QHBoxLayout *titleFrameLayout = new QHBoxLayout(titleFrame);
        titleFrameLayout->setContentsMargins(12, 6, 12, 6);

        QLabel *title = new QLabel("Gestion des Bateaux");
        title->setStyleSheet(
            "font-size:16px;"
            "font-weight:bold;"
            "color:#1e293b;"
            "background:transparent;"
            "border:none;"
            );
        titleFrameLayout->addWidget(title);
        headerLayout->addWidget(titleFrame, 0, Qt::AlignLeft);

        // Bouton Ajouter Bateau en bleu — en dessous du titre
        QPushButton *btnAdd = new QPushButton("➕ Ajouter Bateau");
        btnAdd->setStyleSheet(
            "QPushButton {"
            "  background:#3b82f6;"
            "  color:white;"
            "  height:32px;"
            "  font-size:13px;"
            "  font-weight:bold;"
            "  border-radius:6px;"
            "  border: none;"
            "}"
            "QPushButton:hover {"
            "  background:#2563eb;"
            "}"
            );
        btnAdd->setFixedSize(155,32);
        headerLayout->addWidget(btnAdd, 0, Qt::AlignLeft);

        contentLayout->addWidget(headerWidget);

        // Conteneur avec bordure bleu très épaisse et demi-arc
        QWidget *tableContainer = new QWidget();
        tableContainer->setObjectName("tableContainer");
        tableContainer->setStyleSheet(
            "QWidget#tableContainer {"
            "  background:#ffffff;"
            "  border: 12px solid #3b82f6;"
            "  border-radius: 10px;"
            "}"
            );
        QVBoxLayout *containerLayout = new QVBoxLayout(tableContainer);
        containerLayout->setContentsMargins(8,8,8,8);

        // Tableau
        bateauTable = new QTableWidget();
        bateauTable->setColumnCount(10);
        bateauTable->setHorizontalHeaderLabels({
            "ID","Nom","Immatriculation","Capacité","Longueur","Propriétaire","État","Dernière maint","Disponible","Action"
        });
        bateauTable->horizontalHeader()->setStretchLastSection(true);
        bateauTable->verticalHeader()->setVisible(false);
        bateauTable->setEditTriggers(QTableWidget::NoEditTriggers);
        bateauTable->setSelectionBehavior(QTableWidget::SelectRows);
        bateauTable->setFrameShape(QFrame::NoFrame);

        bateauTable->setStyleSheet(
            "QTableWidget {"
            "  background:white;"
            "  border: none;"
            "}"
            "QHeaderView::section {"
            "  background:#d1d5db;"
            "  font-weight:bold;"
            "  padding:5px;"
            "  border: 1px solid #b0b7c3;"
            "  color: black;"
            "}"
            "QTableWidget::item {"
            "  border-bottom: 1px solid #e5e7eb;"
            "  padding: 3px;"
            "}"
            );

        containerLayout->addWidget(bateauTable);
        contentLayout->addWidget(tableContainer);

        // Data initiale
        nextId = 1;
        addRow("Neptune","TN-001","50","20","Ahmed","Au port",QDate::currentDate(),"Oui");
        addRow("Poseidon","TN-002","70","25","Mohamed","En mer",QDate::currentDate(),"Oui");

        // Ajouter un bateau
        connect(btnAdd,&QPushButton::clicked,this,[=](){
            AddBateauDialog d(this);
            if(d.exec()==QDialog::Accepted){
                addRow(d.getNom(),d.getImmat(),d.getCap(),d.getLongueur(),d.getProprietaire(),d.getEtat(),d.getDateMaint(),d.getDispo());
            }
        });

        mainLayout->addWidget(sidebar);
        mainLayout->addWidget(contentArea);
        setCentralWidget(central);
    }

private:
    QTableWidget *bateauTable;
    int nextId;

    QPushButton* createMenuBtn(QString text)
    {
        QPushButton *btn = new QPushButton(text);
        btn->setFixedHeight(50);
        btn->setStyleSheet(
            "QPushButton{background:#60a5fa;border-radius:12px;text-align:left;padding-left:25px;}"
            "QPushButton:hover{background:#1e3f8f;color:white;}"
            );
        return btn;
    }

    void addRow(QString nom,QString immat,QString cap,QString longueur,QString proprietaire,QString etat,QDate date,QString dispo)
    {
        int r = bateauTable->rowCount();
        bateauTable->insertRow(r);

        bateauTable->setItem(r,0,new QTableWidgetItem(QString::number(nextId++)));
        bateauTable->setItem(r,1,new QTableWidgetItem(nom));
        bateauTable->setItem(r,2,new QTableWidgetItem(immat));
        bateauTable->setItem(r,3,new QTableWidgetItem(cap));
        bateauTable->setItem(r,4,new QTableWidgetItem(longueur));
        bateauTable->setItem(r,5,new QTableWidgetItem(proprietaire));
        bateauTable->setItem(r,6,new QTableWidgetItem(etat));
        bateauTable->setItem(r,7,new QTableWidgetItem(date.toString("dd/MM/yyyy")));
        bateauTable->setItem(r,8,new QTableWidgetItem(dispo));

        // Colonne Action avec Modifier et Supprimer
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0,0,0,0);
        actionLayout->setSpacing(5);

        // Modifier (jaune)
        QPushButton *btnEdit = new QPushButton("✏️");
        btnEdit->setStyleSheet(
            "background:#facc15;"
            "color:black;"
            "border-radius:6px;"
            "height:28px;"
            "width:28px;"
            );
        btnEdit->setFixedSize(28,28);

        // Supprimer (rouge)
        QPushButton *btnDelete = new QPushButton("🗑️");
        btnDelete->setStyleSheet(
            "background:#ef4444;"
            "color:white;"
            "border-radius:6px;"
            "height:28px;"
            "width:28px;"
            );
        btnDelete->setFixedSize(28,28);

        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnDelete);
        actionWidget->setStyleSheet("background:transparent;");
        bateauTable->setCellWidget(r,9,actionWidget);

        // Connexion suppression
        connect(btnDelete,&QPushButton::clicked,this,[=](){
            int row = bateauTable->indexAt(btnDelete->parentWidget()->pos()).row();
            if(row >=0 && QMessageBox::question(this,"Confirmation","Supprimer ce bateau ?")==QMessageBox::Yes)
                bateauTable->removeRow(row);
        });

        // Connexion modification
        connect(btnEdit,&QPushButton::clicked,this,[=](){
            int row = bateauTable->indexAt(btnEdit->parentWidget()->pos()).row();
            if(row<0) return;

            AddBateauDialog d(this);
            d.setData(
                bateauTable->item(row,1)->text(),
                bateauTable->item(row,2)->text(),
                bateauTable->item(row,3)->text(),
                bateauTable->item(row,4)->text(),
                bateauTable->item(row,5)->text(),
                bateauTable->item(row,6)->text(),
                QDate::fromString(bateauTable->item(row,7)->text(),"dd/MM/yyyy"),
                bateauTable->item(row,8)->text()
                );

            if(d.exec()==QDialog::Accepted){
                bateauTable->item(row,1)->setText(d.getNom());
                bateauTable->item(row,2)->setText(d.getImmat());
                bateauTable->item(row,3)->setText(d.getCap());
                bateauTable->item(row,4)->setText(d.getLongueur());
                bateauTable->item(row,5)->setText(d.getProprietaire());
                bateauTable->item(row,6)->setText(d.getEtat());
                bateauTable->item(row,7)->setText(d.getDateMaint().toString("dd/MM/yyyy"));
                bateauTable->item(row,8)->setText(d.getDispo());
            }
        });
    }
};

int main(int argc,char *argv[])
{
    QApplication app(argc,argv);
    DashboardWindow win;
    win.show();
    return app.exec();
}

#include "main.moc"
