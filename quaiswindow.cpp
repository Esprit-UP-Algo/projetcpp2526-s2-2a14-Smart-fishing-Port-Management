#include "quaiswindow.h"
#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPushButton>
#include <cmath>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QtSql/QSqlQuery>
#include <QFileDialog>
#include <QDate>
#include <QPrintDialog>
#include <QTextTable>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QTextLength>
#include <QDateTime>
#include <QStandardPaths>
#include <QDateEdit>
#include <QPageSize>

#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QDoubleValidator>

#include "addquaidialog.h"

#include "addquaidialog.h"

// ==================== END CIRCULAR PROGRESS ====================

// ==================== ADD CONTRACT GENERATOR CLASS HERE ====================
// ==================== FIXED CONTRACT GENERATOR CLASS ====================


// ==================== ADD CONTRACT DIALOG CLASS HERE ====================
class ContractGenerator {
public:
    static bool generateContract(const Quai& quai, const QString& clientName,
                                 const QString& clientCompany, const QString& duration,
                                 const QDate& startDate, QWidget* parent) {

        QString fileName = QFileDialog::getSaveFileName(parent,
                                                        "Enregistrer le contrat",
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                                                            "/Contrat_Quai_" + QString::number(quai.getNumero()) + "_" +
                                                            QDate::currentDate().toString("yyyyMMdd") + ".pdf",
                                                        "Fichiers PDF (*.pdf)");

        if (fileName.isEmpty()) return false;

        // Use QTextDocument for clean, reliable PDF generation
        QTextDocument doc;
        doc.setPageSize(QSizeF(595, 842)); // A4 in points

        QString contractNumber = QString("CT-%1-%2")
                                     .arg(quai.getNumero())
                                     .arg(QDate::currentDate().toString("yyyyMM"));

        QString html = QString(R"(
<html>
<head>
<style>
    body { font-family: 'Segoe UI', Arial, sans-serif; color: #1f2937; margin: 0; padding: 0; }
    .header {
        background: #2B5EA6;
        color: white;
        text-align: center;
        padding: 30px 20px 20px 20px;
        margin-bottom: 0;
    }
    .header h1 { margin: 0; font-size: 28pt; letter-spacing: 4px; }
    .header h3 { margin: 6px 0 0 0; font-size: 11pt; font-weight: normal; letter-spacing: 2px; }
    .contract-ref {
        background: #EFF6FF;
        border-left: 5px solid #2B5EA6;
        padding: 12px 20px;
        margin: 20px 0 10px 0;
        font-size: 10pt;
    }
    .contract-ref b { font-size: 13pt; color: #2B5EA6; }
    .section-title {
        color: #2B5EA6;
        font-size: 13pt;
        font-weight: bold;
        border-bottom: 2px solid #5D9CEC;
        padding-bottom: 4px;
        margin-top: 22px;
        margin-bottom: 10px;
    }
    table.info { width: 100%%; border-collapse: collapse; }
    table.info td { padding: 7px 10px; font-size: 10pt; }
    table.info td.label { font-weight: bold; color: #374151; width: 40%%; }
    table.info td.value { color: #1f2937; }
    .quai-card {
        background: #F0F7FF;
        border: 1px solid #5D9CEC;
        border-radius: 6px;
        padding: 14px;
        margin: 10px 0;
    }
    .tarif-box {
        background: #2B5EA6;
        color: white;
        text-align: center;
        padding: 16px;
        font-size: 16pt;
        font-weight: bold;
        border-radius: 6px;
        margin: 10px 0;
    }
    .terms { font-size: 9pt; color: #4b5563; line-height: 1.8; }
    .terms li { margin-bottom: 3px; }
    .signature-section { margin-top: 40px; }
    table.sig { width: 100%%; }
    table.sig td { text-align: center; padding: 10px; font-size: 9pt; color: #6b7280; }
    .sig-line { border-top: 1px solid #9ca3af; width: 180px; display: inline-block; margin-bottom: 6px; }
    .footer {
        text-align: center;
        font-size: 8pt;
        color: #9ca3af;
        border-top: 1px solid #e5e7eb;
        padding-top: 10px;
        margin-top: 30px;
    }
</style>
</head>
<body>

<div class="header">
    <h1>⚓ PORTFLOW</h1>
    <h3>CONTRAT DE LOCATION DE QUAI</h3>
</div>

<div class="contract-ref">
    Contrat N° : <b>%1</b> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
    Date d'émission : <b>%2</b>
</div>

<div class="section-title">INFORMATIONS CLIENT</div>
<table class="info">
    <tr><td class="label">Nom du client</td><td class="value">%3</td></tr>
    <tr><td class="label">Société / Organisation</td><td class="value">%4</td></tr>
    <tr><td class="label">Date de début</td><td class="value">%5</td></tr>
    <tr><td class="label">Durée du contrat</td><td class="value">%6</td></tr>
</table>

<div class="section-title">INFORMATIONS DU QUAI</div>
<div class="quai-card">
<table class="info">
    <tr><td class="label">Référence</td><td class="value">%7</td></tr>
    <tr><td class="label">Nom du quai</td><td class="value">%8</td></tr>
    <tr><td class="label">Capacité</td><td class="value">%9 emplacements</td></tr>
    <tr><td class="label">Taille maximale</td><td class="value">%10 mètres</td></tr>
    <tr><td class="label">Statut actuel</td><td class="value">%11</td></tr>
</table>
</div>

<div class="section-title">INFORMATIONS FINANCIÈRES</div>
<div class="tarif-box">TARIF JOURNALIER : %12 DT</div>

<div class="section-title">CONDITIONS GÉNÉRALES</div>
<div class="terms">
<ol>
    <li>Le locataire s'engage à utiliser le quai conformément à sa destination.</li>
    <li>Le paiement s'effectue mensuellement et d'avance.</li>
    <li>Le locataire est responsable des dommages causés au quai pendant la location.</li>
    <li>Le contrat peut être résilié avec un préavis de 15 jours.</li>
    <li>Le port se réserve le droit d'inspecter le quai à tout moment.</li>
    <li>Les horaires d'accès sont de 6h00 à 22h00.</li>
    <li>Le non-respect des règles de sécurité entraîne la résiliation immédiate.</li>
    <li>Le locataire doit souscrire une assurance responsabilité civile.</li>
</ol>
</div>

<div class="signature-section">
<table class="sig">
    <tr>
        <td>
            <div class="sig-line">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div><br>
            Signature du locataire<br><i>%3</i>
        </td>
        <td>
            <div class="sig-line">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div><br>
            Signature du port<br><i>PortFlow Administration</i>
        </td>
    </tr>
</table>
</div>

<div class="footer">
    Document généré automatiquement par PortFlow — Contrat valable uniquement avec signature originale
</div>

</body>
</html>
        )")
                           .arg(contractNumber)                                          // %1
                           .arg(QDate::currentDate().toString("dd MMMM yyyy"))          // %2
                           .arg(clientName)                                              // %3
                           .arg(clientCompany)                                           // %4
                           .arg(startDate.toString("dd MMMM yyyy"))                     // %5
                           .arg(duration)                                                // %6
                           .arg(quai.getNumero())                                            // %7
                           .arg((QString("Quai ") + QString::number(quai.getNumero())))                                           // %8
                           .arg(quai.getCapacite())                                      // %9

                           .arg(quai.getEtat())                                        // %11
                           .arg(quai.getTarif(), 0, 'f', 2);                            // %12

        doc.setHtml(html);
        doc.setPageSize(QSizeF(595, 842));

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

        doc.print(&printer);

        QMessageBox::information(parent, "✅ Succès",
                                 QString("Le contrat a été généré avec succès !\n📁 %1").arg(fileName));

        return true;
    }
};
// ==================== END CONTRACT GENERATOR ====================

// ==================== ADD CONTRACT DIALOG CLASS HERE ====================
class ContractDialog : public QDialog {
public:
    ContractDialog(const Quai& quai, QWidget* parent = nullptr)
        : QDialog(parent), m_quai(quai) {

        setFixedSize(560, 620);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 80));

        QWidget* container = new QWidget(this);
        container->setGeometry(10, 10, 540, 600);  // inset for shadow
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
                    stop:0 #059669, stop:1 #34D399);
                border-radius: 20px 20px 0 0;
            }
        )");
        QHBoxLayout* headerLay = new QHBoxLayout(header);
        headerLay->setContentsMargins(25, 0, 18, 0);

        QLabel* titleLbl = new QLabel("📄  Nouveau contrat de location");
        titleLbl->setFont(QFont("Segoe UI", 14, QFont::Bold));
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

        // --- Quai preview card ---
        QFrame* previewCard = new QFrame();
        previewCard->setFixedHeight(72);
        previewCard->setStyleSheet(R"(
            QFrame {
                background: #F0FDF4;
                border: 2px solid #6EE7B7;
                border-radius: 12px;
                margin: 16px 20px 0 20px;
            }
        )");
        QHBoxLayout* previewLay = new QHBoxLayout(previewCard);
        previewLay->setContentsMargins(14, 8, 14, 8);

        QLabel* iconLbl = new QLabel("⚓");
        iconLbl->setFont(QFont("Segoe UI", 22));
        iconLbl->setStyleSheet("color: #059669; background: transparent; border: none;");
        previewLay->addWidget(iconLbl);

        QVBoxLayout* infoLay = new QVBoxLayout();
        infoLay->setSpacing(2);
        QLabel* titlePreview = new QLabel((QString("Quai ") + QString::number(quai.getNumero())) + "  —  " + QString::number(quai.getNumero()));
        titlePreview->setFont(QFont("Segoe UI", 11, QFont::Bold));
        titlePreview->setStyleSheet("color: #065F46; background: transparent; border: none;");
        QLabel* detailsPreview = new QLabel(
            QString("Capacité: %1  |  Taille max: %2 m  |  Tarif: %3 DT/j")
                .arg(quai.getCapacite()).arg(quai.getTarif()));
        detailsPreview->setFont(QFont("Segoe UI", 9));
        detailsPreview->setStyleSheet("color: #6b7280; background: transparent; border: none;");
        infoLay->addWidget(titlePreview);
        infoLay->addWidget(detailsPreview);
        previewLay->addLayout(infoLay, 1);
        mainLay->addWidget(previewCard);

        // --- Form area ---
        QWidget* formArea = new QWidget();
        formArea->setStyleSheet("background: transparent;");
        QVBoxLayout* formLay = new QVBoxLayout(formArea);
        formLay->setContentsMargins(20, 14, 20, 8);
        formLay->setSpacing(6);

        auto addField = [&](const QString& labelText, QLineEdit*& fieldPtr,
                            const QString& placeholder, QLabel*& errLblPtr) {
            QLabel* lbl = new QLabel(labelText);
            lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
            lbl->setStyleSheet("color: #374151; background: transparent;");
            formLay->addWidget(lbl);

            fieldPtr = new QLineEdit();
            fieldPtr->setPlaceholderText(placeholder);
            fieldPtr->setFixedHeight(40);
            fieldPtr->setStyleSheet(R"(
                QLineEdit {
                    background: #F9FAFB; border: 2px solid #E5E7EB;
                    border-radius: 10px; padding: 4px 12px;
                    color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
                }
                QLineEdit:focus { border: 2px solid #059669; background: white; }
            )");
            formLay->addWidget(fieldPtr);

            errLblPtr = new QLabel("");
            errLblPtr->setFont(QFont("Segoe UI", 8));
            errLblPtr->setStyleSheet("color: #DC2626; background: transparent; margin-top: -4px; margin-bottom: 2px;");
            errLblPtr->hide();
            formLay->addWidget(errLblPtr);
        };

        QLabel* nameErr = nullptr;
        QLabel* compErr = nullptr;
        QLabel* emailErr = nullptr;
        QLabel* phoneErr = nullptr;

        addField("Nom complet du client *", clientNameField, "ex: Jean Dupont", nameErr);
        QRegularExpression nameExp("^[a-zA-ZÀ-ÿ\\s]+$");
        connect(clientNameField, &QLineEdit::textChanged, this, [nameErr, nameExp](const QString& text) {
            if (text.isEmpty() || nameExp.match(text).hasMatch()) nameErr->hide();
            else { nameErr->setText("⚠ Seules les lettres sont autorisées."); nameErr->show(); }
        });

        addField("Société / Organisation *", companyField, "ex: Sea Harvest Ltd", compErr);

        addField("Email", emailField, "ex: contact@entreprise.com", emailErr);
        QRegularExpression emailExp("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
        connect(emailField, &QLineEdit::textChanged, this, [emailErr, emailExp](const QString& text) {
            if (text.isEmpty() || emailExp.match(text).hasMatch()) emailErr->hide();
            else { emailErr->setText("⚠ Format d'email invalide."); emailErr->show(); }
        });

        addField("Téléphone", phoneField, "ex: 216XXXXXXXX", phoneErr);
        QRegularExpression phoneExp("^\\+?\\d{8,15}$");
        connect(phoneField, &QLineEdit::textChanged, this, [phoneErr, phoneExp](const QString& text) {
            if (text.isEmpty() || phoneExp.match(text).hasMatch()) phoneErr->hide();
            else { phoneErr->setText("⚠ Le numéro doit contenir entre 8 et 15 chiffres."); phoneErr->show(); }
        });

        // Date + Duration on same row
        QHBoxLayout* dateRow = new QHBoxLayout();
        dateRow->setSpacing(12);

        QVBoxLayout* dateCol = new QVBoxLayout();
        dateCol->setSpacing(4);
        QLabel* dateLbl = new QLabel("Date de début *");
        dateLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        dateLbl->setStyleSheet("color: #374151; background: transparent;");
        startDateField = new QDateEdit(QDate::currentDate());
        startDateField->setCalendarPopup(true);
        startDateField->setFixedHeight(40);
        startDateField->setStyleSheet(R"(
            QDateEdit {
                background: #F9FAFB; border: 2px solid #E5E7EB;
                border-radius: 10px; padding: 4px 12px;
                color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
            }
            QDateEdit:focus { border: 2px solid #059669; background: white; }
        )");
        dateCol->addWidget(dateLbl);
        dateCol->addWidget(startDateField);
        dateRow->addLayout(dateCol);

        QVBoxLayout* durCol = new QVBoxLayout();
        durCol->setSpacing(4);
        QLabel* durLbl = new QLabel("Durée *");
        durLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        durLbl->setStyleSheet("color: #374151; background: transparent;");
        durationField = new QComboBox();
        durationField->addItems({"1 mois", "3 mois", "6 mois", "1 an", "2 ans"});
        durationField->setFixedHeight(40);
        durationField->setStyleSheet(R"(
            QComboBox {
                background: #F9FAFB; border: 2px solid #E5E7EB;
                border-radius: 10px; padding: 4px 12px;
                color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
            }
            QComboBox:focus { border: 2px solid #059669; background: white; }
            QComboBox::drop-down { border: none; width: 28px; }
            QComboBox QAbstractItemView {
                background: white; border: 1px solid #e2e8f0;
                border-radius: 10px; selection-background-color: #D1FAE5;
                selection-color: #065F46; padding: 6px;
            }
        )");
        durCol->addWidget(durLbl);
        durCol->addWidget(durationField);
        dateRow->addLayout(durCol);

        formLay->addLayout(dateRow);
        mainLay->addWidget(formArea, 1);

        // --- Buttons ---
        QHBoxLayout* btnLay = new QHBoxLayout();
        btnLay->setContentsMargins(20, 8, 20, 20);
        btnLay->setSpacing(12);

        QPushButton* cancelBtn = new QPushButton("Annuler");
        cancelBtn->setFixedHeight(42);
        cancelBtn->setMinimumWidth(110);
        cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setStyleSheet(R"(
            QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; padding: 0 18px; }
            QPushButton:hover { background: #E5E7EB; }
        )");
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

        QPushButton* generateBtn = new QPushButton("📄  Générer le contrat");
        generateBtn->setFixedHeight(42);
        generateBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        generateBtn->setCursor(Qt::PointingHandCursor);
        generateBtn->setStyleSheet(R"(
            QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                              stop:0 #059669, stop:1 #34D399);
                          color: white; border: none; border-radius: 10px; padding: 0 20px; }
            QPushButton:hover { background: #047857; }
        )");
        connect(generateBtn, &QPushButton::clicked, this, &QDialog::accept);

        btnLay->addStretch();
        btnLay->addWidget(cancelBtn);
        btnLay->addWidget(generateBtn);
        mainLay->addLayout(btnLay);
    }

    QString getClientName() const { return clientNameField->text(); }
    QString getCompany()    const { return companyField->text(); }
    QString getDuration()   const { return durationField->currentText(); }
    QDate   getStartDate()  const { return startDateField->date(); }

private:
    QLineEdit* clientNameField = nullptr;
    QLineEdit* companyField    = nullptr;
    QLineEdit* emailField      = nullptr;
    QLineEdit* phoneField      = nullptr;
    QDateEdit* startDateField  = nullptr;
    QComboBox* durationField   = nullptr;
    Quai m_quai;
};
QuaisWindow::QuaisWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(1400, 800);
    setWindowTitle("PortFlow - Gestion des Quais");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    for (int i = 0; i < 12; ++i) {
        Quai q;
        q.setNumero(i + 1);
        q.setCapacite(3);

        q.setEtat((i % 3 == 0) ? "Disponible" :
                      (i % 3 == 1) ? "Occupé" : "Maintenance");
        q.setTarif(50.0);
        q.setLocation((i % 3 == 1) ? "Sea Harvest Ltd" : "-");

        quais.append(q);
    }

    setupUI();
    setupQuaiTable();
    loadQuaisFromDatabase();
    populateTable();
}

// ---------- SETUP UI ----------
void QuaisWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(createSidebar());
    mainLayout->addWidget(createContentArea(), 1);
}

// ---------- SIDEBAR ----------
QFrame* QuaisWindow::createSidebar()
{
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(280);
    sidebar->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #2B5EA6,
                stop:1 #5D9CEC
            );
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Logo section
    QFrame* logoFrame = new QFrame();
    logoFrame->setFixedHeight(160);
    logoFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setContentsMargins(20, 15, 20, 15);

    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(180, 100);
    logoContainer->setStyleSheet(R"(
        QFrame {
            background-color: transparent;
            border-radius: 0px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(logoContainer);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    containerLayout->setAlignment(Qt::AlignCenter);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix(":/images/images/logo.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
    } else {
        logoLabel->setText("⚓");
        logoLabel->setStyleSheet(R"(
            QLabel { font-size: 50px; color: white; }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
    }

    logoLabel->setStyleSheet("background: transparent;");
    containerLayout->addWidget(logoLabel);
    logoLayout->addWidget(logoContainer);
    layout->addWidget(logoFrame);

    // Navigation
    QFrame* navFrame = new QFrame();
    navFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* navLayout = new QVBoxLayout(navFrame);
    navLayout->setSpacing(8);
    navLayout->setContentsMargins(20, 20, 20, 20);

    navLayout->addWidget(createNavButton("🏠", "Dashboard"));
    navLayout->addWidget(createNavButton("⛵", "Bateaux"));
    navLayout->addWidget(createNavButton("🐟", "Pêche"));
    navLayout->addWidget(createNavButton("👥", "Employés"));
    navLayout->addWidget(createNavButton("⚓", "Quais", true));
    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));

    navLayout->addStretch();
    navLayout->addWidget(createNavButton("🚪", "Quitter", false, true));

    layout->addWidget(navFrame, 1);
    return sidebar;
}

QPushButton* QuaisWindow::createNavButton(const QString& icon, const QString& text,
                                          bool isActive, bool isLogout)
{
    Q_UNUSED(isActive);
    QPushButton* btn = new QPushButton(icon + "  " + text);
    btn->setFont(QFont("Segoe UI", 12, QFont::Medium));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(55);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255, 255, 255, 0.1); color: white; border: none; border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(239, 68, 68, 0.8); }
        )");
        connect(btn, &QPushButton::clicked, this, &QuaisWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255, 255, 255, 0.25); color: white; border: none; border-radius: 12px; text-align: left; padding-left: 20px; font-weight: bold; }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton { background-color: transparent; color: rgba(255, 255, 255, 0.9); border: none; border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(255, 255, 255, 0.15); }
        )");
    }
    return btn;
}

