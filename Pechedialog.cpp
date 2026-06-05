#include "pechedialog.h"
#include <QApplication>
#include <QMap>
#include <QtGlobal>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QTimer>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDoubleValidator>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QAbstractItemView>
#include <QLineEdit>

// ==================== HELPERS POUR DIALOGUE STYLÉ ====================
class ComboClickFilter : public QObject {
public:
    ComboClickFilter(QObject* parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            QLineEdit* le = qobject_cast<QLineEdit*>(watched);
            if (le && le->parentWidget()) {
                QComboBox* cb = qobject_cast<QComboBox*>(le->parentWidget());
                if (cb && event->type() == QEvent::MouseButtonPress) {
                    cb->showPopup();
                    return true;
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

class DialogMoveFilter : public QObject {
public:
    DialogMoveFilter(QDialog* dialog, QObject* parent = nullptr) : QObject(parent), m_dialog(dialog), m_dragging(false) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (!m_dialog) return QObject::eventFilter(watched, event);
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) {
                    m_dragging = true;
                    m_dragOffset = me->globalPosition().toPoint() - m_dialog->frameGeometry().topLeft();
                    return true;
                }
                break;
            }
            case QEvent::MouseMove: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                    m_dialog->move(me->globalPosition().toPoint() - m_dragOffset);
                    return true;
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) { m_dragging = false; return true; }
                break;
            }
            default: break;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    QDialog* m_dialog;
    bool m_dragging;
    QPoint m_dragOffset;
};

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle) {
    if (!dialog || !dragHandle) return;
    dragHandle->setCursor(Qt::OpenHandCursor);
    dragHandle->installEventFilter(new DialogMoveFilter(dialog, dragHandle));
}

