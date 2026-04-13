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
#include <QDialog>
#include <QRandomGenerator>
#include "quai.h"
#include "connection.h"

static int generateUniqueQuaiNumero()
{
    QSqlQuery checkQuery;

    while (true) {
        const int candidate = QRandomGenerator::global()->bounded(100000, 1000000);
        checkQuery.prepare("SELECT COUNT(*) FROM QUAIS WHERE NUMERO = :numero");
        checkQuery.bindValue(":numero", candidate);

        if (!checkQuery.exec()) {
            return candidate;
        }

        if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
            return candidate;
        }
    }
}

void AddQuaiDialog::updateGeneratedReference()
{
    if (numeroInput->text().trimmed().isEmpty()) {
        numeroInput->setText(QString::number(generateUniqueQuaiNumero()));
    }

    Quai q;
    q.setNumero(numeroInput->text().toInt());
    if (referenceValueLabel) {
        referenceValueLabel->setText(q.getReference());
    }
}

// ---------------------------------------------------------------------------
// applyFieldState  (mirrors the pattern in your edit dialog)
// ---------------------------------------------------------------------------
static void applyFieldState(QLineEdit* field, QLabel* hint,
                            bool isEmpty, bool isValid,
                            const QString& neutralMsg,
                            const QString& errorMsg,
                            const QString& successMsg)
{
    QString fieldBase = R"(
        QLineEdit {
            background: #F9FAFB;
            border: 2px solid %1;
            border-radius: 10px;
            padding: 4px 12px;
            color: #1f2937;
            font-family: 'Segoe UI';
            font-size: 10pt;
        }
        QLineEdit:focus { border: 2px solid %1; background: white; }
    )";

    if (isEmpty) {
        field->setStyleSheet(fieldBase.arg("#E5E7EB"));
        hint->setText(neutralMsg);
        hint->setStyleSheet("color: #9CA3AF; font-size: 8pt; background: transparent;");
    } else if (!isValid) {
        field->setStyleSheet(fieldBase.arg("#EF4444"));
        hint->setText("⚠  " + errorMsg);
        hint->setStyleSheet("color: #EF4444; font-size: 8pt; background: transparent;");
    } else {
        field->setStyleSheet(fieldBase.arg("#22C55E"));
        hint->setText("✓  " + successMsg);
        hint->setStyleSheet("color: #22C55E; font-size: 8pt; background: transparent;");
    }
}