QWidget* QuaisWindow::createContentArea()
{
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #F0F4F8;");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    layout->addWidget(createHeader());
    layout->addWidget(createToolbar());
    layout->addWidget(createTableCard(), 1);

    return content;
}

QFrame* QuaisWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background:transparent;");
    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Quais");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color:#1e3a5f;");
    QLabel* sub = new QLabel("Administration des emplacements et disponibilités");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color:#6b7280;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(45);
        btn->setMinimumWidth(160);
        btn->setStyleSheet(QString("QPushButton{ background:%1; color:white; border:none; border-radius:12px; padding:0 20px; } QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    connect(statsBtn, &QPushButton::clicked, this, &QuaisWindow::afficherStatistiques);

    QPushButton* pdfBtn = makeBtn("📄  Exporter PDF", "#059669", "#047857");
    // Connect the PDF button to show a dialog to select a quai and generate contract
    connect(pdfBtn, &QPushButton::clicked, this, [this]() {
        if (quais.isEmpty()) {
            QMessageBox::warning(this, "Aucun quai", "Aucun quai disponible.");
            return;
        }

        QDialog* dlg = new QDialog(this);
        dlg->setFixedSize(560, 460);
        dlg->setModal(true);
        dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        dlg->setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 80));

        QWidget* container = new QWidget(dlg);
        container->setGeometry(10, 10, 540, 440);  // inset for shadow room
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
                stop:0 #059669, stop:1 #34D399);
            border-radius: 20px 20px 0 0;
        }
    )");
        QHBoxLayout* headerLay = new QHBoxLayout(header);
        headerLay->setContentsMargins(25, 0, 18, 0);

        QLabel* titleLbl = new QLabel("📄  Exporter un contrat PDF");
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
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
        headerLay->addWidget(closeBtn);
        mainLay->addWidget(header);

        // --- Body ---
        QWidget* body = new QWidget();
        body->setStyleSheet("background: transparent;");
        QVBoxLayout* bodyLay = new QVBoxLayout(body);
        bodyLay->setContentsMargins(24, 18, 24, 0);
        bodyLay->setSpacing(10);

        QLabel* instrLbl = new QLabel("Sélectionnez un quai pour générer son contrat de location :");
        instrLbl->setFont(QFont("Segoe UI", 10));
        instrLbl->setStyleSheet("color: #6b7280; background: transparent;");
        instrLbl->setWordWrap(true);
        bodyLay->addWidget(instrLbl);

        // List widget
        QListWidget* quaiList = new QListWidget();
        quaiList->setFont(QFont("Segoe UI", 10));
        quaiList->setStyleSheet(R"(
        QListWidget {
            background: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 6px;
            outline: none;
        }
        QListWidget::item {
            padding: 10px 14px;
            border-radius: 8px;
            color: #1f2937;
        }
        QListWidget::item:selected {
            background: #D1FAE5;
            color: #065F46;
            font-weight: bold;
        }
        QListWidget::item:hover:!selected {
            background: #F3F4F6;
        }
    )");

        for (int i = 0; i < quais.size(); ++i) {
            const Quai& q = quais[i];
            QString emoji = (q.getEtat() == "Disponible") ? "🟢"
                            : (q.getEtat() == "Occupé")     ? "🟡" : "🔴";
            QString text = QString("%1  %2  —  %3     %4 m  |  %5 DT/j")
                               .arg(emoji)
                               .arg(q.getNumero())
                               .arg((QString("Quai ") + QString::number(q.getNumero())))

                               .arg(q.getTarif());
            QListWidgetItem* item = new QListWidgetItem(text);
            item->setData(Qt::UserRole, i);
            if (q.getEtat() == "Maintenance") {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                item->setForeground(QColor("#9ca3af"));
            }
            quaiList->addItem(item);
        }

        bodyLay->addWidget(quaiList, 1);
        mainLay->addWidget(body, 1);

        // --- Buttons ---
        QHBoxLayout* btnLay = new QHBoxLayout();
        btnLay->setContentsMargins(24, 12, 24, 20);
        btnLay->setSpacing(12);

        QPushButton* cancelBtn = new QPushButton("Annuler");
        cancelBtn->setFixedHeight(42);
        cancelBtn->setMinimumWidth(110);
        cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
        connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

        QPushButton* generateBtn = new QPushButton("📄  Générer le contrat");
        generateBtn->setFixedHeight(42);
        generateBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        generateBtn->setCursor(Qt::PointingHandCursor);
        generateBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #059669, stop:1 #34D399);
                      color: white; border: none; border-radius: 10px; padding: 0 20px; }
        QPushButton:hover { background: #047857; }
    )");

        connect(generateBtn, &QPushButton::clicked, [this, dlg, quaiList]() {
            QListWidgetItem* current = quaiList->currentItem();
            if (!current || !(current->flags() & Qt::ItemIsEnabled)) {
                QMessageBox::warning(dlg, "Sélection requise",
                                     "Veuillez sélectionner un quai disponible ou occupé.");
                return;
            }
            int row = current->data(Qt::UserRole).toInt();
            dlg->accept();
            onGenerateContract(row);
        });

        btnLay->addStretch();
        btnLay->addWidget(cancelBtn);
        btnLay->addWidget(generateBtn);
        mainLay->addLayout(btnLay);

        dlg->exec();
    });

    QPushButton* addBtn   = makeBtn("➕  Nouveau Quai",   "#2563EB", "#1D4ED8");

    connect(addBtn, &QPushButton::clicked, this, &QuaisWindow::onAddQuai);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* QuaisWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet(R"(
        QFrame {
            background: white;
            border-radius: 14px;
            border: 1.5px solid #e2e8f0;
        }
    )");
    bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bar->setMinimumHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    /* Search bar */
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher un quai par numéro, type...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding-left:12px; font-size:13px; color:#334155;
        }
        QLineEdit:focus { border:1.5px solid #3b82f6; background:white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &QuaisWindow::onSearch);
    lay->addWidget(searchInput);

    lay->addStretch();

    /* Sort combo */
    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color:#64748b; font-weight:600; border:none; background:transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Numéro ↑", "Numéro ↓", "Tarif ↑", "Tarif ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox {
            background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;
            padding:0 12px; color:#334155;
        }
        QComboBox::drop-down { border:none; }
        QComboBox::down-arrow { image:none; border-left:5px solid transparent; border-right:5px solid transparent; border-top:5px solid #64748b; margin-right:8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QuaisWindow::onSort);
    lay->addWidget(sortCombo);

    return bar;
}

