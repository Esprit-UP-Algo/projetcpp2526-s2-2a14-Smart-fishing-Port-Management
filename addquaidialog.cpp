#include "addquaidialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QSqlQuery>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDialog>
#include "quai.h"
#include "connection.h"

AddQuaiDialog::AddQuaiDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Ajouter un Quai");
    setFixedSize(560, 640);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    // Shadow
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    // Container
    QWidget* container = new QWidget(this);
    container->setGeometry(10, 10, 540, 620);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // --- Header ---
    QFrame* header = new QFrame();
    header->setFixedHeight(75);
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563EB, stop:1 #5D9CEC);
            border-radius: 20px 20px 0 0;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(25, 0, 18, 0);

    QLabel* titleLbl = new QLabel("➕  Nouveau Quai");
    titleLbl->setFont(QFont("Segoe UI", 15, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 16px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(header);

    // --- Form ---
    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(24, 18, 24, 8);
    formLay->setSpacing(6);

    QString fieldStyle = R"(
        QLineEdit {
            background: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 10px;
            padding: 4px 12px;
            color: #1f2937;
            font-family: 'Segoe UI';
            font-size: 10pt;
        }
        QLineEdit:focus {
            border: 2px solid #2563EB;
            background: white;
        }
    )";

    QString comboStyle = R"(
        QComboBox {
            background: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 10px;
            padding: 4px 12px;
            color: #1f2937;
            font-family: 'Segoe UI';
            font-size: 10pt;
        }
        QComboBox:focus { border: 2px solid #2563EB; background: white; }
        QComboBox::drop-down { border: none; width: 28px; }
        QComboBox QAbstractItemView {
            background: white;
            border: 1px solid #e2e8f0;
            border-radius: 10px;
            selection-background-color: #EFF6FF;
            selection-color: #2563EB;
            padding: 6px;
        }
    )";

    auto addField = [&](const QString& labelText, QWidget* widget) {
        QLabel* lbl = new QLabel(labelText);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");
        formLay->addWidget(lbl);
        formLay->addWidget(widget);
    };

    numeroInput = new QLineEdit();
    numeroInput->setPlaceholderText("ex: 1");
    numeroInput->setFixedHeight(40);
    numeroInput->setStyleSheet(fieldStyle);


    locationInput = new QLineEdit();
    locationInput->setPlaceholderText("ex: Zone A - Secteur Nord");
    locationInput->setFixedHeight(40);
    locationInput->setStyleSheet(fieldStyle);
    addField("Localisation *", locationInput);

    // Capacité + Tarif side by side
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(12);

    QVBoxLayout* capCol = new QVBoxLayout();
    capCol->setSpacing(4);
    QLabel* capLbl = new QLabel("Capacité *");
    capLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    capLbl->setStyleSheet("color: #374151; background: transparent;");
    capaciteInput = new QLineEdit();
    capaciteInput->setPlaceholderText("ex: 3");
    capaciteInput->setFixedHeight(40);
    capaciteInput->setStyleSheet(fieldStyle);
    capCol->addWidget(capLbl);
    capCol->addWidget(capaciteInput);
    row1->addLayout(capCol);

    QVBoxLayout* tarifCol = new QVBoxLayout();
    tarifCol->setSpacing(4);
    QLabel* tarifLbl = new QLabel("Tarif (DT/j) *");
    tarifLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    tarifLbl->setStyleSheet("color: #374151; background: transparent;");
    tarifInput = new QLineEdit();
    tarifInput->setPlaceholderText("ex: 50.00");
    tarifInput->setFixedHeight(40);
    tarifInput->setStyleSheet(fieldStyle);
    tarifCol->addWidget(tarifLbl);
    tarifCol->addWidget(tarifInput);
    row1->addLayout(tarifCol);

    formLay->addLayout(row1);

    // Durée + État side by side
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(12);

    QVBoxLayout* dureeCol = new QVBoxLayout();
    dureeCol->setSpacing(4);
    QLabel* dureeLbl = new QLabel("Durée de location (jours) *");
    dureeLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    dureeLbl->setStyleSheet("color: #374151; background: transparent;");
    dureeInput = new QLineEdit();
    dureeInput->setPlaceholderText("ex: 30");
    dureeInput->setFixedHeight(40);
    dureeInput->setStyleSheet(fieldStyle);
    dureeCol->addWidget(dureeLbl);
    dureeCol->addWidget(dureeInput);
    row2->addLayout(dureeCol);

    QVBoxLayout* etatCol = new QVBoxLayout();
    etatCol->setSpacing(4);
    QLabel* etatLbl = new QLabel("État *");
    etatLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    etatLbl->setStyleSheet("color: #374151; background: transparent;");
    etatInput = new QComboBox();
    etatInput->addItems({"Disponible", "Occupé", "Maintenance"});
    etatInput->setFixedHeight(40);
    etatInput->setStyleSheet(comboStyle);
    etatCol->addWidget(etatLbl);
    etatCol->addWidget(etatInput);
    row2->addLayout(etatCol);

    formLay->addLayout(row2);

    mainLay->addWidget(formArea, 1);

    // --- Buttons ---
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(24, 8, 24, 20);
    btnLay->setSpacing(12);

    cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(42);
    cancelBtn->setMinimumWidth(110);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    saveBtn = new QPushButton("💾  Enregistrer");
    saveBtn->setFixedHeight(42);
    saveBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #2563EB, stop:1 #5D9CEC);
                      color: white; border: none; border-radius: 10px; padding: 0 20px; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &AddQuaiDialog::saveQuai);

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(saveBtn);
    mainLay->addLayout(btnLay);
}

