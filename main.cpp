#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QDateEdit>
#include <QPixmap>

// --- Add/Edit Fishing Entry Dialog ---
class AddFishingDialog : public QDialog
{
    Q_OBJECT
public:
    AddFishingDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Batch Details");
        setFixedSize(350, 500);

        QVBoxLayout *layout = new QVBoxLayout(this);

        refEdit = new QLineEdit();
        fishTypeEdit = new QLineEdit();
        quantityEdit = new QLineEdit();
        fishingDate = new QDateEdit(QDate::currentDate());
        fishingDate->setCalendarPopup(true);

        layout->addWidget(new QLabel("Batch Reference (Ref)"));
        layout->addWidget(refEdit);

        layout->addWidget(new QLabel("Fish Type"));
        layout->addWidget(fishTypeEdit);

        layout->addWidget(new QLabel("Quantity (kg)"));
        layout->addWidget(quantityEdit);

        layout->addWidget(new QLabel("Date"));
        layout->addWidget(fishingDate);

        QPushButton *btnOk = new QPushButton("Confirm");
        QPushButton *btnCancel = new QPushButton("Cancel");

        btnOk->setStyleSheet("background:#22c55e;color:white;height:35px;border-radius:6px;font-weight:bold;");
        btnCancel->setStyleSheet("background:#ef4444;color:white;height:35px;border-radius:6px;font-weight:bold;");

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addWidget(btnOk);
        btnLayout->addWidget(btnCancel);
        layout->addLayout(btnLayout);

        connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }

    void setData(QString ref, QString type, QString quantity, QDate date) {
        refEdit->setText(ref);
        fishTypeEdit->setText(type);
        quantityEdit->setText(quantity);
        fishingDate->setDate(date);
    }

    QString getRef() { return refEdit->text(); }
    QString getFishType() { return fishTypeEdit->text(); }
    QString getQuantity() { return quantityEdit->text(); }
    QDate getDate() { return fishingDate->date(); }

private:
    QLineEdit *refEdit, *fishTypeEdit, *quantityEdit;
    QDateEdit *fishingDate;
};

// --- Main Dashboard Window ---
class FishingDashboard : public QMainWindow
{
    Q_OBJECT
public:
    FishingDashboard(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        resize(1400, 800);
        setWindowTitle("PortFlow - Fishing Batch Management");

        QWidget *central = new QWidget();
        central->setStyleSheet("background:#000000;");
        QHBoxLayout *mainLayout = new QHBoxLayout(central);

        // --- SIDEBAR ---
        QWidget *sidebar = new QWidget();
        sidebar->setFixedWidth(230);
        sidebar->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop:0 #2563eb, stop:1 #60a5fa);"
            "border-radius:22px;"
            );
        QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);

        // Logo with IMAGE and WHITE FRAME
        QLabel *logoLabel = new QLabel();
        QPixmap logoPix("C:/Users/Dhafer/Downloads/logo.png"); // Ensure this path is correct
        if(!logoPix.isNull()) {
            logoLabel->setPixmap(logoPix.scaled(160, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            logoLabel->setText("LOGO"); // Fallback if image not found
        }
        logoLabel->setAlignment(Qt::AlignCenter);
        logoLabel->setStyleSheet("background:#ffffff; border-radius:22px; padding: 5px;");
        logoLabel->setFixedHeight(100);
        sideLayout->addWidget(logoLabel);

        // Menu: Fishing is highlighted
        sideLayout->addWidget(createMenuBtn("🏠 Dashboard", false));
        sideLayout->addWidget(createMenuBtn("⛵ Boats", false));
        sideLayout->addWidget(createMenuBtn("🐟 Fishing", true));
        sideLayout->addWidget(createMenuBtn("👥 Employees", false));
        sideLayout->addStretch();

        // --- CONTENT AREA ---
        QWidget *contentArea = new QWidget();
        QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
        contentLayout->setContentsMargins(20, 15, 20, 15);

        // Header
        QWidget *headerWidget = new QWidget();
        QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);

        QWidget *titleFrame = new QWidget();
        titleFrame->setStyleSheet("background:#d1d5db; border-radius: 8px; border: 2px solid #9ca3af;");
        QHBoxLayout *titleL = new QHBoxLayout(titleFrame);
        QLabel *title = new QLabel("Fishing Batch Management");
        title->setStyleSheet("font-size:16px; font-weight:bold; color:#1e293b; border:none; background:transparent;");
        titleL->addWidget(title);
        headerLayout->addWidget(titleFrame, 0, Qt::AlignLeft);

        QPushButton *btnAdd = new QPushButton("➕ Add New Batch");
        btnAdd->setFixedSize(180, 32);
        btnAdd->setStyleSheet("background:#3b82f6; color:white; font-weight:bold; border-radius:6px; border:none;");
        headerLayout->addWidget(btnAdd, 0, Qt::AlignLeft);

        contentLayout->addWidget(headerWidget);

        // --- TABLE ---
        QWidget *tableContainer = new QWidget();
        tableContainer->setObjectName("tableContainer");
        tableContainer->setStyleSheet(
            "QWidget#tableContainer {"
            "  background:white;"
            "  border: 12px solid #3b82f6;"
            "  border-radius: 10px;"
            "}"
            );
        QVBoxLayout *containerLayout = new QVBoxLayout(tableContainer);

        fishingTable = new QTableWidget();
        fishingTable->setColumnCount(6);
        fishingTable->setHorizontalHeaderLabels({"ID", "Ref", "Fish Type", "Qty (kg)", "Date", "Actions"});
        fishingTable->horizontalHeader()->setStretchLastSection(true);
        fishingTable->verticalHeader()->setVisible(false);
        fishingTable->setEditTriggers(QTableWidget::NoEditTriggers);
        fishingTable->setSelectionBehavior(QTableWidget::SelectRows);
        fishingTable->setStyleSheet(
            "QHeaderView::section { background:#d1d5db; font-weight:bold; color:black; padding:5px; border:1px solid #b0b7c3; }"
            "QTableWidget { background:white; border:none; }"
            );

        containerLayout->addWidget(fishingTable);
        contentLayout->addWidget(tableContainer);

        mainLayout->addWidget(sidebar);
        mainLayout->addWidget(contentArea);
        setCentralWidget(central);

        nextId = 3001;
        addFishingRow("BATCH-001", "Sea Bass", "320", QDate::currentDate());

        connect(btnAdd, &QPushButton::clicked, this, [=](){
            AddFishingDialog d(this);
            if(d.exec() == QDialog::Accepted) {
                addFishingRow(d.getRef(), d.getFishType(), d.getQuantity(), d.getDate());
            }
        });
    }

