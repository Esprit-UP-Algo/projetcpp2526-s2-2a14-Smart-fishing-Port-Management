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

    auto createErrorLabel = []() {
        QLabel* l = new QLabel("");
        l->setStyleSheet("color: #EF4444; font-size: 9pt; margin-top: -5px; margin-bottom: 5px; font-weight: 500;");
        l->setVisible(false);
        return l;
    };

    // Ligne 1: Référence
    refEdit = new QLineEdit();
    refEdit->setObjectName("refEdit");
    refEdit->setPlaceholderText("Ex: FRG-001");
    refEdit->setStyleSheet(getInputStyle());
    refEdit->setMinimumHeight(45);
    refError = createErrorLabel();
    grid->addWidget(createLabel("Référence"), 0, 0, 1, 2);
    grid->addWidget(refEdit, 1, 0, 1, 2);
    grid->addWidget(refError, 2, 0, 1, 2);

    // Ligne 2: Capacité et Température
    capEdit = new QLineEdit();
    capEdit->setObjectName("capEdit");
    capEdit->setStyleSheet(getInputStyle());
    capEdit->setMinimumHeight(45);
    capError = createErrorLabel();
    grid->addWidget(createLabel("Capacité (Kg)"), 3, 0);
    grid->addWidget(capEdit, 4, 0);
    grid->addWidget(capError, 5, 0);

    tempEdit = new QLineEdit();
    tempEdit->setObjectName("tempEdit");
    tempEdit->setStyleSheet(getInputStyle());
    tempEdit->setMinimumHeight(45);
    tempError = createErrorLabel();
    grid->addWidget(createLabel("Température (°C)"), 3, 1);
    grid->addWidget(tempEdit, 4, 1);
    grid->addWidget(tempError, 5, 1);

    // Ligne 3: Statut et Type de Poisson
    statusBox = new QComboBox();
    statusBox->setObjectName("statusBox");
    statusBox->addItems({"Disponible", "Occupé", "Maintenance"});
    statusBox->setStyleSheet(getInputStyle());
    statusBox->setMinimumHeight(45);
    statusError = createErrorLabel();
    grid->addWidget(createLabel("Statut"), 6, 0);
    grid->addWidget(statusBox, 7, 0);
    grid->addWidget(statusError, 8, 0);

    fishBox = new QComboBox();
    fishBox->setObjectName("fishBox");
    fishBox->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon", "Sans"});
    fishBox->setStyleSheet(getInputStyle());
    fishBox->setMinimumHeight(45);
    fishError = createErrorLabel();
    grid->addWidget(createLabel("Type de Poisson"), 6, 1);
    grid->addWidget(fishBox, 7, 1);
    grid->addWidget(fishError, 8, 1);

    // Ligne 4: Date de Réservation et Occupation
    dateResEdit = new QDateEdit(QDate::currentDate());
    dateResEdit->setObjectName("dateResEdit");
    dateResEdit->setCalendarPopup(true);
    dateResEdit->setStyleSheet(getInputStyle());
    dateResEdit->setDisplayFormat("dd/MM/yyyy");
    dateResEdit->setMinimumHeight(45);
    dateResError = createErrorLabel();
    grid->addWidget(createLabel("Date Réservation"), 9, 0);
    grid->addWidget(dateResEdit, 10, 0);
    grid->addWidget(dateResError, 11, 0);

    occEdit = new QLineEdit();
    occEdit->setObjectName("occEdit");
    occEdit->setPlaceholderText("Ex: 0");
    occEdit->setStyleSheet(getInputStyle());
    occEdit->setMinimumHeight(45);
    occError = createErrorLabel();
    if (!isEdit) {
        occEdit->setText("0");
    }
    grid->addWidget(createLabel("Occupation (%)"), 9, 1);
    grid->addWidget(occEdit, 10, 1);
    grid->addWidget(occError, 11, 1);

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
    bool isValid = true;
    
    // Helper functionality to reset styles
    auto resetStyle = [this](QWidget* w, QLabel* errorLabel) {
        w->setStyleSheet(getInputStyle());
        errorLabel->setVisible(false);
    };
    
    auto setError = [this](QWidget* w, QLabel* errorLabel, const QString& msg) {
        w->setStyleSheet(getInputStyle() + "border: 2px solid #EF4444; background: #FEF2F2;");
        errorLabel->setText(msg);
        errorLabel->setVisible(true);
    };

    // Reset all
    resetStyle(refEdit, refError);
    resetStyle(capEdit, capError);
    resetStyle(tempEdit, tempError);
    resetStyle(occEdit, occError);
    resetStyle(statusBox, statusError);
    resetStyle(fishBox, fishError);
    resetStyle(dateResEdit, dateResError);

    // Validate Ref
    if (refEdit->text().trimmed().isEmpty()) {
        setError(refEdit, refError, "La référence est obligatoire.");
        isValid = false;
    }

    // Validate Capacity
    bool ok;
    double cap = capEdit->text().toDouble(&ok);
    if (capEdit->text().trimmed().isEmpty()) {
        setError(capEdit, capError, "La capacité est obligatoire.");
        isValid = false;
    } else if (!ok) {
        setError(capEdit, capError, "Nombre requis (ex: 50.5).");
        isValid = false;
    } else if (cap <= 0) {
        setError(capEdit, capError, "La capacité doit être positive.");
        isValid = false;
    }

    // Validate Temp
    double temp = tempEdit->text().toDouble(&ok);
    if (tempEdit->text().trimmed().isEmpty()) {
        setError(tempEdit, tempError, "La température est obligatoire.");
        isValid = false;
    } else if (!ok) {
        setError(tempEdit, tempError, "Nombre requis (ex: -18.0).");
        isValid = false;
    }

    // Validate Occupation
    double occVal = occEdit->text().trimmed().isEmpty() ? 0.0 : occEdit->text().toDouble(&ok);
    if (!occEdit->text().trimmed().isEmpty() && !ok) {
        setError(occEdit, occError, "Nombre requis (0-100).");
        isValid = false;
    } else if (occVal < 0 || occVal > 100) {
        setError(occEdit, occError, "Doit être entre 0 et 100.");
        isValid = false;
    }

    if (isValid) {
        occEdit->setText(QString::number(occVal));
    }

    return isValid;
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