QFrame* QuaisWindow::createTableCard()
{
    QFrame* card = new QFrame();
    card->setStyleSheet("QFrame { background-color: #5D9CEC; border-radius: 20px; padding: 3px; }");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(3, 3, 3, 3);

    QFrame* whiteContainer = new QFrame();
    whiteContainer->setStyleSheet("QFrame { background-color: white; border-radius: 17px; }");
    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    quaiTable = new QTableWidget();
    containerLayout->addWidget(quaiTable);
    layout->addWidget(whiteContainer);

    return card;
}

void QuaisWindow::setupQuaiTable()
{
    quaiTable->setColumnCount(7);
    quaiTable->setHorizontalHeaderLabels({"Référence", "Nom du Quai", "Capacité", "Localisation", "Statut", "Tarif / Durée", "Actions"});
    quaiTable->horizontalHeader()->setStretchLastSection(true);
    quaiTable->verticalHeader()->setVisible(false);
    quaiTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    quaiTable->setSelectionMode(QAbstractItemView::SingleSelection);
    quaiTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    quaiTable->setShowGrid(true);
    quaiTable->horizontalHeader()->setFont(QFont("Segoe UI", 11, QFont::Bold));
    quaiTable->horizontalHeader()->setFixedHeight(50);
    quaiTable->setStyleSheet("QTableWidget { background-color: white; border: 2px solid #d1d5db; border-radius: 16px; gridline-color: #d1d5db; } QTableWidget::item { padding: 12px; border-right: 1px solid #d1d5db; border-bottom: 1px solid #d1d5db; color: #1f2937; background-color: white; font-family: 'Segoe UI'; font-size: 11pt; } QTableWidget::item:selected { background-color: #EBF5FF; color: #2563EB; } QHeaderView::section { background-color: #d1d5db; color: #1f2937; padding: 12px; border: none; font-weight: 600; font-family: 'Segoe UI'; font-size: 11pt; }");

    quaiTable->setColumnWidth(0, 100);
    quaiTable->setColumnWidth(1, 120);
    quaiTable->setColumnWidth(2, 100);
    quaiTable->setColumnWidth(3, 160);
    quaiTable->setColumnWidth(4, 130);
    quaiTable->setColumnWidth(5, 150);
}
void QuaisWindow::loadQuaisFromDatabase()
{
    quais.clear(); // clear the current list

    QSqlQuery query;
    if (!query.exec("SELECT NUMERO, CAPACITE, LOCATION, ETAT, TARIF_LOCATION, DUREE_LOCATION FROM QUAIS")) {
        qDebug() << "Database query error:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int numero           = query.value("NUMERO").toInt();
        int capacite         = query.value("CAPACITE").toInt();
        QString etat         = query.value("ETAT").toString();
        double tarifLocation = query.value("TARIF_LOCATION").toDouble();
        QString location     = query.value("LOCATION").toString();      // <-- ajoute ceci
        QString dureeLocation = query.value("DUREE_LOCATION").toString();

        Quai q(numero, capacite, etat, tarifLocation, location, dureeLocation);

        quais.append(q);
    }
}