static QLabel* locationHint = nullptr;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
AddQuaiDialog::AddQuaiDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Ajouter un Quai");
    setFixedSize(560, 640);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    // --- Shadow ---
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    // --- Main container ---
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

    // --- Shared styles ---
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
        QLineEdit:focus { border: 2px solid #2563EB; background: white; }
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
            background: white; border: 1px solid #e2e8f0; border-radius: 10px;
            selection-background-color: #EFF6FF; selection-color: #2563EB; padding: 6px;
        }
    )";

    // makeColumn: label + field + hint label stacked, returns the hint so signals can use it
    auto makeColumn = [&](const QString& labelText, QWidget* field,
                          QLabel*& hintOut, const QString& neutralHint) -> QVBoxLayout*
    {
        QVBoxLayout* col = new QVBoxLayout();
        col->setSpacing(2);

        QLabel* lbl = new QLabel(labelText);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");

        hintOut = new QLabel(neutralHint);
        hintOut->setFont(QFont("Segoe UI", 8));
        hintOut->setStyleSheet("color: #9CA3AF; font-size: 8pt; background: transparent;");

        col->addWidget(lbl);
        col->addWidget(field);
        col->addWidget(hintOut);
        return col;
    };

    // --- Form area ---
    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(24, 18, 24, 8);
    formLay->setSpacing(10);

    // Hidden numero input (auto-generated by app)
    numeroInput = new QLineEdit();
    referenceValueLabel = new QLabel();
    referenceValueLabel->setFixedHeight(40);
    referenceValueLabel->setAlignment(Qt::AlignCenter);
    referenceValueLabel->setStyleSheet(
        "QLabel { background: #EFF6FF; color: #1D4ED8; border: 2px dashed #93C5FD; border-radius: 10px; font-weight: bold; }");
    formLay->addLayout(makeColumn("Référence *", referenceValueLabel, locationHint,
                                  "Référence générée automatiquement"));
    updateGeneratedReference();

    // --- Localisation (full width) ---
    locationInput = new QLineEdit();
    locationInput->setPlaceholderText("ex: Zone A - Secteur Nord");
    locationInput->setFixedHeight(40);
    locationInput->setStyleSheet(fieldStyle);

    locationHint = nullptr;
    formLay->addLayout(makeColumn("Localisation *", locationInput, locationHint,
                                  "Texte décrivant la zone du quai"));

    connect(locationInput, &QLineEdit::textChanged, this, [=](const QString& t) {
        applyFieldState(locationInput, locationHint,
                        t.trimmed().isEmpty(), !t.trimmed().isEmpty(),
                        "Texte décrivant la zone du quai",
                        "Ce champ est obligatoire",
                        "Localisation valide ✓");
    });

    // --- Row 1: Capacité + Tarif ---
    capaciteInput = new QLineEdit();
    capaciteInput->setPlaceholderText("ex: 3");
    capaciteInput->setFixedHeight(40);
    capaciteInput->setStyleSheet(fieldStyle);

    tarifInput = new QLineEdit();
    tarifInput->setPlaceholderText("ex: 50.00");
    tarifInput->setFixedHeight(40);
    tarifInput->setStyleSheet(fieldStyle);

    QLabel* capaciteHint;
    QLabel* tarifHint;

    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(12);
    row1->addLayout(makeColumn("Capacité *",     capaciteInput, capaciteHint, "Nombre entier supérieur à 0"));
    row1->addLayout(makeColumn("Tarif (DT/j) *", tarifInput,    tarifHint,    "Nombre positif, ex: 50.00"));
    formLay->addLayout(row1);

    connect(capaciteInput, &QLineEdit::textChanged, this, [=](const QString& t) {
        bool ok; int val = t.toInt(&ok);
        applyFieldState(capaciteInput, capaciteHint,
                        t.isEmpty(), (ok && val > 0),
                        "Nombre entier supérieur à 0",
                        "Doit être un entier > 0",
                        "Capacité valide ✓");
    });

    connect(tarifInput, &QLineEdit::textChanged, this, [=](const QString& t) {
        bool ok; double val = t.toDouble(&ok);
        applyFieldState(tarifInput, tarifHint,
                        t.isEmpty(), (ok && val > 0),
                        "Nombre positif, ex: 50.00",
                        "Doit être un nombre > 0",
                        "Tarif valide ✓");
    });

    // --- Row 2: Durée + État ---
    dureeInput = new QLineEdit();
    dureeInput->setPlaceholderText("ex: 30");
    dureeInput->setFixedHeight(40);
    dureeInput->setStyleSheet(fieldStyle);

    etatInput = new QComboBox();
    etatInput->addItems({"Disponible", "Occupé", "Maintenance"});
    etatInput->setFixedHeight(40);
    etatInput->setStyleSheet(comboStyle);

    QLabel* dureeHint;
    // État is always valid (combo), just add a spacer hint so alignment matches other columns
    QLabel* etatHint = new QLabel("");
    etatHint->setStyleSheet("background: transparent;");

    auto makeDureeCol = [&]() -> QVBoxLayout* {
        QVBoxLayout* col = new QVBoxLayout();
        col->setSpacing(2);
        QLabel* lbl = new QLabel("Durée de location (jours) *");
        lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");
        dureeHint = new QLabel("Nombre entier supérieur à 0");
        dureeHint->setFont(QFont("Segoe UI", 8));
        dureeHint->setStyleSheet("color: #9CA3AF; font-size: 8pt; background: transparent;");
        col->addWidget(lbl);
        col->addWidget(dureeInput);
        col->addWidget(dureeHint);
        return col;
    };

    auto makeEtatCol = [&]() -> QVBoxLayout* {
        QVBoxLayout* col = new QVBoxLayout();
        col->setSpacing(2);
        QLabel* lbl = new QLabel("État *");
        lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");
        col->addWidget(lbl);
        col->addWidget(etatInput);
        col->addWidget(etatHint);   // spacer so rows align
        return col;
    };

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(12);
    row2->addLayout(makeDureeCol());
    row2->addLayout(makeEtatCol());
    formLay->addLayout(row2);

    connect(dureeInput, &QLineEdit::textChanged, this, [=](const QString& t) {
        bool ok; int val = t.toInt(&ok);
        applyFieldState(dureeInput, dureeHint,
                        t.isEmpty(), (ok && val > 0),
                        "Nombre entier supérieur à 0",
                        "Doit être un entier > 0",
                        "Durée valide ✓");
    });

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

// ---------------------------------------------------------------------------
// getData
// ---------------------------------------------------------------------------
Quai AddQuaiDialog::getData() const
{
    Quai d;
    d.setNumero(numeroInput->text().toInt());
    d.setLocation(locationInput->text().trimmed());
    d.setCapacite(capaciteInput->text().toInt());
    d.setTarif(tarifInput->text().toDouble());
    d.setDureeLocation(dureeInput->text().trimmed());
    d.setEtat(etatInput->currentText());
    return d;
}

// ---------------------------------------------------------------------------
// saveQuai
// Fires all textChanged signals so hints paint themselves, then blocks if
// anything is still invalid — no popup, no close.
// ---------------------------------------------------------------------------
void AddQuaiDialog::saveQuai()
{
    // Trigger live validators on every field so hints update even for untouched fields
    emit locationInput->textChanged(locationInput->text());
    emit capaciteInput->textChanged(capaciteInput->text());
    emit tarifInput->textChanged(tarifInput->text());
    emit dureeInput->textChanged(dureeInput->text());

    // Mirror the same rules used in the live validators
    bool locOk = !locationInput->text().trimmed().isEmpty();

    bool capOk; int  capVal = capaciteInput->text().toInt(&capOk);
    capOk = capOk && capVal > 0;

    bool tarOk; double tarVal = tarifInput->text().toDouble(&tarOk);
    tarOk = tarOk && tarVal > 0;

    bool durOk; int durVal = dureeInput->text().toInt(&durOk);
    durOk = durOk && durVal > 0;

    // If anything is invalid, hints are already red — just stop here (no close, no popup)
    if (!locOk || !capOk || !tarOk || !durOk)
        return;

    // --- DB insert ---
    if (!Connection::getInstance().createconnect()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données échouée !");
        return;
    }

    updateGeneratedReference();
    Quai d = getData();
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO QUAIS (IDQUAI, NUMERO, LOCATION, CAPACITE, TARIF_LOCATION, DUREE_LOCATION, ETAT)
        VALUES (seq_quais.NEXTVAL,
                :numero,
                :location, :capacite, :tarif, :duree, :etat)
    )");

    query.bindValue(":numero",   d.getNumero());
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