private:
    QTableWidget *fishingTable;
    int nextId;

    QPushButton* createMenuBtn(QString text, bool isSelected) {
        QPushButton *btn = new QPushButton(text);
        btn->setFixedHeight(50);
        if (isSelected) {
            btn->setStyleSheet("QPushButton{background:#1e3f8f; color:white; border-radius:12px; text-align:left; padding-left:25px; font-weight:bold;}");
        } else {
            btn->setStyleSheet("QPushButton{background:#60a5fa; border-radius:12px; text-align:left; padding-left:25px;}"
                               "QPushButton:hover{background:#1e3f8f; color:white;}");
        }
        return btn;
    }

    void addFishingRow(QString ref, QString type, QString qty, QDate date) {
        int r = fishingTable->rowCount();
        fishingTable->insertRow(r);

        fishingTable->setItem(r, 0, new QTableWidgetItem(QString::number(nextId++)));
        fishingTable->setItem(r, 1, new QTableWidgetItem(ref));
        fishingTable->setItem(r, 2, new QTableWidgetItem(type));
        fishingTable->setItem(r, 3, new QTableWidgetItem(qty));
        fishingTable->setItem(r, 4, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *aLayout = new QHBoxLayout(actionWidget);
        aLayout->setContentsMargins(0,0,0,0);
        aLayout->setSpacing(5);

        QPushButton *btnEdit = new QPushButton("✏️");
        QPushButton *btnDel = new QPushButton("🗑️");
        btnEdit->setFixedSize(28,28); btnDel->setFixedSize(28,28);
        btnEdit->setStyleSheet("background:#facc15; border-radius:6px;");
        btnDel->setStyleSheet("background:#ef4444; border-radius:6px;");

        aLayout->addWidget(btnEdit);
        aLayout->addWidget(btnDel);
        actionWidget->setStyleSheet("background:transparent;");
        fishingTable->setCellWidget(r, 5, actionWidget);

        connect(btnDel, &QPushButton::clicked, this, [=](){
            int row = fishingTable->indexAt(btnDel->parentWidget()->pos()).row();
            if(row >= 0 && QMessageBox::question(this, "Delete", "Remove this batch record?") == QMessageBox::Yes)
                fishingTable->removeRow(row);
        });

        connect(btnEdit, &QPushButton::clicked, this, [=](){
            int row = fishingTable->indexAt(btnEdit->parentWidget()->pos()).row();
            AddFishingDialog d(this);
            d.setData(fishingTable->item(row,1)->text(),
                      fishingTable->item(row,2)->text(),
                      fishingTable->item(row,3)->text(),
                      QDate::fromString(fishingTable->item(row,4)->text(), "yyyy-MM-dd"));

            if(d.exec() == QDialog::Accepted) {
                fishingTable->item(row,1)->setText(d.getRef());
                fishingTable->item(row,2)->setText(d.getFishType());
                fishingTable->item(row,3)->setText(d.getQuantity());
                fishingTable->item(row,4)->setText(d.getDate().toString("yyyy-MM-dd"));
            }
        });
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FishingDashboard win;
    win.show();
    return app.exec();
}

#include "main.moc"