void QuaisWindow::populateTable(const QString& filterText)
{
    quaiTable->setRowCount(0);

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];

        // Filter if needed
        if (!filterText.isEmpty()) {
            QString searchLower = filterText.toLower();
            if (!(QString("Quai ") + QString::number(q.getNumero())).toLower().contains(searchLower) &&
                !q.getEtat().toLower().contains(searchLower) &&
                !q.getDureeLocation().toLower().contains(searchLower) &&
                !QString::number(q.getNumero()).contains(searchLower))
                continue;
        }

        int row = quaiTable->rowCount();
        quaiTable->insertRow(row);
        quaiTable->setRowHeight(row, 65);

        // Numero
        QTableWidgetItem* numeroItem = new QTableWidgetItem(QString::number(q.getNumero()));
        numeroItem->setForeground(QBrush(QColor("#5D9CEC")));
        numeroItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
        quaiTable->setItem(row, 0, numeroItem);

        // Name
        quaiTable->setItem(row, 1, new QTableWidgetItem(QString("Quai %1").arg(q.getNumero())));

        // Capacity
        quaiTable->setItem(row, 2, new QTableWidgetItem(QString::number(q.getCapacite())));

        // Location
        quaiTable->setItem(row, 3, new QTableWidgetItem(q.getLocation()));

        // Status
        quaiTable->setCellWidget(row, 4, createStatusBadge(q.getEtat()));

        // Tarif / Duration
        quaiTable->setItem(row, 5, new QTableWidgetItem(
                                       QString("%1 DT / %2").arg(q.getTarif()).arg(q.getDureeLocation())
                                       ));

        // Action Buttons
        quaiTable->setCellWidget(row, 6, createActionButtons(i));
    }
}
QWidget* QuaisWindow::createStatusBadge(const QString& status)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    QLabel* badge = new QLabel(status);
    badge->setFont(QFont("Segoe UI", 10, QFont::Medium));
    badge->setFixedHeight(32);
    badge->setAlignment(Qt::AlignCenter);

    if (status == "Disponible") badge->setStyleSheet("QLabel { background-color: #D1FAE5; color: #065F46; border-radius: 8px; padding: 6px 16px; }");
    else if (status == "Occupé") badge->setStyleSheet("QLabel { background-color: #FEF3C7; color: #92400E; border-radius: 8px; padding: 6px 16px; }");
    else badge->setStyleSheet("QLabel { background-color: #FEE2E2; color: #991B1B; border-radius: 8px; padding: 6px 16px; }");

    layout->addWidget(badge);
    return widget;
}