// --- Get Data ---
Quai AddQuaiDialog::getData() const
{
    Quai d;
    d.setNumero(numeroInput->text().toInt());
    d.setLocation(locationInput->text());
    d.setCapacite(capaciteInput->text().toInt());
    d.setTarif(tarifInput->text().toDouble());
    d.setDureeLocation(dureeInput->text()); // <-- QString now
    d.setEtat(etatInput->currentText());
    return d;
}

// --- Save Quai ---
void AddQuaiDialog::saveQuai()
{// Basic validation
    if (
        locationInput->text().trimmed().isEmpty() ||
        capaciteInput->text().trimmed().isEmpty() ||
        tarifInput->text().trimmed().isEmpty() ||
        dureeInput->text().trimmed().isEmpty()) {

        QMessageBox::warning(this, "Champs requis",
                             "Veuillez remplir tous les champs obligatoires (*).");
        return;
    }

    // Convert and validate numeric fields
    bool okCapacite, okTarif, okDuree;

    int capacite = capaciteInput->text().toInt(&okCapacite);
    double tarif = tarifInput->text().toDouble(&okTarif);  // tarif could be float
    int duree = dureeInput->text().toInt(&okDuree);


    if (!okCapacite) {
        QMessageBox::warning(this, "Erreur", "La capacité doit être un nombre entier.");
        return;
    }
    if (!okTarif) {
        QMessageBox::warning(this, "Erreur", "Le tarif doit être un nombre valide.");
        return;
    }
    if (!okDuree) {
        QMessageBox::warning(this, "Erreur", "La durée doit être un nombre entier.");
        return;
    }

    // Check database connection
    if (!Connection::getInstance().createconnect()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données échouée !");
        return;
    }

    Quai d = getData();
    QSqlQuery query;

    // We use NVL(MAX(NUMERO), 0) + 1 to find the next number in the sequence
    query.prepare("INSERT INTO QUAIS (IDQUAI, NUMERO, LOCATION, CAPACITE, TARIF_LOCATION, DUREE_LOCATION, ETAT) "
                  "VALUES (seq_quais.NEXTVAL, "
                  "(SELECT NVL(MAX(NUMERO), 0) + 1 FROM QUAIS), "
                  ":location, :capacite, :tarif, :duree, :etat)");

    // Remove the bindValue for :numero since it's now handled by the subquery
    query.bindValue(":location", d.getLocation());
    query.bindValue(":capacite", d.getCapacite());
    query.bindValue(":tarif",    d.getTarif());
    query.bindValue(":duree",    d.getDureeLocation());
    query.bindValue(":etat",     d.getEtat());

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur",
                              "Impossible d'insérer le quai : " + query.lastError().text());
    } else {
        QMessageBox::information(this, "✅ Succès", "Quai ajouté avec succès !");
        accept();
    }
}
