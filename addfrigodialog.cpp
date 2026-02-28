#include "addfrigodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QScrollArea>

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
    setFixedSize(500, 650);
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
    QVBoxLayout* form = new QVBoxLayout(content);
    form->setSpacing(15);
    form->setContentsMargins(30, 30, 30, 30);

    auto addLabel = [&](QString text) {
        QLabel* l = new QLabel(text);
        l->setStyleSheet("color: #64748b; font-weight: 600; font-size: 11pt;");
        form->addWidget(l);
    };

    addLabel("Référence");
    refEdit = new QLineEdit();
    refEdit->setPlaceholderText("Ex: FRG-001");
    refEdit->setStyleSheet(getInputStyle());
    form->addWidget(refEdit);

    QHBoxLayout* row1 = new QHBoxLayout();
    QVBoxLayout* capCol = new QVBoxLayout();
    QLabel* l1 = new QLabel("Capacité (Kg)");
    l1->setStyleSheet("color: #64748b; font-weight: 600;");
    capCol->addWidget(l1);
    capEdit = new QLineEdit();
    capEdit->setStyleSheet(getInputStyle());
    capCol->addWidget(capEdit);
    row1->addLayout(capCol);

    QVBoxLayout* tempCol = new QVBoxLayout();
    QLabel* l2 = new QLabel("Température (°C)");
    l2->setStyleSheet("color: #64748b; font-weight: 600;");
    tempCol->addWidget(l2);
    tempEdit = new QLineEdit();
    tempEdit->setStyleSheet(getInputStyle());
    tempCol->addWidget(tempEdit);
    row1->addLayout(tempCol);
    form->addLayout(row1);

    addLabel("Statut");
    statusBox = new QComboBox();
    statusBox->addItems({"Disponible", "Occupé", "Maintenance"});
    statusBox->setStyleSheet(getInputStyle());
    form->addWidget(statusBox);

    addLabel("Type de Poisson");
    fishBox = new QComboBox();
    fishBox->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon", "Sans"});
    fishBox->setStyleSheet(getInputStyle());
    form->addWidget(fishBox);

    addLabel("Occupation (%)");
    occEdit = new QLineEdit();
    occEdit->setPlaceholderText("0.0");
    occEdit->setStyleSheet(getInputStyle());
    form->addWidget(occEdit);

    form->addStretch();

    // Buttons
    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* cancel = new QPushButton("Annuler");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("background:#e2e8f0; color:#475569; border:none; border-radius:10px; height:45px; font-weight:600;");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    
    QPushButton* save = new QPushButton("Enregistrer");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("background:#5D9CEC; color:white; border:none; border-radius:10px; height:45px; font-weight:600;");
    connect(save, &QPushButton::clicked, this, &QDialog::accept);
    
    btns->addWidget(cancel);
    btns->addWidget(save);
    form->addLayout(btns);

    mainLayout->addWidget(content);
}

QString AddFrigoDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox {
            background: white; border: 1.5px solid #e2e8f0; border-radius: 10px; padding: 10px; font-size: 11pt; color: #1e293b;
        }
        QLineEdit:focus, QComboBox:focus { border: 1.5px solid #2563eb; }
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
}

FrigoModel AddFrigoDialog::getData() const
{
    return FrigoModel(0, refEdit->text(), capEdit->text().toDouble(), 
                      tempEdit->text().toDouble(), statusBox->currentText(), 
                      fishBox->currentText(), occEdit->text().toDouble());
}

// Legacy methods kept for build compatibility if needed
void AddFrigoDialog::setData(QString, QString, QString, QString, QString, QString) {}
QString AddFrigoDialog::getId() { return ""; }
QString AddFrigoDialog::getCap() { return capEdit->text(); }
QString AddFrigoDialog::getHum() { return "0"; }
QString AddFrigoDialog::getTemp() { return tempEdit->text(); }
QString AddFrigoDialog::getStatus() { return statusBox->currentText(); }
QString AddFrigoDialog::getFish() { return fishBox->currentText(); }