QWidget* QuaisWindow::createActionButtons(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);

    auto makeBtn = [&](const QString& icon, const QString& bg, const QString& hover, const QString& tooltip = "") {
        QPushButton* btn = new QPushButton(icon);
        btn->setFixedSize(36, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setStyleSheet(QString("QPushButton { background-color: %1; border: none; border-radius: 8px; font-size: 16px; } QPushButton:hover { background-color: %2; }").arg(bg, hover));
        return btn;
    };

    // Edit button
    QPushButton* editBtn = makeBtn("✏️", "#FEF3C7", "#FDE68A", "Modifier le quai");
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditQuai(row); });
    layout->addWidget(editBtn);

    // Delete button
    QPushButton* deleteBtn = makeBtn("🗑️", "#FEE2E2", "#FECACA", "Supprimer le quai");
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteQuai(row); });
    layout->addWidget(deleteBtn);

    // Contract button (NEW)
    QPushButton* contractBtn = makeBtn("📄", "#E0F2FE", "#BAE6FD", "Générer un contrat");
    connect(contractBtn, &QPushButton::clicked, [this, row]() {
        if (row >= 0 && row < quais.size()) {
            onGenerateContract(row);
        }
    });
    layout->addWidget(contractBtn);

    return widget;
}

void QuaisWindow::onSearch(const QString& text) { populateTable(text); }

