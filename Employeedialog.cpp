#include "employeedialog.h"
#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QDate>
#include <QScrollArea>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateEdit>
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

EmployeeDialog::EmployeeDialog(QWidget *parent, Employee* employeeData)
    : QDialog(parent), employeeData(employeeData), isEdit(employeeData != nullptr)
{
    setupUi();
    if (isEdit) populateFields();
}

EmployeeDialog::~EmployeeDialog() {}

void EmployeeDialog::setupUi()
{
    setWindowTitle(isEdit ? "Modifier Employé" : "Nouvel Employé");
    setFixedSize(680, 750);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 100));

    QWidget* container = new QWidget(this);
    container->setGeometry(15, 15, 650, 720);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 28px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header avec Dégradé Vert (Emerald)
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

    QLabel* title = new QLabel(isEdit ? "Modifier le Profil" : "Nouvel Employé");
    title->setFont(QFont("Segoe UI", 22, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    titleCol->addWidget(title);

    QLabel* sub = new QLabel(isEdit ? "✏️  Mise à jour des informations personnelles" : "👥  Enregistrement d'un nouveau membre de l'équipe");
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

    addLabel("PRÉNOM");
    firstNameInput = new QLineEdit();
    firstNameInput->setPlaceholderText("Ahmed");
    firstNameInput->setStyleSheet(getInputStyle());
    firstNameInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-ZÀ-ÿ\\s-]*$"), this));
    formLayout->addWidget(firstNameInput);
    firstNameErrorLabel = new QLabel("");
    firstNameErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    firstNameErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    firstNameErrorLabel->hide();
    formLayout->addWidget(firstNameErrorLabel);
    connect(firstNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateFirstName);

    addLabel("NOM");
    lastNameInput = new QLineEdit();
    lastNameInput->setPlaceholderText("Khalil");
    lastNameInput->setStyleSheet(getInputStyle());
    lastNameInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-ZÀ-ÿ\\s-]*$"), this));
    formLayout->addWidget(lastNameInput);
    lastNameErrorLabel = new QLabel("");
    lastNameErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    lastNameErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    lastNameErrorLabel->hide();
    formLayout->addWidget(lastNameErrorLabel);
    connect(lastNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateLastName);

    addLabel("NUMÉRO CIN (8 CHIFFRES)");
    cinInput = new QLineEdit();
    cinInput->setPlaceholderText("12345678");
    cinInput->setMaxLength(8);
    cinInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,8}$"), this));
    cinInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(cinInput);
    cinErrorLabel = new QLabel("");
    cinErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    cinErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    cinErrorLabel->hide();
    formLayout->addWidget(cinErrorLabel);
    connect(cinInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateCin);

    addLabel("TÉLÉPHONE");
    phoneInput = new QLineEdit();
    phoneInput->setPlaceholderText("98765432");
    phoneInput->setMaxLength(8);
    phoneInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,8}$"), this));
    phoneInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(phoneInput);
    phoneErrorLabel = new QLabel("");
    phoneErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    phoneErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    phoneErrorLabel->hide();
    formLayout->addWidget(phoneErrorLabel);
    connect(phoneInput, &QLineEdit::textChanged, this, &EmployeeDialog::validatePhone);

    addLabel("EMAIL PROFESSIONNEL");
    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("nom@gmail.com");
    emailInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(emailInput);
    emailErrorLabel = new QLabel("");
    emailErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    emailErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    emailErrorLabel->hide();
    formLayout->addWidget(emailErrorLabel);
    connect(emailInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateEmail);

    QHBoxLayout* posRow = new QHBoxLayout();
    QVBoxLayout* pCol = new QVBoxLayout();
    addLabel("POSITION");
    positionCombo = new QComboBox();
    positionCombo->addItems({"Marin", "Pêcheur", "RH", "Technicien", "Sécurité", "Livreur"});
    positionCombo->setStyleSheet(getInputStyle());
    pCol->addWidget(positionCombo);
    posRow->addLayout(pCol);

    QVBoxLayout* sCol = new QVBoxLayout();
    addLabel("SALAIRE (DT)");
    salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("1200");
    salaryInput->setStyleSheet(getInputStyle());
    salaryInput->setValidator(new QIntValidator(0, 100000, this));
    sCol->addWidget(salaryInput);
    salaryErrorLabel = new QLabel("");
    salaryErrorLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    salaryErrorLabel->setStyleSheet("color: #EF4444; margin-top: 2px; margin-bottom: 5px;");
    salaryErrorLabel->hide();
    sCol->addWidget(salaryErrorLabel);
    connect(salaryInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateSalary);
    posRow->addLayout(sCol);
    formLayout->addLayout(posRow);

    addLabel("DATE DE RECRUTEMENT");
    dateInput = new QDateEdit();
    dateInput->setCalendarPopup(true);
    dateInput->setDate(QDate::currentDate());
    dateInput->setStyleSheet(getInputStyle());
    formLayout->addWidget(dateInput);

    addLabel("STATUT DU COMPTE");
    statusCombo = new QComboBox();
    statusCombo->addItems({"Actif", "Congé", "Inactif"});
    statusCombo->setStyleSheet(getInputStyle());
    formLayout->addWidget(statusCombo);

    addLabel("RFID UID");
    QHBoxLayout* rfidLayout = new QHBoxLayout();
    rfidInput = new QLineEdit();
    rfidInput->setPlaceholderText("XX:XX:XX:XX");
    rfidInput->setStyleSheet(getInputStyle());
    rfidLayout->addWidget(rfidInput, 1);

    scanBtn = new QPushButton("Scanner Carte");
    scanBtn->setFixedWidth(140);
    scanBtn->setFixedHeight(48);
    scanBtn->setCursor(Qt::PointingHandCursor);
    scanBtn->setStyleSheet(R"(
        QPushButton { background: #2563EB; color: white; border-radius: 12px; font-weight: bold; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(scanBtn, &QPushButton::clicked, this, &EmployeeDialog::onScanRfid);
    rfidLayout->addWidget(scanBtn);
    formLayout->addLayout(rfidLayout);

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

    QPushButton* saveBtn = new QPushButton(isEdit ? "💾  Mettre à jour" : "➕  Enregistrer Employé");
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
    connect(saveBtn, &QPushButton::clicked, this, &EmployeeDialog::onSaveClicked);
    btnLay->addWidget(saveBtn, 2);

    mainLayout->addWidget(footer);

    // Connections pour validation
    connect(firstNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateFirstName);
    connect(lastNameInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateLastName);
    connect(cinInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateCin);
    connect(phoneInput, &QLineEdit::textChanged, this, &EmployeeDialog::validatePhone);
    connect(emailInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateEmail);
    connect(salaryInput, &QLineEdit::textChanged, this, &EmployeeDialog::validateSalary);

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

void EmployeeDialog::updateFieldStyle(QWidget* field, bool isValid) {
    field->setProperty("state", isValid ? "success" : "error");
    field->style()->unpolish(field);
    field->style()->polish(field);
}

QString EmployeeDialog::getInputStyle() const
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
            border: 2px solid #2563EB;
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

void EmployeeDialog::populateFields()
{
    if (!employeeData) return;
    firstNameInput->setText(employeeData->firstName);
    lastNameInput->setText(employeeData->lastName);
    cinInput->setText(employeeData->cin);
    phoneInput->setText(employeeData->phone);
    emailInput->setText(employeeData->email);
    int posIndex = positionCombo->findText(employeeData->position);
    if (posIndex >= 0) positionCombo->setCurrentIndex(posIndex);
    salaryInput->setText(employeeData->salary.split(" ").first());
    QDate dt = QDate::fromString(employeeData->date, "dd/MM/yyyy");
    if (dt.isValid()) dateInput->setDate(dt);
    int index = statusCombo->findText(employeeData->status);
    if (index >= 0) statusCombo->setCurrentIndex(index);
    rfidInput->setText(employeeData->rfid_uid);
}

Employee EmployeeDialog::getData() const
{
    Employee employee;
    if(isEdit && employeeData) employee.id = employeeData->id;
    employee.firstName = firstNameInput->text().trimmed();
    employee.lastName = lastNameInput->text().trimmed();
    employee.cin = cinInput->text().trimmed();
    employee.phone = phoneInput->text().trimmed();
    employee.email = emailInput->text().trimmed();
    employee.position = positionCombo->currentText();
    employee.salary = salaryInput->text().trimmed();
    employee.date = dateInput->date().toString("dd/MM/yyyy");
    employee.status = statusCombo->currentText();
    employee.rfid_uid = rfidInput->text().trimmed();
    return employee;
}

void EmployeeDialog::validateFirstName(const QString &text)
{
    if (text.isEmpty()) { firstNameErrorLabel->setText("Le prénom est obligatoire"); firstNameErrorLabel->show(); updateFieldStyle(firstNameInput, false); }
    else if (text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"))) { firstNameErrorLabel->setText("utiliser que des lettres"); firstNameErrorLabel->show(); updateFieldStyle(firstNameInput, false); }
    else { firstNameErrorLabel->hide(); updateFieldStyle(firstNameInput, true); }
}

void EmployeeDialog::validateLastName(const QString &text)
{
    if (text.isEmpty()) { lastNameErrorLabel->setText("Le nom est obligatoire"); lastNameErrorLabel->show(); updateFieldStyle(lastNameInput, false); }
    else if (text.contains(QRegularExpression("[^a-zA-ZÀ-ÿ\\s\\-]"))) { lastNameErrorLabel->setText("utiliser que des lettres"); lastNameErrorLabel->show(); updateFieldStyle(lastNameInput, false); }
    else { lastNameErrorLabel->hide(); updateFieldStyle(lastNameInput, true); }
}

void EmployeeDialog::validateCin(const QString &text)
{
    if (text.isEmpty()) { cinErrorLabel->setText("Le CIN est obligatoire"); cinErrorLabel->show(); updateFieldStyle(cinInput, false); }
    else if (text.contains(QRegularExpression("[^0-9]"))) { cinErrorLabel->setText("utiliser que des chiffres"); cinErrorLabel->show(); updateFieldStyle(cinInput, false); }
    else if (text.length() != 8) { cinErrorLabel->setText(QString("Le CIN doit comporter 8 chiffres (%1/8)").arg(text.length())); cinErrorLabel->show(); updateFieldStyle(cinInput, false); }
    else { cinErrorLabel->hide(); updateFieldStyle(cinInput, true); }
}

void EmployeeDialog::validateSalary(const QString &text)
{
    if (text.isEmpty()) { salaryErrorLabel->setText("Le salaire est obligatoire"); salaryErrorLabel->show(); updateFieldStyle(salaryInput, false); }
    else if (text.contains(QRegularExpression("[^0-9]"))) { salaryErrorLabel->setText("utiliser que des chiffres"); salaryErrorLabel->show(); updateFieldStyle(salaryInput, false); }
    else if (text.toInt() < 1000 || text.toInt() > 10000) { salaryErrorLabel->setText("le salaire doit être entre 1000 et 10000"); salaryErrorLabel->show(); updateFieldStyle(salaryInput, false); }
    else { salaryErrorLabel->hide(); updateFieldStyle(salaryInput, true); }
}

void EmployeeDialog::validatePhone(const QString &text)
{
    if (text.isEmpty()) { phoneErrorLabel->setText("Le numéro est obligatoire"); phoneErrorLabel->show(); updateFieldStyle(phoneInput, false); }
    else if (text.contains(QRegularExpression("[^0-9]"))) { phoneErrorLabel->setText("utiliser que des chiffres"); phoneErrorLabel->show(); updateFieldStyle(phoneInput, false); }
    else if (text.length() != 8) { phoneErrorLabel->setText(QString("Le numéro doit comporter 8 chiffres (%1/8)").arg(text.length())); phoneErrorLabel->show(); updateFieldStyle(phoneInput, false); }
    else { phoneErrorLabel->hide(); updateFieldStyle(phoneInput, true); }
}

void EmployeeDialog::validateEmail(const QString &text)
{
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@gmail\\.com$");
    if (text.isEmpty()) { emailErrorLabel->setText("L'email est obligatoire"); emailErrorLabel->show(); updateFieldStyle(emailInput, false); }
    else if (!emailRegex.match(text).hasMatch()) { emailErrorLabel->setText("Format: nom@gmail.com"); emailErrorLabel->show(); updateFieldStyle(emailInput, false); }
    else { emailErrorLabel->hide(); updateFieldStyle(emailInput, true); }
}

void EmployeeDialog::onSaveClicked()
{
    validateCin(cinInput->text());
    validatePhone(phoneInput->text());
    validateEmail(emailInput->text());
    validateFirstName(firstNameInput->text());
    validateLastName(lastNameInput->text());
    validateSalary(salaryInput->text());

    if (!cinErrorLabel->isHidden() || !firstNameErrorLabel->isHidden() || !lastNameErrorLabel->isHidden() || !salaryErrorLabel->isHidden() || !phoneErrorLabel->isHidden() || !emailErrorLabel->isHidden()) return;

    QString currentCin = cinInput->text();
    QSqlQuery checkQuery;
    QString sql = "SELECT COUNT(*) FROM EMPLOYEES WHERE CIN = :cin";
    if (isEdit && employeeData) sql += " AND ID_EMPLOYE != :id";
    checkQuery.prepare(sql);
    checkQuery.bindValue(":cin", currentCin);
    if (isEdit && employeeData) checkQuery.bindValue(":id", employeeData->id.toInt());

    if (checkQuery.exec() && checkQuery.next()) { 
        if (checkQuery.value(0).toInt() > 0) {
            QMessageBox::critical(this, "Erreur", "Le CIN existe déjà.");
            return;
        }
    }
    accept();
}

void EmployeeDialog::onScanRfid()
{
    scanBtn->setText("⏳ Approchez...");
    MainWindow* mainWin = nullptr;
    for (QWidget* widget : QApplication::topLevelWidgets()) { if ((mainWin = qobject_cast<MainWindow*>(widget))) break; }
    if (mainWin) connect(mainWin, &MainWindow::rfidScanned, this, &EmployeeDialog::updateRfidField, Qt::UniqueConnection);
}

void EmployeeDialog::updateRfidField(const QString& uid)
{
    rfidInput->setText(uid);
    scanBtn->setText("Scanner Carte");
    MainWindow* mainWin = nullptr;
    for (QWidget* widget : QApplication::topLevelWidgets()) { if ((mainWin = qobject_cast<MainWindow*>(widget))) break; }
    if (mainWin) disconnect(mainWin, &MainWindow::rfidScanned, this, &EmployeeDialog::updateRfidField);
}