PecheDialog::PecheDialog(QWidget *parent, Peche* pecheData)
    : QDialog(parent), pecheData(pecheData), isEdit(pecheData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

PecheDialog::~PecheDialog() {}

void PecheDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Lot de Pêche" : "Nouveau Lot de Pêche");
    setFixedSize(650, 720);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 100));

    QWidget* container = new QWidget(this);
    container->setGeometry(15, 15, 620, 690);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 28px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header avec Dégradé Premium (Bleu Portuaire)
    QFrame* header = new QFrame();
    header->setFixedHeight(130);
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563EB, stop:1 #5D9CEC);
            border-top-left-radius: 28px;
            border-top-right-radius: 28px;
        }
    )");
    
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(35, 0, 25, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    titleCol->setAlignment(Qt::AlignVCenter);

    QLabel* title = new QLabel(isEdit ? "Modifier Lot de Pêche" : "Ajouter un Lot");
    title->setFont(QFont("Segoe UI", 22, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    titleCol->addWidget(title);

    QLabel* sub = new QLabel(isEdit ? "✏️  Modification des données de capture" : "🎣  Enregistrement d'une nouvelle prise en mer");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color: rgba(255, 255, 255, 0.85); background: transparent;");
    titleCol->addWidget(sub);
    headerLay->addLayout(titleCol, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(38, 38);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.15); color: white; border: none;
                      border-radius: 19px; font-size: 15px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.3); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLay->addWidget(closeBtn);

    mainLayout->addWidget(header);
    makeDialogMovable(this, header);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    QWidget* formContent = new QWidget();
    formContent->setStyleSheet("background: transparent;");
    QVBoxLayout* formLayout = new QVBoxLayout(formContent);
    formLayout->setSpacing(18);
    formLayout->setContentsMargins(45, 30, 45, 30);

    auto addLabel = [&](const QString& text) {
        QLabel* lbl = new QLabel(text);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
        lbl->setStyleSheet("color: #4B5563; margin-bottom: 2px; letter-spacing: 0.5px;");
        formLayout->addWidget(lbl);
        return lbl;
    };

    addLabel("RÉFÉRENCE DU LOT");
    referenceInput = new QLineEdit();
    referenceInput->setPlaceholderText("Format: REF-2025-001");
    referenceInput->setStyleSheet(getInputStyle());
    QRegularExpression refPartRegex("^REF(-\\d{0,4}(-\\d{0,6})?)?$");
    referenceInput->setValidator(new QRegularExpressionValidator(refPartRegex, this));
    formLayout->addWidget(referenceInput);
    
    refErrorLabel = new QLabel("");
    refErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    refErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    refErrorLabel->hide();
    formLayout->addWidget(refErrorLabel);

    connect(referenceInput, &QLineEdit::textChanged, this, [=](const QString& t){
        QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
        bool ok = refRegex.match(t.trimmed()).hasMatch();
        updateFieldStyle(referenceInput, ok);
        if(!ok) { refErrorLabel->setText("Format REF-YYYY-ID requis"); refErrorLabel->show(); }
        else refErrorLabel->hide();
    });

    // removed old refErrorLabel creation here as it's added above now

    addLabel("ESPÈCE");
    especeCombo = new QComboBox();
    especeCombo->addItems({"Sardine", "Thon", "Merlan", "Crevette", "Saumon"});
    especeCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(especeCombo);

    addLabel("QUANTITÉ (Kg)");
    quantiteInput = new QLineEdit();
    quantiteInput->setPlaceholderText("Ex: 450.5");
    quantiteInput->setStyleSheet(getInputStyle());
    QDoubleValidator* qteVal = new QDoubleValidator(0.01, 999999.99, 2, this);
    qteVal->setNotation(QDoubleValidator::StandardNotation);
    quantiteInput->setValidator(qteVal);
    formLayout->addWidget(quantiteInput);

    qteErrorLabel = new QLabel("");
    qteErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    qteErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    qteErrorLabel->hide();
    formLayout->addWidget(qteErrorLabel);

    connect(quantiteInput, &QLineEdit::textChanged, this, [=](const QString& t){
        QString valStr = t;
        bool ok; double val = valStr.replace(",",".").toDouble(&ok);
        bool valid = ok && val > 0;
        updateFieldStyle(quantiteInput, valid);
        if(!valid) { qteErrorLabel->setText("Nombre positif requis"); qteErrorLabel->show(); }
        else qteErrorLabel->hide();
    });

    // removed old qteErrorLabel creation here

    addLabel("DATE DE CAPTURE");
    dateInput = new QDateEdit();
    dateInput->setCalendarPopup(true);
    dateInput->setDate(QDate::currentDate());
    dateInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(dateInput);

    addLabel("BATEAU D'ORIGINE");
    boatCombo = new QComboBox();
    boatCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(boatCombo);

    addLabel("DESTINATION DE STOCKAGE (FRIGO)");
    QHBoxLayout* fridgeLay = new QHBoxLayout();
    fridgeCombo = new QComboBox();
    fridgeCombo->setStyleSheet(getInputStyle());
    fridgeLay->addWidget(fridgeCombo, 1);

    autoSelectBtn = new QPushButton("✨ Auto-AI");
    autoSelectBtn->setFixedWidth(120);
    autoSelectBtn->setFixedHeight(48);
    autoSelectBtn->setCursor(Qt::PointingHandCursor);
    autoSelectBtn->setStyleSheet(R"(
        QPushButton { background: #0EA5E9; color: white; border-radius: 12px; font-weight: bold; }
        QPushButton:hover { background: #0284C7; }
    )");
    fridgeLay->addWidget(autoSelectBtn);
    formLayout->addLayout(fridgeLay);

    autoMsgLabel = new QLabel("");
    autoMsgLabel->setStyleSheet("color: #059669; font-size: 9pt; font-weight: bold;");
    autoMsgLabel->hide();
    formLayout->addWidget(autoMsgLabel);

    availableSpaceLabel = new QLabel("");
    availableSpaceLabel->setStyleSheet("color: #64748B; font-size: 9pt; font-weight: 500;");
    formLayout->addWidget(availableSpaceLabel);

    addLabel("PÊCHEUR RESPONSABLE");
    fishermanCombo = new QComboBox();
    fishermanCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(fishermanCombo);

    scrollArea->setWidget(formContent);
    mainLayout->addWidget(scrollArea, 1);

    // Footer avec boutons stylés
    QFrame* footer = new QFrame();
    footer->setFixedHeight(90);
    QHBoxLayout* btnLay = new QHBoxLayout(footer);
    btnLay->setContentsMargins(45, 0, 45, 15);
    btnLay->setSpacing(20);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(50);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #4B5563; border-radius: 15px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLay->addWidget(cancelBtn, 1);

    QPushButton* saveBtn = new QPushButton(isEdit ? "💾  Mettre à jour" : "➕  Enregistrer le Lot");
    saveBtn->setFixedHeight(50);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    saveBtn->setStyleSheet(R"(
        QPushButton { 
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563EB, stop:1 #5D9CEC);
            color: white; border: none; border-radius: 15px; padding: 0 20px;
        }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &PecheDialog::onSave);
    btnLay->addWidget(saveBtn, 2);

    mainLayout->addWidget(footer);

    // Initialisation des données
    QSqlQuery bQuery("SELECT IdBateau, NomBateau FROM BATEAUX");
    while (bQuery.next()) boatCombo->addItem(bQuery.value(1).toString(), bQuery.value(0));

    QSqlQuery eQuery("SELECT ID_EMPLOYE, PRENOM || ' ' || NOM FROM EMPLOYEES WHERE \"POSITION\" = 'Pêcheur'");
    while (eQuery.next()) fishermanCombo->addItem(eQuery.value(1).toString(), eQuery.value(0));

    refreshFridgeCombo(especeCombo->currentText());

    connect(especeCombo, &QComboBox::currentTextChanged, this, [=](const QString &e){
        refreshFridgeCombo(e);
        if (autoMsgLabel) autoMsgLabel->hide();
    });

    connect(autoSelectBtn, &QPushButton::clicked, this, &PecheDialog::autoSelectFridge);
    connect(fridgeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int){ updateAvailableSpaceDisplay(); });
    connect(quantiteInput, &QLineEdit::textChanged, this, [=](const QString&){ updateAvailableSpaceDisplay(); });

    // Fix black popup background on combo dropdowns
    ComboClickFilter* comboFilter = new ComboClickFilter(this);
    for (QComboBox* combo : findChildren<QComboBox*>()) {
        if (!combo->isEditable()) {
            combo->setEditable(true);
            combo->lineEdit()->setReadOnly(true);
            combo->lineEdit()->setCursor(Qt::PointingHandCursor);
            combo->lineEdit()->installEventFilter(comboFilter);
        }
        if (combo->view() && combo->view()->window()) {
            combo->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
            combo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
        }
    }
}

void PecheDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

QString PecheDialog::getInputStyle() const
{
    return R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 10px 15px;
            color: #1F2937;
            font-size: 11pt;
            outline: none;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #EA580C;
            background-color: white;
            outline: none;
        }
        *[state="error"] { border: 2px solid #EF4444; background-color: #FEF2F2; }
        *[state="success"] { border: 2px solid #10B981; }
        QComboBox::drop-down { border: none; width: 30px; outline: none; }
        QComboBox::down-arrow { image: none; border-left: 6px solid transparent; border-right: 6px solid transparent; border-top: 6px solid #6B7280; margin-right: 15px; }
        QComboBox QAbstractItemView {
            background: white;
            color: #1e293b;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            selection-background-color: #EBF5FF;
            selection-color: #1e293b;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 32px;
            padding: 4px 10px;
            border-radius: 4px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #F1F5F9;
        }
    )";
}

void PecheDialog::populateFields()
{
    if (!pecheData) return;
    referenceInput->setText(pecheData->getReference());
    int especeIndex = especeCombo->findText(pecheData->getEspece());
    if (especeIndex >= 0) especeCombo->setCurrentIndex(especeIndex);
    quantiteInput->setText(pecheData->getQuantiteKg());

    QDate dt = QDate::fromString(pecheData->getDateCapture(), "dd/MM/yyyy");
    if (dt.isValid()) dateInput->setDate(dt);

    int bIdx = boatCombo->findData(pecheData->getIdBateau());
    if (bIdx >= 0) boatCombo->setCurrentIndex(bIdx);
    
    int fIdx = fridgeCombo->findData(pecheData->getIdFrigo());
    if (fIdx >= 0) fridgeCombo->setCurrentIndex(fIdx);

    int eIdx = fishermanCombo->findData(pecheData->getIdPecheur());
    if (eIdx >= 0) fishermanCombo->setCurrentIndex(eIdx);
}

void PecheDialog::onSave() { if (validateInputs()) accept(); }

bool PecheDialog::validateInputs()
{
    QString ref = referenceInput->text().trimmed();
    QString qte = quantiteInput->text().trimmed();
    bool isValid = true;

    // Validation Référence
    QRegularExpression fullRegex("^REF-\\d{4}-\\d+$");
    if (ref.isEmpty()) {
        updateFieldStyle(referenceInput, false);
        refErrorLabel->setText("⚠️ La référence ne peut pas être vide.");
        refErrorLabel->show();
        isValid = false;
    } else if (!fullRegex.match(ref).hasMatch()) {
        updateFieldStyle(referenceInput, false);
        refErrorLabel->setText("⚠️ Format invalide! Attendu : REF-annee-nombre (ex: REF-2026-001)");
        refErrorLabel->show();
        isValid = false;
    } else {
        int currentId = -1;
        if (isEdit && pecheData) {
            QString idStr = pecheData->getIdLot();
            currentId = idStr.startsWith("LOT") ? idStr.mid(3).toInt() : idStr.toInt();
        }
        if (Peche::referenceExiste(ref, currentId)) {
            updateFieldStyle(referenceInput, false);
            refErrorLabel->setText("⚠️ Cette référence est déjà utilisée.");
            refErrorLabel->show();
            isValid = false;
        } else {
            updateFieldStyle(referenceInput, true);
            refErrorLabel->hide();
        }
    }

    // Validation Quantité
    bool ok;
    double val = qte.replace(",",".").toDouble(&ok);
    if (qte.isEmpty() || !ok || val <= 0) {
        updateFieldStyle(quantiteInput, false);
        qteErrorLabel->setText("⚠️ La quantité doit être un nombre positif.");
        qteErrorLabel->show();
        isValid = false;
    } else {
        updateFieldStyle(quantiteInput, true);
        qteErrorLabel->hide();
    }

    if (boatCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un bateau.");
        isValid = false;
    }

    if (fridgeCombo->currentIndex() == -1 || fridgeCombo->currentData().toString().isEmpty()) {
        showError(QString("Aucun frigo compatible pour l'espèce '%1'.").arg(especeCombo->currentText()));
        isValid = false;
    } else {
        // Vérification capacité frigo
        QString fid = fridgeCombo->currentData().toString();
        QSqlQuery capQ;
        capQ.prepare("SELECT CAPACITE, OCCUPATION FROM FRIGOS WHERE IDFRIGO = :id");
        capQ.bindValue(":id", fid);
        if (capQ.exec() && capQ.next()) {
            double cap = capQ.value(0).toDouble();
            double occ = capQ.value(1).toDouble();
            double oldQte = (isEdit && pecheData && pecheData->getIdFrigo() == fid) ? pecheData->getQuantiteKg().toDouble() : 0;
            if (occ - oldQte + val > cap) {
                showError(QString("Capacité insuffisante ! Reste : %1 Kg.").arg(cap - occ + oldQte));
                isValid = false;
            }
        }
    }

    if (fishermanCombo->currentIndex() == -1) {
        showError("Veuillez sélectionner un pêcheur.");
        isValid = false;
    }

    return isValid;
}

void PecheDialog::showError(const QString& msg) { QMessageBox::warning(this, "Validation", msg); }

Peche PecheDialog::getData() const
{
    Peche peche;
    if (isEdit && pecheData) peche.setIdLot(pecheData->getIdLot());
    peche.setReference(referenceInput->text().trimmed());
    peche.setEspece(especeCombo->currentText());
    peche.setQuantiteKg(quantiteInput->text().trimmed());
    peche.setDateCapture(dateInput->date().toString("dd/MM/yyyy"));
    peche.setIdBateau(boatCombo->currentData().toString());
    peche.setIdFrigo(fridgeCombo->currentData().toString());
    peche.setIdPecheur(fishermanCombo->currentData().toString());
    return peche;
}

void PecheDialog::refreshFridgeCombo(const QString &espece)
{
    QString currentFrigoId = fridgeCombo->currentData().toString();
    fridgeCombo->clear();

    QSqlQuery fQuery;
    QString currentFid = (isEdit && pecheData) ? pecheData->getIdFrigo() : "";
    
    fQuery.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, OCCUPATION "
                   "FROM FRIGOS "
                   "WHERE UPPER(TYPE_POISSON) = UPPER(:espece) "
                   "AND (STATUT = 'Disponible' OR IDFRIGO = :curid) "
                   "ORDER BY REFERENCE");
    fQuery.bindValue(":curid", currentFid);
    fQuery.bindValue(":espece", espece);

    if (fQuery.exec()) {
        while (fQuery.next()) {
            QString frigoId  = fQuery.value(0).toString();
            QString ref      = fQuery.value(1).toString();
            double  cap      = fQuery.value(2).toDouble();
            double  occ      = fQuery.value(3).toDouble();
            double oldQte = (isEdit && pecheData && pecheData->getIdFrigo() == frigoId) ? pecheData->getQuantiteKg().toDouble() : 0;
            double  libre    = cap - occ + oldQte;
            fridgeCombo->addItem(QString("%1 (%2 kg libres)").arg(ref).arg(libre, 0, 'f', 1), frigoId);
        }
    }

    if (fridgeCombo->count() == 0) {
        fridgeCombo->addItem("⚠️ Aucun frigo compatible", "");
    } else {
        int idx = fridgeCombo->findData(currentFrigoId);
        if (idx >= 0) fridgeCombo->setCurrentIndex(idx);
    }
}

void PecheDialog::autoSelectFridge()
{
    QString espece = especeCombo->currentText();
    bool ok;
    double qteNeeded = quantiteInput->text().toDouble(&ok);
    if (!ok || qteNeeded <= 0) { showError("Saisissez une quantité pour l'IA."); return; }

    QMap<QString, double> idealTemps;
    idealTemps["Sardine"] = -2.0; idealTemps["Thon"] = -18.0; idealTemps["Crevette"]= -20.0;
    idealTemps["Merlan"] = 0.0; idealTemps["Saumon"] = -4.0;
    double targetTemp = idealTemps.value(espece, -5.0);

    QSqlQuery query;
    query.prepare("SELECT IDFRIGO, REFERENCE, CAPACITE, OCCUPATION, TEMPERATURE FROM FRIGOS WHERE UPPER(TYPE_POISSON) = UPPER(:type) AND STATUT = 'Disponible'");
    query.bindValue(":type", espece);

    if (query.exec()) {
        QString bestId; QString bestRef; double minDiff = 999;
        while (query.next()) {
            double free = query.value(2).toDouble() - query.value(3).toDouble();
            if (free >= qteNeeded) {
                double diff = qAbs(query.value(4).toDouble() - targetTemp);
                if (diff < minDiff) { minDiff = diff; bestId = query.value(0).toString(); bestRef = query.value(1).toString(); }
            }
        }
        if (!bestId.isEmpty()) {
            fridgeCombo->setCurrentIndex(fridgeCombo->findData(bestId));
            autoMsgLabel->setText("✨ IA : " + bestRef + " sélectionné.");
            autoMsgLabel->show();
        } else showError("Aucun stockage optimal trouvé.");
    }
}

void PecheDialog::updateAvailableSpaceDisplay()
{
    QString fid = fridgeCombo->currentData().toString();
    if (fid.isEmpty()) { availableSpaceLabel->setText(""); return; }
    QSqlQuery q;
    q.prepare("SELECT CAPACITE, OCCUPATION, REFERENCE FROM FRIGOS WHERE IDFRIGO = :id");
    q.bindValue(":id", fid);
    if (q.exec() && q.next()) {
        double cap = q.value(0).toDouble();
        double occ = q.value(1).toDouble();
        double old = (isEdit && pecheData && pecheData->getIdFrigo() == fid) ? pecheData->getQuantiteKg().toDouble() : 0;
        double available = cap - occ + old;
        availableSpaceLabel->setText(QString("📦 Espace dans %1 : %2 Kg libres").arg(q.value(2).toString()).arg(available, 0, 'f', 1));
    }
}