void QuaisWindow::onAddQuai()
{
    AddQuaiDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        loadQuaisFromDatabase();
        populateTable(searchInput->text());
    }
}
void QuaisWindow::onEditQuai(int row) {
    if (row < 0 || row >= quais.size()) return;

    Quai& q = quais[row];

    // --- Create frameless dialog ---
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Modifier Quai");
    dlg->setFixedSize(600, 600);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 600, 600);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // --- Header band ---
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(80);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2B5EA6, stop:1 #5D9CEC);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("✏️  Modifier le Quai");
    titleLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 18px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    headerLay->addWidget(closeBtn);

    mainLay->addWidget(headerBand);

    // --- Form area ---
    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(30, 20, 30, 10);
    formLay->setSpacing(14);

    auto applyFieldState = [](QLineEdit* f, QLabel* h,
                              bool isEmpty, bool isValid,
                              const QString& idle, const QString& err, const QString& ok) {
        if (isEmpty) {
            f->setStyleSheet(R"(
            QLineEdit { background:#F9FAFB; border:1.5px solid #E5E7EB;
                        border-radius:10px; padding:4px 14px; color:#1f2937;
                        font-family:'Segoe UI'; font-size:11pt; }
            QLineEdit:focus { border:1.5px solid #378ADD; background:white; }
        )");
            h->setText("  " + idle);
            h->setStyleSheet("color:#9ca3af; background:transparent; margin-top:-3px; margin-bottom:4px;");
        } else if (isValid) {
            f->setStyleSheet(R"(
            QLineEdit { background:white; border:1.5px solid #1D9E75;
                        border-radius:10px; padding:4px 14px; color:#1f2937;
                        font-family:'Segoe UI'; font-size:11pt; }
            QLineEdit:focus { border:1.5px solid #1D9E75; background:white; }
        )");
            h->setText("✓  " + ok);
            h->setStyleSheet("color:#0F6E56; background:transparent; margin-top:-3px; margin-bottom:4px;");
        } else {
            f->setStyleSheet(R"(
            QLineEdit { background:#FEF2F2; border:1.5px solid #E24B4A;
                        border-radius:10px; padding:4px 14px; color:#1f2937;
                        font-family:'Segoe UI'; font-size:11pt; }
            QLineEdit:focus { border:1.5px solid #E24B4A; background:#FEF2F2; }
        )");
            h->setText("✕  " + err);
            h->setStyleSheet("color:#A32D2D; background:transparent; margin-top:-3px; margin-bottom:4px;");
        }
    };

    auto makeField = [&](const QString& label, const QString& value, QLabel*& hintLbl,
                         const QString& idleHint) -> QLineEdit* {
        QLabel* lbl = new QLabel(label);
        lbl->setFont(QFont("Segoe UI", 8, QFont::Medium));
        lbl->setStyleSheet("color: #6b7280; background: transparent; letter-spacing: 0.5px;");

        QLineEdit* field = new QLineEdit(value);
        field->setFont(QFont("Segoe UI", 11));
        field->setFixedHeight(44);
        field->setStyleSheet(R"(
        QLineEdit { background: #F9FAFB; border: 1.5px solid #E5E7EB;
                    border-radius: 10px; padding: 4px 14px; color: #1f2937; }
        QLineEdit:focus { border: 1.5px solid #378ADD; background: white; }
    )");
        formLay->addWidget(lbl);
        formLay->addWidget(field);

        hintLbl = new QLabel("  " + idleHint);
        hintLbl->setFont(QFont("Segoe UI", 8));
        hintLbl->setStyleSheet("color:#9ca3af; background:transparent; margin-top:-3px; margin-bottom:4px;");
        formLay->addWidget(hintLbl);

        return field;
    };

    QLabel* nomHint   = nullptr;
    QLabel* capHint   = nullptr;
    QLabel* tarifHint = nullptr;

    QLineEdit* nomField      = makeField("NOM DU QUAI", (QString("Quai ") + QString::number(q.getNumero())), nomHint, "");
    nomField->setReadOnly(true);
    nomHint->setText("");

    QLineEdit* capaciteField = makeField("CAPACITÉ", QString::number(q.getCapacite()), capHint, "Nombre entier supérieur à 0");
    connect(capaciteField, &QLineEdit::textChanged, dlg, [=](const QString& t) {
        bool ok; int val = t.toInt(&ok);
        applyFieldState(capaciteField, capHint, t.isEmpty(), (ok && val > 0),
                        "Nombre entier supérieur à 0", "Doit être un entier > 0", "Capacité valide ✓");
    });

    QLineEdit* tarifField = makeField("TARIF (DT)", QString::number(q.getTarif()), tarifHint, "Nombre positif ex: 50.00");
    connect(tarifField, &QLineEdit::textChanged, dlg, [=](const QString& t) {
        bool ok; double val = t.toDouble(&ok);
        applyFieldState(tarifField, tarifHint, t.isEmpty(), (ok && val > 0),
                        "Nombre positif ex: 50.00", "Doit être un nombre > 0", "Tarif valide ✓");
    });

    // --- Statut combo ---
    QLabel* statutLbl = new QLabel("Statut");
    statutLbl->setFont(QFont("Segoe UI", 10, QFont::Medium));
    statutLbl->setStyleSheet("color: #374151; background: transparent;");
    formLay->addWidget(statutLbl);

    QComboBox* statutCombo = new QComboBox();
    statutCombo->addItems({"Disponible", "Occupé", "Maintenance"});
    statutCombo->setCurrentText(q.getEtat());
    statutCombo->setFont(QFont("Segoe UI", 11));
    statutCombo->setFixedHeight(44);
    statutCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    statutCombo->setMinimumContentsLength(12);
    statutCombo->setStyleSheet(R"(
        QComboBox { background: #F9FAFB; border: 1.5px solid #E5E7EB; border-radius: 10px;
                    padding: 4px 14px; color: #1f2937; font-family: 'Segoe UI'; font-size: 11pt; }
        QComboBox:focus { border: 1.5px solid #378ADD; background: white; }
        QComboBox::drop-down { border: none; width: 36px; subcontrol-origin: padding; subcontrol-position: right center; }
        QComboBox::down-arrow { width: 12px; height: 12px; }
        QComboBox QAbstractItemView { background: white; border: 1px solid #e2e8f0;
            border-radius: 10px; selection-background-color: #eff6ff;
            selection-color: #2563EB; padding: 6px; min-width: 180px; }
    )");
    formLay->addWidget(statutCombo);

    mainLay->addWidget(formArea, 1);

    // --- Buttons ---
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(30, 20, 30, 30);
    btnLay->setSpacing(15);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(45);
    cancelBtn->setFont(QFont("Segoe UI", 11, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background:#F3F4F6; color:#374151; border-radius:12px; font-weight:bold; }");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton("💾  Enregistrer");
    saveBtn->setFixedHeight(45);
    saveBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background:#2563EB; color:white; border-radius:12px; font-weight:bold; }");
    connect(saveBtn, &QPushButton::clicked, [=, &q]() {
        QString capText = capaciteField->text().trimmed();
        QString tarifText = tarifField->text().trimmed();

        if (capText.isEmpty() || tarifText.isEmpty()) {
            QMessageBox::warning(dlg, "Erreur", "Veuillez remplir tous les champs !");
            return;
        }

        bool okCap = false;
        int cap = capText.toInt(&okCap);
        if (!okCap || cap <= 0) {
            QMessageBox::warning(dlg, "Erreur", "La capacité doit être un nombre entier supérieur à zéro.");
            return;
        }

        bool okTarif = false;
        double tarif = tarifText.replace(",", ".").toDouble(&okTarif);
        if (!okTarif || tarif <= 0.0) {
            QMessageBox::warning(dlg, "Erreur", "Le tarif doit être un nombre positif valide.");
            return;
        }

        QSqlQuery query;
        query.prepare("UPDATE QUAIS SET CAPACITE = :cap, ETAT = :etat, TARIF_LOCATION = :tarif WHERE NUMERO = :num");
        query.bindValue(":cap", cap);
        query.bindValue(":etat", statutCombo->currentText());
        query.bindValue(":tarif", tarif);
        query.bindValue(":num", q.getNumero());

        if (query.exec()) {
            QSqlDatabase::database().commit();
            dlg->accept();
            loadQuaisFromDatabase();
            populateTable();
        } else {
            QMessageBox::critical(dlg, "Erreur SQL", query.lastError().text());
        }
    });

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(saveBtn);
    mainLay->addLayout(btnLay);

    dlg->exec();

    if (dlg->result() == QDialog::Accepted) {
        loadQuaisFromDatabase();
        populateTable(searchInput->text());
    }

}void QuaisWindow::onDeleteQuai(int row) {
    if (row < 0 || row >= quais.size()) return;

    const Quai& q = quais[row];

    QDialog* dlg = new QDialog(this);
    dlg->setFixedSize(420, 260);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 420, 260);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Red header band
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(75);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #991B1B, stop:1 #EF4444);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("🗑️  Supprimer le Quai");
    titleLbl->setFont(QFont("Segoe UI", 15, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(34, 34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 17px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(headerBand);

    // Message
    QVBoxLayout* bodyLay = new QVBoxLayout();
    bodyLay->setContentsMargins(30, 24, 30, 10);

    // FIXED: Convert both placeholders to QString
    QLabel* msgLbl = new QLabel(
        QString("Êtes-vous sûr de vouloir supprimer le quai\n<b>%1 — %2</b> ?")
            .arg(QString::number(q.getNumero()))
            .arg(QString("Quai ") + QString::number(q.getNumero()))
        );
    msgLbl->setFont(QFont("Segoe UI", 11));
    msgLbl->setStyleSheet("color: #374151; background: transparent;");
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setTextFormat(Qt::RichText);
    bodyLay->addWidget(msgLbl);

    QLabel* warnLbl = new QLabel("⚠️  Cette action est irréversible.");
    warnLbl->setFont(QFont("Segoe UI", 9));
    warnLbl->setStyleSheet("color: #B91C1C; background: transparent;");
    warnLbl->setAlignment(Qt::AlignCenter);
    bodyLay->addWidget(warnLbl);

    mainLay->addLayout(bodyLay);
    mainLay->addStretch();

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(30, 0, 30, 24);
    btnLay->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setFont(QFont("Segoe UI", 11, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* deleteBtn = new QPushButton("🗑️  Supprimer");
    deleteBtn->setFixedHeight(44);
    deleteBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #991B1B, stop:1 #EF4444);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #B91C1C; }
    )");
    connect(deleteBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(deleteBtn);
    mainLay->addLayout(btnLay);

    if (dlg->exec() == QDialog::Accepted) {
        QSqlQuery query;
        // Utilisation d'une requête préparée pour plus de sécurité
        query.prepare("DELETE FROM QUAIS WHERE NUMERO = :num");
        query.bindValue(":num", q.getNumero());

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur",
                                  "Impossible de supprimer le quai de la base de données :\n" + query.lastError().text());
        } else {
            // Validation de la transaction si nécessaire
            QSqlDatabase::database().commit();

            // Mise à jour de l'interface
            loadQuaisFromDatabase();
            populateTable(searchInput->text());

            // Optionnel : Petit feedback visuel ou sonore
            qDebug() << "Quai n°" << q.getNumero() << "supprimé avec succès.";
        }
    }

    // Nettoyage manuel si le dialogue n'a pas Qt::WA_DeleteOnClose
    dlg->deleteLater();
}

void QuaisWindow::onUpdateQuai(int id) {
    // Your logic here. Even an empty body will fix the linker:
    Q_UNUSED(id); // Avoid unused parameter warning if nothing is done yet
}
void QuaisWindow::onLogout() {
    if (QMessageBox::question(this, "Quitter", "Quitter l'application ?") == QMessageBox::Yes)
        this->close();
}

void QuaisWindow::onSort(int index) {
    if (quais.isEmpty()) return;

    switch(index) {
    case 1: // Numéro Croissant
        std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getNumero() < b.getNumero(); });
        break;
    case 2: // Numéro Décroissant
        std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getNumero() > b.getNumero(); });
        break;
    case 3: // Tarif Croissant
        std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getTarif() < b.getTarif(); });
        break;
    case 4: // Tarif Décroissant
        std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getTarif() > b.getTarif(); });
        break;
    }
    populateTable(searchInput->text());
}

// ==================== BEAUTIFUL STATISTICS DIALOG ====================
void QuaisWindow::afficherStatistiques()
{
    // Compute stats
    int totalQuais = quais.size();
    int totalBerths = 0, occupiedBerths = 0, availableBerths = 0, maintenanceBerths = 0;
    double totalRevenue = 0.0;

    for (const Quai& q : quais) {
        int cap = q.getCapacite();
        totalBerths += cap;
        if (q.getEtat() == "Occupé") {
            occupiedBerths += cap;
            totalRevenue += q.getTarif() * cap;
        } else if (q.getEtat() == "Disponible") {
            availableBerths += cap;
        } else {
            maintenanceBerths += cap;
        }
    }

    double occupancyRate = (totalBerths > 0) ? (double)occupiedBerths / totalBerths * 100 : 0;
    double averageRevenue = (totalQuais > 0) ? totalRevenue / totalQuais : 0;

    // Create dialog
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Statistiques des Quais");
    dlg->setFixedSize(720, 520);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    // Drop shadow
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    // Main container
    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 720, 520);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet(R"(
        QWidget {
            background: white;
            border-radius: 24px;
        }
    )");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // ---- Header band ----
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(90);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2B5EA6, stop:1 #5D9CEC);
            border-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(30, 0, 20, 0);

    QLabel* titleLbl = new QLabel("📊  Statistiques des Quais");
    titleLbl->setFont(QFont("Segoe UI", 18, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 18px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    headerLay->addWidget(closeBtn);

    mainLay->addWidget(headerBand);

    // ---- Subtitle ----
    QLabel* subLbl = new QLabel("Vue d'ensemble de l'occupation et des revenus du port");
    subLbl->setFont(QFont("Segoe UI", 10));
    subLbl->setStyleSheet("color: #6b7280; padding: 16px 30px 0px 30px; background: transparent;");
    mainLay->addWidget(subLbl);

    // ---- Circular charts row ----
    QFrame* chartsFrame = new QFrame();
    chartsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* chartsLay = new QHBoxLayout(chartsFrame);
    chartsLay->setContentsMargins(20, 20, 20, 10);
    chartsLay->setSpacing(10);

    // Chart data: value, max, label, unit, color
    struct ChartData { double val; double max; QString label; QString unit; QColor color; };
    QList<ChartData> charts = {
                               { (double)totalQuais,       20.0,             "Total\nQuais",           "",   QColor("#2B5EA6") },
                               { occupancyRate,            100.0,             "Taux\nd'Occupation",     "%",  QColor("#7C3AED") },
                               { (double)occupiedBerths,   (double)totalBerths, "Emplacements\nOccupés", "",   QColor("#EF8C2A") },
                               { (double)availableBerths,  (double)totalBerths, "Emplacements\nLibres",  "",   QColor("#059669") },
                               };

    QList<CircularProgress*> progressWidgets;
    for (const ChartData& cd : charts) {
        CircularProgress* cp = new CircularProgress(cd.val, cd.max, cd.label, cd.unit, cd.color);
        chartsLay->addWidget(cp, 0, Qt::AlignCenter);
        progressWidgets.append(cp);
    }
    mainLay->addWidget(chartsFrame);

    // ---- Stats cards row ----
    QFrame* cardsFrame = new QFrame();
    cardsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* cardsLay = new QHBoxLayout(cardsFrame);
    cardsLay->setContentsMargins(24, 4, 24, 4);
    cardsLay->setSpacing(14);

    auto makeCard = [&](const QString& icon, const QString& val, const QString& label,
                        const QString& bg, const QString& textColor) {
        QFrame* card = new QFrame();
        card->setFixedHeight(90);
        card->setStyleSheet(QString("QFrame { background: %1; border-radius: 16px; }").arg(bg));
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(16, 8, 16, 8);

        QLabel* iconLbl = new QLabel(icon);
        iconLbl->setFont(QFont("Segoe UI", 22));
        iconLbl->setStyleSheet("background: transparent;");
        cl->addWidget(iconLbl);

        QVBoxLayout* vl = new QVBoxLayout();
        QLabel* valLbl = new QLabel(val);
        valLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
        valLbl->setStyleSheet(QString("color: %1; background: transparent;").arg(textColor));
        QLabel* labLbl = new QLabel(label);
        labLbl->setFont(QFont("Segoe UI", 9));
        labLbl->setStyleSheet("color: #6b7280; background: transparent;");
        vl->addWidget(valLbl);
        vl->addWidget(labLbl);
        cl->addLayout(vl, 1);
        return card;
    };

    cardsLay->addWidget(makeCard("💰",
                                 QString("%1 DT").arg(totalRevenue, 0, 'f', 0),
                                 "Chiffre d'Affaires", "#EFF6FF", "#1D4ED8"));

    cardsLay->addWidget(makeCard("📈",
                                 QString("%1 DT").arg(averageRevenue, 0, 'f', 0),
                                 "Revenu Moyen / Quai", "#F5F3FF", "#6D28D9"));

    cardsLay->addWidget(makeCard("🔧",
                                 QString::number(maintenanceBerths),
                                 "En Maintenance", "#FEF2F2", "#991B1B"));

    mainLay->addWidget(cardsFrame);
    mainLay->addStretch();

    // ---- Close button at bottom ----
    QHBoxLayout* bottomLay = new QHBoxLayout();
    bottomLay->setContentsMargins(24, 0, 24, 20);
    QPushButton* okBtn = new QPushButton("  Fermer");
    okBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    okBtn->setFixedHeight(44);
    okBtn->setFixedWidth(160);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #2B5EA6, stop:1 #5D9CEC);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    bottomLay->addStretch();
    bottomLay->addWidget(okBtn);
    mainLay->addLayout(bottomLay);


    // Animate all wheels with a staggered delay
    for (int i = 0; i < progressWidgets.size(); ++i) {
        CircularProgress* cp = progressWidgets[i];
        QTimer::singleShot(i * 150, cp, [cp]() { cp->animateTo(); });
    }
    dlg->exec();
}
    void QuaisWindow::onGenerateContract(int row)
    {
        if (row < 0 || row >= quais.size()) return;

        const Quai& quai = quais[row];

        // Check if quai is available or occupied
        if (quai.getEtat() == "Maintenance") {
            QMessageBox::warning(this, "Quai en maintenance",
                                 "Ce quai est en maintenance et ne peut pas faire l'objet d'un contrat.");
            return;
        }

        // Create and show contract dialog
        ContractDialog dialog(quai, this);

        if (dialog.exec() == QDialog::Accepted) {
            QString clientName = dialog.getClientName();
            QString company = dialog.getCompany();
            QString duration = dialog.getDuration();
            QDate startDate = dialog.getStartDate();

            // Validate required fields
            if (clientName.isEmpty() || company.isEmpty() || duration.isEmpty()) {
                QMessageBox::warning(this, "Champs requis",
                                     "Veuillez remplir tous les champs obligatoires (nom, société, durée).");
                return;
            }

            QRegularExpression nameExp("^[a-zA-ZÀ-ÿ\\s]+$");
            if (!clientName.contains(nameExp)) {
                QMessageBox::warning(this, "Erreur de saisie", "Le nom ne doit contenir que des lettres.");
                return;
            }

            // Generate the contract
            if (ContractGenerator::generateContract(quai, clientName, company,
                                                    duration, startDate, this)) {
                // Optionally update quai status to "Occupé" if needed
                if (quai.getEtat() == "Disponible") {
                    int answer = QMessageBox::question(this, "Mettre à jour le statut",
                                                       "Souhaitez-vous marquer ce quai comme 'Occupé' ?",
                                                       QMessageBox::Yes | QMessageBox::No);

                    if (answer == QMessageBox::Yes) {
                        // Update quai status
                        Quai& q = quais[row]; // Get non-const reference
                        q.setEtat("Occupé");
                        q.setLocation(company);
                        populateTable(searchInput->text());
                    }
                }
            }
        }
    }

