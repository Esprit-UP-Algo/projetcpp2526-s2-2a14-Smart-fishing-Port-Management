#include "addfrigodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>
#include <QDateEdit>
#include <QDate>
#include <QMessageBox>

AddFrigoDialog::AddFrigoDialog(QWidget *parent, FrigoModel* frigoData)
    : QDialog(parent), frigoData(frigoData), isEdit(frigoData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

AddFrigoDialog::~AddFrigoDialog() {}

void AddFrigoDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Frigo" : "Ajouter Frigo");
    setFixedSize(600, 720);
    setStyleSheet("QDialog { background-color: #F8FAFC; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QFrame* header = new QFrame();
    header->setFixedHeight(100);
    header->setStyleSheet("background-color: #5D9CEC;");
    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    QLabel* title = new QLabel(windowTitle());
    title->setFont(QFont("Segoe UI", 20, QFont::Bold));
    title->setStyleSheet("color: white;");
    title->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(title);
    mainLayout->addWidget(header);

    // Content
    QWidget* content = new QWidget();
    QVBoxLayout* mainContentLayout = new QVBoxLayout(content);
    mainContentLayout->setContentsMargins(40, 30, 40, 30);
    mainContentLayout->setSpacing(25);

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(25);
    grid->setVerticalSpacing(15);

    auto createLabel = [](const QString& text) {
        QLabel* l = new QLabel(text);
        l->setStyleSheet("color: #2C3E50; font-weight: 700; font-size: 11pt; margin-bottom: 2px;");
        return l;
    };

    // Ligne 1: Référence
    refEdit = new QLineEdit();
    refEdit->setPlaceholderText("Ex: FRG-001");
    refEdit->setStyleSheet(getInputStyle());
    refEdit->setMinimumHeight(45);
    grid->addWidget(createLabel("Référence"), 0, 0, 1, 2);
    grid->addWidget(refEdit, 1, 0, 1, 2);

    // Ligne 2: Capacité et Température
    capEdit = new QLineEdit();
    capEdit->setStyleSheet(getInputStyle());
    capEdit->setMinimumHeight(45);
    grid->addWidget(createLabel("Capacité (Kg)"), 2, 0);
    grid->addWidget(capEdit, 3, 0);

    tempEdit = new QLineEdit();
    tempEdit->setStyleSheet(getInputStyle());
    tempEdit->setMinimumHeight(45);
    grid->addWidget(createLabel("Température (°C)"), 2, 1);
    grid->addWidget(tempEdit, 3, 1);

    // Ligne 3: Statut et Type de Poisson
    statusBox = new QComboBox();
    statusBox->addItems({"Disponible", "Occupé", "Maintenance"});
    statusBox->setStyleSheet(getInputStyle());
    statusBox->setMinimumHeight(45);
    grid->addWidget(createLabel("Statut"), 4, 0);
    grid->addWidget(statusBox, 5, 0);

    fishBox = new QComboBox();
    fishBox->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon", "Sans"});
    fishBox->setStyleSheet(getInputStyle());
    fishBox->setMinimumHeight(45);
    grid->addWidget(createLabel("Type de Poisson"), 4, 1);
    grid->addWidget(fishBox, 5, 1);

    // Ligne 4: Date de Réservation et Occupation
    dateResEdit = new QDateEdit(QDate::currentDate());
    dateResEdit->setCalendarPopup(true);
    dateResEdit->setStyleSheet(getInputStyle());
    dateResEdit->setDisplayFormat("dd/MM/yyyy");
    dateResEdit->setMinimumHeight(45);
    grid->addWidget(createLabel("Date Réservation"), 6, 0);
    grid->addWidget(dateResEdit, 7, 0);

    occEdit = new QLineEdit();
    occEdit->setPlaceholderText("Ex: 50.5");
    occEdit->setStyleSheet(getInputStyle());
    occEdit->setMinimumHeight(45);
    grid->addWidget(createLabel("Occupation (%)"), 6, 1);
    grid->addWidget(occEdit, 7, 1);

    mainContentLayout->addLayout(grid);
    mainContentLayout->addStretch();

    // Buttons
    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* cancel = new QPushButton("Annuler");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("background:#e2e8f0; color:#475569; border:none; border-radius:10px; height:45px; font-weight:600;");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    
    QPushButton* save = new QPushButton("Enregistrer");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("background:#5D9CEC; color:white; border:none; border-radius:10px; height:45px; font-weight:600;");
    connect(save, &QPushButton::clicked, this, &AddFrigoDialog::onSave);
    
    btns->addWidget(cancel);
    btns->addWidget(save);
    mainContentLayout->addLayout(btns);

    mainLayout->addWidget(content);
}

void AddFrigoDialog::onSave()
{
    if (validateInputs()) {
        accept();
    }
}

bool AddFrigoDialog::validateInputs()
{
    if (refEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "La référence est obligatoire.");
        return false;
    }

    bool ok;
    capEdit->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Validation", "Capacité invalide (nombre requis).");
        return false;
    }

    tempEdit->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Validation", "Température invalide (nombre requis).");
        return false;
    }

    occEdit->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Validation", "Occupation invalide (nombre requis).");
        return false;
    }

    return true;
}

QString AddFrigoDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background: #F8FAFC; border: 2px solid #E2E8F0; border-radius: 12px; padding: 10px 15px; font-size: 12pt; color: #1E293B;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 2px solid #5D9CEC; background: white; }
        
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #64748B;
            margin-right: 15px;
        }
        QDateEdit::drop-down {
            border: none;
            width: 30px;
        }
        QDateEdit::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #64748B;
            margin-right: 15px;
        }
    )";
}

void AddFrigoDialog::populateFields()
{
    if (!frigoData) return;
    refEdit->setText(frigoData->getRef());
    capEdit->setText(QString::number(frigoData->getCap()));
    tempEdit->setText(QString::number(frigoData->getTemp()));
    occEdit->setText(QString::number(frigoData->getOcc()));
    statusBox->setCurrentText(frigoData->getStat());
    fishBox->setCurrentText(frigoData->getType());
    
    QDate dt = QDate::fromString(frigoData->getDateRes(), "dd/MM/yyyy");
    if (dt.isValid()) dateResEdit->setDate(dt);
}

FrigoModel AddFrigoDialog::getData() const
{
    return FrigoModel("", refEdit->text(), capEdit->text().toDouble(), 
                      fishBox->currentText(), statusBox->currentText(), 
                      dateResEdit->date().toString("dd/MM/yyyy"), 
                      tempEdit->text().toDouble(), occEdit->text().toDouble());
}

// Legacy methods kept for build compatibility if needed
void AddFrigoDialog::setData(QString, QString, QString, QString, QString, QString) {}
QString AddFrigoDialog::getId() { return ""; }
QString AddFrigoDialog::getCap() { return capEdit->text(); }
QString AddFrigoDialog::getHum() { return "0"; }
QString AddFrigoDialog::getTemp() { return tempEdit->text(); }
QString AddFrigoDialog::getStatus() { return statusBox->currentText(); }
QString AddFrigoDialog::getFish() { return fishBox->currentText(); }
