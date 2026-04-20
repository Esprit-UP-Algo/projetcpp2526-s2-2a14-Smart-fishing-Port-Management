#include "quaiswindow.h"
#include <algorithm>
#include <limits>
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
#include <QVariantMap>
#include <QDateEdit>
#include <QPageSize>
#include <QSettings>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlRecord>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QMouseEvent>
#include <QEvent>

#include "addquaidialog.h"
#include "Bateauwindow.h"

static QString boatDisplayLabel(const QVariantMap& bateauInfo)
{
    const QString nom = bateauInfo.value("nom").toString();
    const QString immatriculation = bateauInfo.value("immatriculation").toString();
    if (immatriculation.isEmpty())
        return nom;
    return QString("%1 (%2)").arg(nom, immatriculation);
}

static bool isOccupiedState(const QString& etat)
{
    return etat.contains("occup", Qt::CaseInsensitive);
}

static QString formatRemainingTime(int totalSeconds)
{
    const int safeSeconds = std::max(0, totalSeconds);
    const int hours = safeSeconds / 3600;
    const int minutes = (safeSeconds % 3600) / 60;
    const int seconds = safeSeconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

static QString availabilityDeadlineSettingsKey(int quaiNumber)
{
    return QString("quais/availability_deadlines/%1").arg(quaiNumber);
}

static QString quaiSessionStartSettingsKey(int quaiNumber)
{
    return QString("quais/energy/session_start/%1").arg(quaiNumber);
}

static QString quaiSessionHistorySettingsKey(int quaiNumber)
{
    return QString("quais/energy/session_history/%1").arg(quaiNumber);
}

static QDateTime loadPersistedAvailabilityDeadline(int quaiNumber)
{
    QSettings settings("PortFlow", "PortFlow");
    return settings.value(availabilityDeadlineSettingsKey(quaiNumber)).toDateTime();
}

static void persistAvailabilityDeadline(int quaiNumber, const QDateTime& deadline)
{
    QSettings settings("PortFlow", "PortFlow");
    const QString key = availabilityDeadlineSettingsKey(quaiNumber);

    if (deadline.isValid())
        settings.setValue(key, deadline);
    else
        settings.remove(key);
}

static QDateTime loadPersistedSessionStart(int quaiNumber)
{
    QSettings settings("PortFlow", "PortFlow");
    return settings.value(quaiSessionStartSettingsKey(quaiNumber)).toDateTime();
}

static void persistSessionStart(int quaiNumber, const QDateTime& start)
{
    QSettings settings("PortFlow", "PortFlow");
    const QString key = quaiSessionStartSettingsKey(quaiNumber);

    if (start.isValid())
        settings.setValue(key, start);
    else
        settings.remove(key);
}

static QStringList loadPersistedSessionHistory(int quaiNumber)
{
    QSettings settings("PortFlow", "PortFlow");
    return settings.value(quaiSessionHistorySettingsKey(quaiNumber)).toStringList();
}

static void appendPersistedSessionHistory(int quaiNumber, const QDateTime& start, const QDateTime& end)
{
    if (!start.isValid() || !end.isValid() || end <= start)
        return;

    QSettings settings("PortFlow", "PortFlow");
    const QString key = quaiSessionHistorySettingsKey(quaiNumber);
    QStringList history = settings.value(key).toStringList();
    history.append(start.toString(Qt::ISODate) + "|" + end.toString(Qt::ISODate));

    static constexpr int kMaxSavedSessions = 20;
    while (history.size() > kMaxSavedSessions)
        history.removeFirst();

    settings.setValue(key, history);
}

struct DockUsageMonitoringAnalysis {
    int quaiNumber = 0;
    int sessionCount = 0;
    qint64 totalOccupiedSeconds = 0;
    qint64 averageOccupiedSeconds = 0;
    qint64 longestOccupiedSeconds = 0;
    double utilizationScore = 0.0;
    double anomalyScore = 0.0;
    QString statusLabel;
    QString recommendation;
    QString anomalySummary;
    QColor accentColor;
};

static void accumulateDockUsageMonitoring(DockUsageMonitoringAnalysis& analysis,
                                          const QDateTime& sessionStart, const QDateTime& sessionEnd)
{
    if (!sessionStart.isValid() || !sessionEnd.isValid() || sessionEnd <= sessionStart)
        return;

    const qint64 sessionSeconds = sessionStart.secsTo(sessionEnd);
    analysis.totalOccupiedSeconds += sessionSeconds;
    analysis.longestOccupiedSeconds = std::max(analysis.longestOccupiedSeconds, sessionSeconds);
    ++analysis.sessionCount;
}

static DockUsageMonitoringAnalysis buildDockUsageMonitoringAnalysis(const Quai& quai, const QHash<int, QDateTime>& deadlines)
{
    DockUsageMonitoringAnalysis analysis;
    analysis.quaiNumber = quai.getNumero();

    const QStringList history = loadPersistedSessionHistory(quai.getNumero());
    for (const QString& entry : history) {
        const QStringList parts = entry.split('|');
        if (parts.size() != 2)
            continue;

        const QDateTime start = QDateTime::fromString(parts[0], Qt::ISODate);
        const QDateTime end = QDateTime::fromString(parts[1], Qt::ISODate);
        accumulateDockUsageMonitoring(analysis, start, end);
    }

    const QDateTime activeSessionStart = loadPersistedSessionStart(quai.getNumero());
    if (activeSessionStart.isValid()) {
        QDateTime sessionEnd = QDateTime::currentDateTime();
        const auto deadlineIt = deadlines.constFind(quai.getNumero());
        if (deadlineIt != deadlines.constEnd() && deadlineIt.value().isValid())
            sessionEnd = std::min(sessionEnd, deadlineIt.value());

        accumulateDockUsageMonitoring(analysis, activeSessionStart, sessionEnd);
    }

    if (analysis.sessionCount > 0)
        analysis.averageOccupiedSeconds = analysis.totalOccupiedSeconds / analysis.sessionCount;

    const double averageHours = analysis.averageOccupiedSeconds / 3600.0;
    const double longestHours = analysis.longestOccupiedSeconds / 3600.0;
    const bool underUtilized = (analysis.sessionCount <= 1 && averageHours < 1.5)
                               || (analysis.sessionCount <= 2 && averageHours < 1.0);
    const bool overloadPattern = (analysis.sessionCount >= 4 && averageHours > 3.5)
                                 || longestHours > 8.0;
    const bool unjustifiedOccupation = isOccupiedState(quai.getEtat()) && longestHours > 6.0;

    analysis.utilizationScore = std::clamp((analysis.sessionCount * 18.0) + (averageHours * 12.0), 0.0, 100.0);
    if (underUtilized)
        analysis.anomalyScore += 38.0;
    if (overloadPattern)
        analysis.anomalyScore += 34.0;
    if (unjustifiedOccupation)
        analysis.anomalyScore += 42.0;
    analysis.anomalyScore = std::clamp(analysis.anomalyScore, 0.0, 100.0);

    QStringList anomalies;
    if (underUtilized)
        anomalies << "Sous-utilisation prolongee";
    if (overloadPattern)
        anomalies << "Surcharge frequente";
    if (unjustifiedOccupation)
        anomalies << "Occupation incoherente";
    if (anomalies.isEmpty())
        anomalies << "Aucune anomalie majeure";
    analysis.anomalySummary = anomalies.join(" | ");

    if (analysis.anomalyScore >= 65.0) {
        analysis.statusLabel = "Alerte critique";
        analysis.recommendation = "Reaffecter l'activite et verifier la logique d'occupation.";
        analysis.accentColor = QColor("#DC2626");
    } else if (analysis.anomalyScore >= 30.0) {
        analysis.statusLabel = "A surveiller";
        analysis.recommendation = "Surveiller la repartition et reduire les anomalies d'usage.";
        analysis.accentColor = QColor("#D97706");
    } else {
        analysis.statusLabel = "Utilisation stable";
        analysis.recommendation = "Performance operationnelle coherente.";
        analysis.accentColor = QColor("#059669");
    }

    return analysis;
}

static QString formatDurationLabel(qint64 totalSeconds)
{
    const qint64 safeSeconds = std::max<qint64>(0, totalSeconds);
    const qint64 hours = safeSeconds / 3600;
    const qint64 minutes = (safeSeconds % 3600) / 60;
    return QString("%1 h %2 min").arg(hours).arg(minutes, 2, 10, QChar('0'));
}

class DialogMoveFilter : public QObject
{
public:
    DialogMoveFilter(QDialog* dialog, QObject* parent = nullptr)
        : QObject(parent), m_dialog(dialog)
    {
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        Q_UNUSED(watched);

        if (!m_dialog)
            return QObject::eventFilter(watched, event);

        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragOffset = mouseEvent->globalPosition().toPoint() - m_dialog->frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                m_dialog->move(mouseEvent->globalPosition().toPoint() - m_dragOffset);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;
        }
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QDialog* m_dialog = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle)
{
    if (!dialog || !dragHandle)
        return;

    dragHandle->setCursor(Qt::OpenHandCursor);
    dragHandle->installEventFilter(new DialogMoveFilter(dialog, dragHandle));
}

static int estimateQuaiAvailabilityMinutes(int quaiNumber)
{
    static constexpr int kDefaultEstimateMinutes = 60;

    QSqlQuery query;
    query.prepare(
        "SELECT NVL(MAX(LONGEUR), 0) "
        "FROM   BATEAUX "
        "WHERE  IDQUAI = (SELECT IDQUAI FROM QUAIS WHERE NUMERO = :numero) "
        "  AND  ETAT IN ('Au port', 'En maintenance')"
        );
    query.bindValue(":numero", quaiNumber);

    const bool success = query.exec() && query.next();
    if (!success)
        return kDefaultEstimateMinutes;

    const int occupiedLength = query.value(0).toInt();
    if (occupiedLength <= 0)
        return kDefaultEstimateMinutes;

    return Quai::calculerTempsEstime(occupiedLength);
}

class ContractGenerator {
public:
    static bool generateContract(const Quai& quai, const QString& clientName,
                                 const QString& clientCompany, const QString& duration,
                                 const QDate& startDate, QWidget* parent)
    {
        QString fileName = QFileDialog::getSaveFileName(
            parent,
            "Enregistrer le contrat",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                + "/Contrat_Quai_" + QString::number(quai.getNumero()) + "_"
                + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
            "Fichiers PDF (*.pdf)"
            );

        if (fileName.isEmpty())
            return false;

        const QString contractNumber = QString("CT-%1-%2")
                                           .arg(quai.getNumero())
                                           .arg(QDate::currentDate().toString("yyyyMM"));

        const QString html = QString(R"(
<html>
<head>
<style>
    body { font-family: 'Segoe UI', Arial, sans-serif; color: #1f2937; margin: 0; padding: 0; }
    .header { background: #2B5EA6; color: white; text-align: center; padding: 30px 20px 20px 20px; }
    .header h1 { margin: 0; font-size: 28pt; letter-spacing: 4px; }
    .header h3 { margin: 6px 0 0 0; font-size: 11pt; font-weight: normal; letter-spacing: 2px; }
    .contract-ref { background: #EFF6FF; border-left: 5px solid #2B5EA6; padding: 12px 20px; margin: 20px 0 10px 0; font-size: 10pt; }
    .contract-ref b { font-size: 13pt; color: #2B5EA6; }
    .section-title { color: #2B5EA6; font-size: 13pt; font-weight: bold; border-bottom: 2px solid #5D9CEC; padding-bottom: 4px; margin-top: 22px; margin-bottom: 10px; }
    table.info { width: 100%%; border-collapse: collapse; }
    table.info td { padding: 7px 10px; font-size: 10pt; }
    table.info td.label { font-weight: bold; color: #374151; width: 40%%; }
    table.info td.value { color: #1f2937; }
    .quai-card { background: #F0F7FF; border: 1px solid #5D9CEC; border-radius: 6px; padding: 14px; margin: 10px 0; }
    .tarif-box { background: #2B5EA6; color: white; text-align: center; padding: 16px; font-size: 16pt; font-weight: bold; border-radius: 6px; margin: 10px 0; }
    .terms { font-size: 9pt; color: #4b5563; line-height: 1.8; }
    .terms li { margin-bottom: 3px; }
    .signature-section { margin-top: 40px; }
    table.sig { width: 100%%; }
    table.sig td { text-align: center; padding: 10px; font-size: 9pt; color: #6b7280; }
    .sig-line { border-top: 1px solid #9ca3af; width: 180px; display: inline-block; margin-bottom: 6px; }
    .footer { text-align: center; font-size: 8pt; color: #9ca3af; border-top: 1px solid #e5e7eb; padding-top: 10px; margin-top: 30px; }
</style>
</head>
<body>
<div class="header">
    <h1>⚓ PORTFLOW</h1>
    <h3>CONTRAT DE LOCATION DE QUAI</h3>
</div>
<div class="contract-ref">
    Contrat N° : <b>%1</b> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp; Date d'émission : <b>%2</b>
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
            <div class="sig-line">&nbsp;</div><br>
            Signature du locataire<br><i>%3</i>
        </td>
        <td>
            <div class="sig-line">&nbsp;</div><br>
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
                                 .arg(contractNumber)
                                 .arg(QDate::currentDate().toString("dd MMMM yyyy"))
                                 .arg(clientName)
                                 .arg(clientCompany)
                                 .arg(startDate.toString("dd MMMM yyyy"))
                                 .arg(duration)
                                 .arg(quai.getReference())
                                 .arg(quai.getNomQuai())
                                 .arg(quai.getCapacite())
                                 .arg(quai.getCapacite())
                                 .arg(quai.getEtat())
                                 .arg(quai.getTarif(), 0, 'f', 2);

        QTextDocument doc;
        doc.setPageSize(QSizeF(595, 842));
        doc.setHtml(html);

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
        doc.print(&printer);

        QMessageBox::information(
            parent,
            "Succès",
            QString("Le contrat a été généré avec succès !\n%1").arg(fileName)
            );

        return true;
    }
};

class ContractDialog : public QDialog {
public:
    ContractDialog(const Quai& quai, QWidget* parent = nullptr)
        : QDialog(parent), m_quai(quai)
    {
        setFixedSize(640, 720);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 80));

        QWidget* container = new QWidget(this);
        container->setGeometry(10, 10, 620, 700);
        container->setGraphicsEffect(shadow);
        container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

        QVBoxLayout* mainLay = new QVBoxLayout(container);
        mainLay->setContentsMargins(0, 0, 0, 0);
        mainLay->setSpacing(0);

        // Header
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
        makeDialogMovable(this, header);

        // Quai preview card
        QFrame* previewCard = new QFrame();
        previewCard->setFixedHeight(88);
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
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setFixedSize(42, 42);
        previewLay->addWidget(iconLbl);

        QVBoxLayout* infoLay = new QVBoxLayout();
        infoLay->setSpacing(2);

        QLabel* titlePreview = new QLabel(quai.getNomQuai() + "  —  " + quai.getReference());
        titlePreview->setFont(QFont("Segoe UI", 11, QFont::Bold));
        titlePreview->setStyleSheet("color: #065F46; background: transparent; border: none;");
        titlePreview->setWordWrap(true);

        QLabel* detailsPreview = new QLabel(
            QString("Capacité: %1  |  Tarif: %2 DT/j")
                .arg(quai.getCapacite())
                .arg(quai.getTarif())
            );
        detailsPreview->setFont(QFont("Segoe UI", 9));
        detailsPreview->setStyleSheet("color: #6b7280; background: transparent; border: none;");
        detailsPreview->setWordWrap(true);

        infoLay->addWidget(titlePreview);
        infoLay->addWidget(detailsPreview);
        previewLay->addLayout(infoLay, 1);
        mainLay->addWidget(previewCard);

        // Form area
        QWidget* formArea = new QWidget();
        formArea->setStyleSheet("background: transparent;");
        QVBoxLayout* formLay = new QVBoxLayout(formArea);
        formLay->setContentsMargins(24, 18, 24, 10);
        formLay->setSpacing(8);

        auto addField = [&](const QString& labelText, QLineEdit*& fieldPtr,
                            const QString& placeholder, QLabel*& errLblPtr)
        {
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

        QLabel* nameErr  = nullptr;
        QLabel* compErr  = nullptr;
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

        // Date + Duration row
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

        // Buttons
        QHBoxLayout* btnLay = new QHBoxLayout();
        btnLay->setContentsMargins(24, 10, 24, 24);
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
    setStyleSheet("QMainWindow { background-color: #F0F4F8; }");

    setupUI();
    setupQuaiTable();
    availabilityRefreshTimer = new QTimer(this);
    availabilityRefreshTimer->setInterval(1000);
    connect(availabilityRefreshTimer, &QTimer::timeout, this, &QuaisWindow::refreshAvailabilityCountdowns);
    availabilityRefreshTimer->start();
    loadQuaisFromDatabase();
    populateTable();
}

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
    logoContainer->setStyleSheet("QFrame { background-color: transparent; border-radius: 0px; }");

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
        logoLabel->setStyleSheet("QLabel { font-size: 50px; color: white; }");
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
    QPushButton* btn = new QPushButton(icon + "  " + text);
    btn->setFont(QFont("Segoe UI", 12, QFont::Medium));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(55);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255,255,255,0.1); color: white; border: none;
                          border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(239,68,68,0.8); }
        )");
        connect(btn, &QPushButton::clicked, this, &QuaisWindow::onLogout);
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton { background-color: rgba(255,255,255,0.25); color: white; border: none;
                          border-radius: 12px; text-align: left; padding-left: 20px; font-weight: bold; }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton { background-color: transparent; color: rgba(255,255,255,0.9); border: none;
                          border-radius: 12px; text-align: left; padding-left: 20px; }
            QPushButton:hover { background-color: rgba(255,255,255,0.15); }
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
    hdr->setStyleSheet("background: transparent;");
    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    QLabel* title = new QLabel("Gestion des Quais");
    title->setFont(QFont("Segoe UI", 26, QFont::Bold));
    title->setStyleSheet("color: #1e3a5f;");
    QLabel* sub = new QLabel("Administration des emplacements et disponibilités");
    sub->setFont(QFont("Segoe UI", 10));
    sub->setStyleSheet("color: #6b7280;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    lay->addLayout(titleCol, 1);

    auto makeBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(45);
        btn->setMinimumWidth(160);
        btn->setStyleSheet(QString(
                               "QPushButton { background: %1; color: white; border: none; border-radius: 12px; padding: 0 20px; }"
                               "QPushButton:hover { background: %2; }"
                               ).arg(bg, hover));
        return btn;
    };

    QPushButton* statsBtn = makeBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    connect(statsBtn, &QPushButton::clicked, this, &QuaisWindow::afficherStatistiques);

    QPushButton* pdfBtn = makeBtn("📄  Exporter PDF", "#059669", "#047857");
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
        container->setGeometry(10, 10, 540, 440);
        container->setGraphicsEffect(shadow);
        container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

        QVBoxLayout* mainLay = new QVBoxLayout(container);
        mainLay->setContentsMargins(0, 0, 0, 0);
        mainLay->setSpacing(0);

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
        makeDialogMovable(dlg, header);

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

        QListWidget* quaiList = new QListWidget();
        quaiList->setFont(QFont("Segoe UI", 10));
        quaiList->setStyleSheet(R"(
            QListWidget {
                background: #F9FAFB; border: 2px solid #E5E7EB;
                border-radius: 12px; padding: 6px; outline: none;
            }
            QListWidget::item { padding: 10px 14px; border-radius: 8px; color: #1f2937; }
            QListWidget::item:selected { background: #D1FAE5; color: #065F46; font-weight: bold; }
            QListWidget::item:hover:!selected { background: #F3F4F6; }
        )");

        for (int i = 0; i < quais.size(); ++i) {
            const Quai& q = quais[i];
            const QString emoji = (q.getEtat() == "Disponible") ? "🟢"
                                  : (q.getEtat() == "Occupé")     ? "🟡" : "🔴";
            const QString text = QString("%1  %2  —  Quai %2  |  %3 DT/j")
                                     .arg(emoji)
                                     .arg(q.getNumero())
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
            const int row = current->data(Qt::UserRole).toInt();
            dlg->accept();
            onGenerateContract(row);
        });

        btnLay->addStretch();
        btnLay->addWidget(cancelBtn);
        btnLay->addWidget(generateBtn);
        mainLay->addLayout(btnLay);

        dlg->exec();
    });

    QPushButton* smartAssignBtn = makeBtn("IA  Affecter un bateau", "#EA580C", "#C2410C");
    connect(smartAssignBtn, &QPushButton::clicked, this, &QuaisWindow::onAutoAssignBoat);

    QPushButton* addBtn = makeBtn("➕  Nouveau Quai", "#2563EB", "#1D4ED8");
    connect(addBtn, &QPushButton::clicked, this, &QuaisWindow::onAddQuai);

    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(smartAssignBtn);
    lay->addWidget(addBtn);
    return hdr;
}

QFrame* QuaisWindow::createToolbar()
{
    QFrame* bar = new QFrame();
    bar->setStyleSheet(R"(
        QFrame { background: white; border-radius: 14px; border: 1.5px solid #e2e8f0; }
    )");
    bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bar->setMinimumHeight(62);

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(12);

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("🔍  Rechercher un quai par numéro, type...");
    searchInput->setFixedWidth(350);
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px;
                    padding-left: 12px; font-size: 13px; color: #334155; }
        QLineEdit:focus { border: 1.5px solid #3b82f6; background: white; }
    )");
    connect(searchInput, &QLineEdit::textChanged, this, &QuaisWindow::onSearch);
    lay->addWidget(searchInput);
    lay->addStretch();

    QLabel* sortLbl = new QLabel("Trier par :");
    sortLbl->setStyleSheet("color: #64748b; font-weight: 600; border: none; background: transparent;");
    lay->addWidget(sortLbl);

    sortCombo = new QComboBox();
    sortCombo->setFixedWidth(200);
    sortCombo->setFixedHeight(40);
    sortCombo->addItems({"Par défaut", "Numéro ↑", "Numéro ↓", "Tarif ↑", "Tarif ↓"});
    sortCombo->setStyleSheet(R"(
        QComboBox { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px;
                    padding: 0 12px; color: #334155; }
        QComboBox::drop-down { border: none; }
        QComboBox::down-arrow { image: none; border-left: 5px solid transparent;
                                border-right: 5px solid transparent;
                                border-top: 5px solid #64748b; margin-right: 8px; }
    )");
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuaisWindow::onSort);
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

    QWidget* warningsPanel = new QWidget();
    warningsPanel->setStyleSheet(R"(
        QWidget { background-color: #F8FAFC; border: 1px solid #CBD5E1; border-radius: 16px; }
        QLabel { color: #1F2937; font-family: 'Segoe UI'; }
    )");
    QVBoxLayout* warningsLayout = new QVBoxLayout(warningsPanel);
    warningsLayout->setContentsMargins(16, 16, 16, 16);
    warningsLayout->setSpacing(6);

    QLabel* warningsHeader = new QLabel("Warnings");
    warningsHeader->setFont(QFont("Segoe UI", 11, QFont::DemiBold));
    warningsLayout->addWidget(warningsHeader);

    warningsContentLabel = new QLabel("No warnings for the moment");
    warningsContentLabel->setWordWrap(true);
    warningsContentLabel->setFont(QFont("Segoe UI", 10));
    warningsLayout->addWidget(warningsContentLabel);

    containerLayout->addWidget(warningsPanel);
    layout->addWidget(whiteContainer);

    return card;
}

void QuaisWindow::setupQuaiTable()
{
    quaiTable->setColumnCount(7);
    quaiTable->setHorizontalHeaderLabels({
        "Référence", "Nom du Quai", "Capacité", "Localisation", "Statut", "Tarif / Durée", "Actions"
    });
    quaiTable->horizontalHeader()->setStretchLastSection(true);
    quaiTable->verticalHeader()->setVisible(false);
    quaiTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    quaiTable->setSelectionMode(QAbstractItemView::SingleSelection);
    quaiTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    quaiTable->setShowGrid(true);
    quaiTable->horizontalHeader()->setFont(QFont("Segoe UI", 11, QFont::Bold));
    quaiTable->horizontalHeader()->setFixedHeight(50);
    quaiTable->setStyleSheet(R"(
        QTableWidget { background-color: white; border: 2px solid #d1d5db;
                       border-radius: 16px; gridline-color: #d1d5db; }
        QTableWidget::item { padding: 12px; border-right: 1px solid #d1d5db;
                             border-bottom: 1px solid #d1d5db; color: #1f2937;
                             background-color: white; font-family: 'Segoe UI'; font-size: 11pt; }
        QTableWidget::item:selected { background-color: #EBF5FF; color: #2563EB; }
        QHeaderView::section { background-color: #d1d5db; color: #1f2937; padding: 12px;
                               border: none; font-weight: 600;
                               font-family: 'Segoe UI'; font-size: 11pt; }
    )");
    quaiTable->setColumnWidth(0, 140);
    quaiTable->setColumnWidth(1, 120);
    quaiTable->setColumnWidth(2, 100);
    quaiTable->setColumnWidth(3, 160);
    quaiTable->setColumnWidth(4, 130);
    quaiTable->setColumnWidth(5, 150);
    connect(quaiTable, &QTableWidget::cellClicked, this, &QuaisWindow::onQuaiCellClicked);
}

void QuaisWindow::loadQuaisFromDatabase()
{
    quais.clear();
    QList<int> expiredQuais;

    QSqlQuery query;
    if (!query.exec("SELECT NUMERO, CAPACITE, LOCATION, ETAT, TARIF_LOCATION, DUREE_LOCATION "
                    "FROM QUAIS ORDER BY IDQUAI")) {
        qDebug() << "Database query error:" << query.lastError().text();
        return;
    }

    int ordreNom = 1;
    while (query.next()) {
        const int    numero        = query.value("NUMERO").toInt();
        const int    capacite      = query.value("CAPACITE").toInt();
        const QString etat         = query.value("ETAT").toString();
        const double tarifLocation = query.value("TARIF_LOCATION").toDouble();
        const QString location     = query.value("LOCATION").toString();
        const QString dureeLocation = query.value("DUREE_LOCATION").toString();

        Quai q(numero, capacite, etat, tarifLocation, location, dureeLocation);
        q.setOrdreNom(ordreNom++);
        quais.append(q);

        if (isOccupiedState(etat)) {
            if (!loadPersistedSessionStart(numero).isValid()) {
                QDateTime sessionStart = QDateTime::currentDateTime().addSecs(-estimateCountdownSeconds(q));
                const QDateTime persistedDeadline = loadPersistedAvailabilityDeadline(numero);
                if (persistedDeadline.isValid())
                    sessionStart = persistedDeadline.addSecs(-estimateCountdownSeconds(q));
                persistSessionStart(numero, sessionStart);
            }
            ensureAvailabilityTimerForQuai(q);
            if (remainingAvailabilitySeconds(numero) <= 0)
                expiredQuais.append(numero);
        } else {
            quaiAvailabilityDeadlines.remove(numero);
            persistAvailabilityDeadline(numero, QDateTime());
            persistSessionStart(numero, QDateTime());
        }
    }

    if (!expiredQuais.isEmpty()) {
        for (int numero : expiredQuais) {
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE NUMERO = :numero");
            updateQuery.bindValue(":numero", numero);

            if (!updateQuery.exec()) {
                qDebug() << "Failed to free expired quai" << numero << ":" << updateQuery.lastError().text();
                continue;
            }

            quaiAvailabilityDeadlines.remove(numero);
            persistAvailabilityDeadline(numero, QDateTime());
            persistSessionStart(numero, QDateTime());
        }

        loadQuaisFromDatabase();
    }
}

void QuaisWindow::populateTable(const QString& filterText)
{
    quaiTable->setRowCount(0);
    QStringList warningEntries;

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];

        if (!filterText.isEmpty()) {
            const QString searchLower = filterText.toLower();
            if (!q.getNomQuai().toLower().contains(searchLower) &&
                !q.getReference().toLower().contains(searchLower) &&
                !q.getEtat().toLower().contains(searchLower) &&
                !q.getDureeLocation().toLower().contains(searchLower) &&
                !QString::number(q.getNumero()).contains(searchLower))
                continue;
        }

        const int row = quaiTable->rowCount();
        quaiTable->insertRow(row);
        quaiTable->setRowHeight(row, 65);

        QTableWidgetItem* numeroItem = new QTableWidgetItem(q.getReference());
        numeroItem->setForeground(QBrush(QColor("#5D9CEC")));
        numeroItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
        numeroItem->setData(Qt::UserRole, q.getNumero());
        const DockUsageMonitoringAnalysis monitoringAnalysis = buildDockUsageMonitoringAnalysis(q, quaiAvailabilityDeadlines);
        if (monitoringAnalysis.anomalyScore >= 65.0) {
            numeroItem->setBackground(QColor("#FEF2F2"));
            numeroItem->setToolTip(QString("Monitoring intelligent: %1")
                                       .arg(monitoringAnalysis.anomalySummary));
        } else if (monitoringAnalysis.anomalyScore >= 30.0) {
            numeroItem->setBackground(QColor("#FFF7ED"));
            numeroItem->setToolTip(QString("Monitoring intelligent: %1")
                                       .arg(monitoringAnalysis.anomalySummary));
        }
        quaiTable->setItem(row, 0, numeroItem);

        quaiTable->setItem(row, 1, new QTableWidgetItem(q.getNomQuai()));
        quaiTable->setItem(row, 2, new QTableWidgetItem(QString::number(q.getCapacite())));
        quaiTable->setItem(row, 3, new QTableWidgetItem(q.getLocation()));
        quaiTable->setCellWidget(row, 4, createStatusBadge(q.getEtat()));
        quaiTable->setItem(row, 5, new QTableWidgetItem(
                                       QString("%1 DT / %2").arg(q.getTarif()).arg(q.getDureeLocation())
                                       ));
        quaiTable->setCellWidget(row, 6, createActionButtons(i));

        if (monitoringAnalysis.anomalyScore >= 30.0)
            warningEntries << QString("Quai %1 : %2").arg(q.getNumero()).arg(monitoringAnalysis.anomalySummary);
    }

    if (warningEntries.isEmpty())
        warningsContentLabel->setText("No warnings for the moment");
    else
        warningsContentLabel->setText(warningEntries.join("\n"));
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

    if (status == "Disponible")
        badge->setStyleSheet("QLabel { background-color: #D1FAE5; color: #065F46; border-radius: 8px; padding: 6px 16px; }");
    else if (isOccupiedState(status))
        badge->setStyleSheet("QLabel { background-color: #FEF3C7; color: #92400E; border-radius: 8px; padding: 6px 16px; }");
    else
        badge->setStyleSheet("QLabel { background-color: #FEE2E2; color: #991B1B; border-radius: 8px; padding: 6px 16px; }");

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

    auto makeBtn = [&](const QString& icon, const QString& bg,
                       const QString& hover, const QString& tooltip = "") {
        QPushButton* btn = new QPushButton(icon);
        btn->setFixedSize(36, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setStyleSheet(QString(
                               "QPushButton { background-color: %1; border: none; border-radius: 8px; font-size: 16px; }"
                               "QPushButton:hover { background-color: %2; }"
                               ).arg(bg, hover));
        return btn;
    };

    QPushButton* editBtn = makeBtn("✏️", "#FEF3C7", "#FDE68A", "Modifier le quai");
    connect(editBtn, &QPushButton::clicked, [this, row]() { onEditQuai(row); });
    layout->addWidget(editBtn);

    QPushButton* deleteBtn = makeBtn("🗑️", "#FEE2E2", "#FECACA", "Supprimer le quai");
    connect(deleteBtn, &QPushButton::clicked, [this, row]() { onDeleteQuai(row); });
    layout->addWidget(deleteBtn);

    QPushButton* contractBtn = makeBtn("📄", "#E0F2FE", "#BAE6FD", "Générer un contrat");
    connect(contractBtn, &QPushButton::clicked, [this, row]() {
        if (row >= 0 && row < quais.size())
            onGenerateContract(row);
    });
    layout->addWidget(contractBtn);

    return widget;
}

int QuaisWindow::findQuaiIndexByNumero(int numero) const
{
    for (int i = 0; i < quais.size(); ++i) {
        if (quais[i].getNumero() == numero)
            return i;
    }
    return -1;
}

void QuaisWindow::ensureAvailabilityTimerForQuai(const Quai& quai)
{
    if (!loadPersistedSessionStart(quai.getNumero()).isValid())
        persistSessionStart(quai.getNumero(), QDateTime::currentDateTime());

    if (quaiAvailabilityDeadlines.contains(quai.getNumero()))
        return;

    QDateTime deadline = loadPersistedAvailabilityDeadline(quai.getNumero());
    if (!deadline.isValid()) {
        deadline = QDateTime::currentDateTime().addSecs(estimateCountdownSeconds(quai));
        persistAvailabilityDeadline(quai.getNumero(), deadline);
    }

    quaiAvailabilityDeadlines.insert(quai.getNumero(), deadline);
}

int QuaisWindow::estimateCountdownSeconds(const Quai& quai) const
{
    int minutes = estimateQuaiAvailabilityMinutes(quai.getNumero());
    if (minutes <= 0)
        minutes = Quai::calculerTempsEstime(quai.getCapacite());
    if (minutes <= 0)
        minutes = 60;
    return std::max(60, minutes * 60);
}

int QuaisWindow::remainingAvailabilitySeconds(int numero) const
{
    const auto it = quaiAvailabilityDeadlines.constFind(numero);
    if (it == quaiAvailabilityDeadlines.constEnd())
        return 0;

    const qint64 remaining = QDateTime::currentDateTime().secsTo(it.value());
    return static_cast<int>(std::max<qint64>(0, remaining));
}

void QuaisWindow::markQuaiAsAvailable(int numero, bool showNotification)
{
    QSqlQuery query;
    query.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE NUMERO = :numero");
    query.bindValue(":numero", numero);

    if (!query.exec()) {
        qDebug() << "Failed to free quai" << numero << ":" << query.lastError().text();
        return;
    }

    const QDateTime sessionStart = loadPersistedSessionStart(numero);
    const QDateTime sessionEnd = QDateTime::currentDateTime();
    appendPersistedSessionHistory(numero, sessionStart, sessionEnd);
    quaiAvailabilityDeadlines.remove(numero);
    persistAvailabilityDeadline(numero, QDateTime());
    persistSessionStart(numero, QDateTime());

    loadQuaisFromDatabase();
    populateTable(searchInput->text());

    if (showNotification) {
        QMessageBox::information(this,
                                 "Quai disponible",
                                 QString("Le quai %1 est de nouveau disponible.").arg(numero));
    }
}

void QuaisWindow::showAvailabilityCountdownPopup(int numero)
{
    const int quaiIndex = findQuaiIndexByNumero(numero);
    if (quaiIndex < 0)
        return;

    const Quai& quai = quais[quaiIndex];
    QDialog dialog(this);
    dialog.setWindowTitle("Disponibilite du quai");
    dialog.setFixedSize(560, 360);
    dialog.setModal(true);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(&dialog);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(&dialog);
    container->setGeometry(10, 10, 540, 340);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(78);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2B5EA6, stop:1 #5D9CEC);
            border-radius: 24px 24px 0 0;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(28, 0, 18, 0);

    QLabel* title = new QLabel(QString("Timer du quai %1").arg(quai.getNumero()), headerBand);
    title->setFont(QFont("Segoe UI", 16, QFont::Bold));
    title->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(title, 1);

    QPushButton* closeBtn = new QPushButton("X", headerBand);
    closeBtn->setFixedSize(34, 34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 17px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    layout->addWidget(headerBand);
    makeDialogMovable(&dialog, headerBand);

    QWidget* body = new QWidget(container);
    body->setStyleSheet("background: transparent;");
    QVBoxLayout* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(28, 22, 28, 0);
    bodyLay->setSpacing(14);

    QLabel* subtitle = new QLabel(body);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color: #64748b; font-size: 13px;");
    bodyLay->addWidget(subtitle);

    QLabel* countdown = new QLabel(body);
    countdown->setAlignment(Qt::AlignCenter);
    countdown->setMinimumHeight(116);
    countdown->setStyleSheet(
        "QLabel { background: #EFF6FF; color: #1D4ED8; border: 2px solid #BFDBFE; "
        "border-radius: 18px; font: 700 28px 'Consolas'; padding: 14px 10px; }"
        );
    bodyLay->addWidget(countdown);

    QLabel* status = new QLabel(body);
    status->setAlignment(Qt::AlignCenter);
    status->setWordWrap(true);
    status->setStyleSheet("color: #0f766e; font-size: 12px; background: #F8FAFC; border-radius: 12px; padding: 12px;");
    bodyLay->addWidget(status);
    layout->addWidget(body, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(28, 14, 28, 24);
    btnLay->addStretch();

    QPushButton* closeActionBtn = new QPushButton("Fermer", container);
    closeActionBtn->setFixedHeight(42);
    closeActionBtn->setMinimumWidth(110);
    closeActionBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    closeActionBtn->setCursor(Qt::PointingHandCursor);
    closeActionBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none;
                      border-radius: 12px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(closeActionBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    btnLay->addWidget(closeActionBtn);
    layout->addLayout(btnLay);

    QTimer popupTimer(&dialog);
    popupTimer.setInterval(1000);

    auto refreshPopup = [this, numero, quai, subtitle, countdown, status, &dialog]() {
        const bool occupied = isOccupiedState(quai.getEtat()) || quaiAvailabilityDeadlines.contains(numero);

        if (!occupied) {
            subtitle->setText("Ce quai est actuellement disponible.");
            countdown->setText("Disponible");
            status->setText("Aucun compte a rebours actif pour ce quai.");
            return;
        }

        ensureAvailabilityTimerForQuai(quai);
        const int remaining = remainingAvailabilitySeconds(numero);
        subtitle->setText("Temps restant avant retour au statut disponible :");
        countdown->setText(formatRemainingTime(remaining));

        if (remaining <= 0) {
            markQuaiAsAvailable(numero, false);
            status->setText("Le quai est disponible.");
            dialog.accept();
            return;
        }

        status->setText("Le statut repassera automatiquement à Disponible à la fin du compte à rebours.");
    };

    connect(&popupTimer, &QTimer::timeout, &dialog, refreshPopup);
    refreshPopup();
    popupTimer.start();
    dialog.exec();
}

void QuaisWindow::onQuaiCellClicked(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem* numeroItem = quaiTable->item(row, 0);
    if (!numeroItem)
        return;

    const int numero = numeroItem->data(Qt::UserRole).toInt();
    const int quaiIndex = findQuaiIndexByNumero(numero);
    if (quaiIndex < 0)
        return;

    showAvailabilityCountdownPopup(numero);
}

void QuaisWindow::refreshAvailabilityCountdowns()
{
    QList<int> expiredQuais;
    for (auto it = quaiAvailabilityDeadlines.constBegin(); it != quaiAvailabilityDeadlines.constEnd(); ++it) {
        if (QDateTime::currentDateTime() >= it.value())
            expiredQuais.append(it.key());
    }

    for (int numero : expiredQuais)
        markQuaiAsAvailable(numero);
}

bool QuaisWindow::assignerQuaiAutomatiquement(const QVariantMap& bateauInfo, Quai& quaiChoisi,
                                              int& tempsEstime, QString& explication)
{
    const int longueur       = bateauInfo.value("longueur").toInt();
    const QString etatBateau = bateauInfo.value("etat").toString();
    tempsEstime = Quai::calculerTempsEstime(longueur);

    int meilleurIndex = -1;
    int meilleurScore = std::numeric_limits<int>::max();

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];
        if (q.getEtat() != "Disponible" || !q.peutAccueillirLongueur(longueur))
            continue;

        const int ecartCapacite = q.getCapacite() - longueur;
        const int score = (ecartCapacite * 100) + static_cast<int>(q.getTarif() * 10.0);
        if (score < meilleurScore) {
            meilleurScore = score;
            meilleurIndex = i;
        }
    }

    if (meilleurIndex >= 0) {
        quaiChoisi = quais[meilleurIndex];
        explication = QString("Temps estimé : %1 min. Quai %2 retenu avec une longueur maximale de %3 m pour un bateau de %4 m.")
                          .arg(tempsEstime)
                          .arg(quaiChoisi.getNumero())
                          .arg(quaiChoisi.getCapacite())
                          .arg(longueur);
        return true;
    }

    if (etatBateau != "En mer") {
        explication = QString("Aucun quai disponible ne peut accueillir un bateau de %1 m. "
                              "La capacité du quai représente la longueur maximale autorisée.")
                          .arg(longueur);
        return false;
    }

    int meilleurQuaiReserve  = -1;
    int meilleurDelai        = std::numeric_limits<int>::max();
    int meilleurScoreReserve = std::numeric_limits<int>::max();

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];
        if (q.getEtat() == "Maintenance" || !q.peutAccueillirLongueur(longueur))
            continue;

        const int delaiDisponibilite = (q.getEtat() == "Disponible")
                                           ? 0 : estimateQuaiAvailabilityMinutes(q.getNumero());
        const int ecartCapacite = q.getCapacite() - longueur;
        const int score = (delaiDisponibilite * 1000)
                          + (ecartCapacite * 100)
                          + static_cast<int>(q.getTarif() * 10.0);

        if (score < meilleurScoreReserve) {
            meilleurScoreReserve = score;
            meilleurDelai        = delaiDisponibilite;
            meilleurQuaiReserve  = i;
        }
    }

    if (meilleurQuaiReserve < 0) {
        explication = QString("Aucun quai actuel ou futur n'est compatible avec un bateau de %1 m.")
        .arg(longueur);
        return false;
    }

    quaiChoisi = quais[meilleurQuaiReserve];
    explication = QString("Temps estimé du bateau : %1 min. Aucun quai n'est libre, "
                          "donc le quai %2 est réservé en second choix avec une disponibilité estimée dans %3 min.")
                      .arg(tempsEstime)
                      .arg(quaiChoisi.getNumero())
                      .arg(meilleurDelai);
    return true;
}

void QuaisWindow::onSearch(const QString& text)
{
    populateTable(text);
}

void QuaisWindow::onAddQuai()
{
    AddQuaiDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        loadQuaisFromDatabase();
        populateTable(searchInput->text());
    }
}
void QuaisWindow::onAutoAssignBoat()
{
    // ── Reusable styled popup helpers ────────────────────────────────────────
    auto showStyledDialog = [&](const QString& title, const QString& message,
                                const QString& gradientStart, const QString& gradientEnd,
                                const QString& icon, bool hasCancel = false) -> bool
    {
        QDialog* popup = new QDialog(this);
        popup->setFixedSize(460, 240);
        popup->setModal(true);
        popup->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        popup->setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(popup);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 80));

        QWidget* container = new QWidget(popup);
        container->setGeometry(10, 10, 440, 220);
        container->setGraphicsEffect(shadow);
        container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

        QVBoxLayout* lay = new QVBoxLayout(container);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);

        // Header
        QFrame* hdr = new QFrame();
        hdr->setFixedHeight(64);
        hdr->setStyleSheet(QString(R"(
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 %1, stop:1 %2);
                border-radius: 20px 20px 0 0;
            }
        )").arg(gradientStart, gradientEnd));
        QHBoxLayout* hdrLay = new QHBoxLayout(hdr);
        hdrLay->setContentsMargins(22, 0, 16, 0);

        QLabel* hdrIcon = new QLabel(icon);
        hdrIcon->setFont(QFont("Segoe UI", 18));
        hdrIcon->setStyleSheet("background: transparent;");
        hdrLay->addWidget(hdrIcon);

        QLabel* hdrTitle = new QLabel(title);
        hdrTitle->setFont(QFont("Segoe UI", 13, QFont::Bold));
        hdrTitle->setStyleSheet("color: white; background: transparent;");
        hdrLay->addWidget(hdrTitle, 1);

        QPushButton* xBtn = new QPushButton("✕");
        xBtn->setFixedSize(30, 30);
        xBtn->setCursor(Qt::PointingHandCursor);
        xBtn->setStyleSheet(R"(
            QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                          border-radius: 15px; font-size: 12px; font-weight: bold; }
            QPushButton:hover { background: rgba(255,255,255,0.4); }
        )");
        connect(xBtn, &QPushButton::clicked, popup, &QDialog::reject);
        hdrLay->addWidget(xBtn);
        lay->addWidget(hdr);
        makeDialogMovable(popup, hdr);

        // Message
        QLabel* msgLbl = new QLabel(message);
        msgLbl->setFont(QFont("Segoe UI", 10));
        msgLbl->setStyleSheet("color: #374151; background: transparent;");
        msgLbl->setWordWrap(true);
        msgLbl->setAlignment(Qt::AlignCenter);
        msgLbl->setContentsMargins(24, 16, 24, 0);
        lay->addWidget(msgLbl, 1);

        // Buttons
        QHBoxLayout* btnLay = new QHBoxLayout();
        btnLay->setContentsMargins(20, 8, 20, 18);
        btnLay->setSpacing(10);
        btnLay->addStretch();

        bool result = false;

        if (hasCancel) {
            QPushButton* noBtn = new QPushButton("Non");
            noBtn->setFixedHeight(38);
            noBtn->setMinimumWidth(90);
            noBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
            noBtn->setCursor(Qt::PointingHandCursor);
            noBtn->setStyleSheet(R"(
                QPushButton { background: #F3F4F6; color: #374151; border: none;
                              border-radius: 10px; padding: 0 16px; }
                QPushButton:hover { background: #E5E7EB; }
            )");
            connect(noBtn, &QPushButton::clicked, popup, &QDialog::reject);
            btnLay->addWidget(noBtn);
        }

        QPushButton* okBtn = new QPushButton(hasCancel ? "Oui, continuer" : "Compris");
        okBtn->setFixedHeight(38);
        okBtn->setMinimumWidth(hasCancel ? 130 : 100);
        okBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        okBtn->setCursor(Qt::PointingHandCursor);
        okBtn->setStyleSheet(QString(R"(
            QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                              stop:0 %1, stop:1 %2);
                          color: white; border: none; border-radius: 10px; padding: 0 16px; }
            QPushButton:hover { background: %1; }
        )").arg(gradientStart, gradientEnd));
        connect(okBtn, &QPushButton::clicked, popup, &QDialog::accept);
        btnLay->addWidget(okBtn);

        lay->addLayout(btnLay);

        result = (popup->exec() == QDialog::Accepted);
        popup->deleteLater();
        return result;
    };

    auto showError = [&](const QString& title, const QString& msg) {
        showStyledDialog(title, msg, "#991B1B", "#EF4444", "❌");
    };
    auto showWarning = [&](const QString& title, const QString& msg) {
        showStyledDialog(title, msg, "#92400E", "#F59E0B", "⚠️");
    };
    auto showInfo = [&](const QString& title, const QString& msg) {
        showStyledDialog(title, msg, "#1D4ED8", "#60A5FA", "ℹ️");
    };
    auto showSuccess = [&](const QString& title, const QString& msg) {
        showStyledDialog(title, msg, "#065F46", "#34D399", "✅");
    };

    // ── Load boats ───────────────────────────────────────────────────────────
    const bool hasAvailableQuai = std::any_of(quais.begin(), quais.end(), [](const Quai& q) {
        return q.getEtat() == "Disponible";
    });

    QSqlQuery boatQuery;
    if (!boatQuery.exec(
            "SELECT IDBATEAU, NOMBATEAU, IMMATRICULATION, NVL(LONGEUR, 0) AS LONGEUR, "
            "       NVL(CAPACITE, 0) AS CAPACITE, ETAT "
            "FROM BATEAUX "
            "WHERE ETAT IN ('Au port', 'En maintenance', 'En mer') "
            "ORDER BY CASE "
            "    WHEN ETAT = 'Au port'        THEN 1 "
            "    WHEN ETAT = 'En maintenance' THEN 2 "
            "    WHEN ETAT = 'En mer'         THEN 3 "
            "    ELSE 4 END, NOMBATEAU")) {
        showError("Erreur base de données",
                  "Impossible de charger les bateaux :\n" + boatQuery.lastError().text());
        return;
    }

    QList<QVariantMap> bateaux;
    while (boatQuery.next()) {
        QVariantMap bateau;
        bateau.insert("id",              boatQuery.value("IDBATEAU").toString());
        bateau.insert("nom",             boatQuery.value("NOMBATEAU").toString());
        bateau.insert("immatriculation", boatQuery.value("IMMATRICULATION").toString());
        bateau.insert("longueur",        boatQuery.value("LONGEUR").toInt());
        bateau.insert("capacite",        boatQuery.value("CAPACITE").toInt());
        bateau.insert("etat",            boatQuery.value("ETAT").toString());
        bateaux.append(bateau);
    }

    if (bateaux.isEmpty()) {
        showWarning("Aucun bateau disponible",
                    "Aucun bateau n'est actuellement disponible\npour une affectation automatique.");
        return;
    }

    // ── Main dialog ──────────────────────────────────────────────────────────
    QDialog* dlg = new QDialog(this);
    dlg->setFixedSize(580, 480);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(10, 10, 560, 460);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Header band
    QFrame* headerBand = new QFrame();
    headerBand->setFixedHeight(80);
    headerBand->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #EA580C, stop:1 #FB923C);
            border-radius: 24px 24px 0 0;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(28, 0, 20, 0);

    QLabel* titleLbl = new QLabel("⚡  Affectation intelligente");
    titleLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
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
    makeDialogMovable(dlg, headerBand);

    // Body
    QWidget* body = new QWidget();
    body->setStyleSheet("background: transparent;");
    QVBoxLayout* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(28, 20, 28, 0);
    bodyLay->setSpacing(16);

    QLabel* subtitle = new QLabel(
        "Sélectionnez un bateau — le système choisira automatiquement le quai\n"
        "le plus adapté en fonction de la longueur et de la disponibilité.");
    subtitle->setFont(QFont("Segoe UI", 10));
    subtitle->setStyleSheet("color: #6b7280; background: transparent;");
    subtitle->setWordWrap(true);
    bodyLay->addWidget(subtitle);

    QLabel* selectLbl = new QLabel("Bateau à affecter");
    selectLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    selectLbl->setStyleSheet("color: #374151; background: transparent;");
    bodyLay->addWidget(selectLbl);

    QComboBox* boatCombo = new QComboBox();
    boatCombo->setFixedHeight(44);
    boatCombo->setFont(QFont("Segoe UI", 10));
    boatCombo->setStyleSheet(R"(
        QComboBox {
            background: #F9FAFB; border: 2px solid #E5E7EB;
            border-radius: 12px; padding: 4px 14px;
            color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
        }
        QComboBox:focus { border: 2px solid #EA580C; background: white; }
        QComboBox::drop-down { border: none; width: 32px; }
        QComboBox QAbstractItemView {
            background: white; border: 1px solid #e2e8f0;
            border-radius: 12px; selection-background-color: #FFF7ED;
            selection-color: #C2410C; padding: 6px;
        }
    )");
    for (const QVariantMap& bateau : bateaux)
        boatCombo->addItem(boatDisplayLabel(bateau), bateau);
    bodyLay->addWidget(boatCombo);

    // Info card
    QFrame* infoCard = new QFrame();
    infoCard->setStyleSheet(R"(
        QFrame {
            background: #FFF7ED;
            border: 1.5px solid #FED7AA;
            border-radius: 14px;
        }
    )");
    QGridLayout* infoGrid = new QGridLayout(infoCard);
    infoGrid->setContentsMargins(18, 14, 18, 14);
    infoGrid->setHorizontalSpacing(24);
    infoGrid->setVerticalSpacing(10);

    auto makeInfoRow = [&](int row, const QString& iconText, const QString& labelText) -> QLabel* {
        QLabel* icon = new QLabel(iconText);
        icon->setFont(QFont("Segoe UI", 14));
        icon->setStyleSheet("background: transparent;");
        icon->setFixedSize(30, 30);
        icon->setAlignment(Qt::AlignCenter);

        QLabel* lbl = new QLabel(labelText);
        lbl->setFont(QFont("Segoe UI", 9));
        lbl->setStyleSheet("color: #92400E; background: transparent;");

        QLabel* val = new QLabel("—");
        val->setFont(QFont("Segoe UI", 10, QFont::Bold));
        val->setStyleSheet("color: #7C2D12; background: transparent;");

        infoGrid->addWidget(icon, row, 0, Qt::AlignVCenter);
        infoGrid->addWidget(lbl,  row, 1, Qt::AlignVCenter);
        infoGrid->addWidget(val,  row, 2, Qt::AlignVCenter | Qt::AlignRight);

        return val;
    };

    QLabel* valLongueur   = makeInfoRow(0, "📏", "Longueur du bateau");
    QLabel* valCapacite   = makeInfoRow(1, "⚓", "Longueur minimale requise du quai");
    QLabel* valEtat       = makeInfoRow(2, "🚢", "État actuel");
    QLabel* valEstimation = makeInfoRow(3, "⏱", "Temps estimé de dockage");

    bodyLay->addWidget(infoCard);
    mainLay->addWidget(body, 1);

    // Buttons
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(28, 12, 28, 24);
    btnLay->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setMinimumWidth(110);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none;
                      border-radius: 12px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* assignBtn = new QPushButton("⚡  Affecter automatiquement");
    assignBtn->setFixedHeight(44);
    assignBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    assignBtn->setCursor(Qt::PointingHandCursor);
    assignBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #EA580C, stop:1 #FB923C);
                      color: white; border: none; border-radius: 12px; padding: 0 20px; }
        QPushButton:hover { background: #C2410C; }
    )");
    connect(assignBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(assignBtn);
    mainLay->addLayout(btnLay);

    // Live preview
    auto refreshPreview = [boatCombo, valLongueur, valCapacite, valEtat, valEstimation]() {
        const QVariantMap bateau    = boatCombo->currentData().toMap();
        const int         longueur  = bateau.value("longueur").toInt();
        const int         estimation = Quai::calculerTempsEstime(longueur);
        const QString     etat      = bateau.value("etat").toString();
        valLongueur->setText(QString("%1 m").arg(longueur));
        valCapacite->setText(QString("%1 m minimum").arg(longueur));
        valEtat->setText(etat);
        valEstimation->setText(QString("%1 min").arg(estimation));
    };
    refreshPreview();
    connect(boatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            dlg, [refreshPreview](int) { refreshPreview(); });

    if (dlg->exec() != QDialog::Accepted)
        return;

    const QVariantMap bateau = boatCombo->currentData().toMap();

    if (bateau.value("longueur").toInt() <= 0) {
        showWarning("Longueur invalide",
                    "Le bateau sélectionné n'a pas de longueur valide.\n"
                    "Veuillez en choisir un autre pour continuer.");
        return;
    }

    if (hasAvailableQuai && bateau.value("etat").toString() == "En mer") {
        showInfo("Bateau en mer — second choix",
                 "Les bateaux en mer sont traités en second choix.\n"
                 "Un quai libre est disponible : l'affectation priorise\n"
                 "les bateaux au port ou en maintenance.");
        return;
    }

    Quai    quaiChoisi;
    int     tempsEstime = 0;
    QString explication;
    if (!assignerQuaiAutomatiquement(bateau, quaiChoisi, tempsEstime, explication)) {
        showWarning("Aucun quai compatible", explication);
        return;
    }

    // ── Database transaction ─────────────────────────────────────────────────
    QSqlDatabase db = QSqlDatabase::database();
    const bool startedTransaction = db.transaction();

    QSqlQuery oldQuaiQuery;
    oldQuaiQuery.prepare("SELECT IDQUAI FROM BATEAUX WHERE IDBATEAU = :id");
    oldQuaiQuery.bindValue(":id", bateau.value("id").toString());

    if (!oldQuaiQuery.exec() || !oldQuaiQuery.next()) {
        if (startedTransaction) db.rollback();
        showError("Erreur de lecture",
                  "Impossible de lire le quai actuel du bateau.\n"
                  "Vérifiez la connexion à la base de données.");
        return;
    }
    const QVariant ancienIdQuai = oldQuaiQuery.value(0);

    QSqlQuery updateBoatQuery;
    updateBoatQuery.prepare(
        "UPDATE BATEAUX "
        "SET IDQUAI = (SELECT IDQUAI FROM QUAIS WHERE NUMERO = :numero) "
        "WHERE IDBATEAU = :id"
        );
    updateBoatQuery.bindValue(":numero", quaiChoisi.getNumero());
    updateBoatQuery.bindValue(":id",     bateau.value("id").toString());

    QSqlQuery updateQuaiQuery;
    updateQuaiQuery.prepare("UPDATE QUAIS SET ETAT = 'Occupé' WHERE NUMERO = :numero");
    updateQuaiQuery.bindValue(":numero", quaiChoisi.getNumero());

    const bool assignationImmediate = (quaiChoisi.getEtat() == "Disponible");
    if (!updateBoatQuery.exec() || (assignationImmediate && !updateQuaiQuery.exec())) {
        if (startedTransaction) db.rollback();
        showError("Erreur SQL",
                  updateBoatQuery.lastError().isValid()
                      ? updateBoatQuery.lastError().text()
                      : updateQuaiQuery.lastError().text());
        return;
    }

    if (ancienIdQuai.isValid() && !ancienIdQuai.isNull()) {
        QSqlQuery countQuery;
        countQuery.prepare(
            "SELECT COUNT(*) FROM BATEAUX "
            "WHERE IDQUAI = :idquai AND IDBATEAU <> :idbateau"
            );
        countQuery.bindValue(":idquai",   ancienIdQuai);
        countQuery.bindValue(":idbateau", bateau.value("id").toString());

        if (countQuery.exec() && countQuery.next() && countQuery.value(0).toInt() == 0) {
            QSqlQuery freeOldQuaiQuery;
            freeOldQuaiQuery.prepare(
                "UPDATE QUAIS SET ETAT = 'Disponible' "
                "WHERE IDQUAI = :idquai AND ETAT = 'Occupé'"
                );
            freeOldQuaiQuery.bindValue(":idquai", ancienIdQuai);
            freeOldQuaiQuery.exec();
        }
    }

    if (startedTransaction && !db.commit()) {
        db.rollback();
        showError("Erreur de transaction",
                  "La transaction d'affectation n'a pas pu être validée.\n"
                  "Aucune modification n'a été enregistrée.");
        return;
    }

    loadQuaisFromDatabase();
    populateTable(searchInput->text());
    BateauWindow::refreshAllTables();

    showSuccess("Affectation réussie",
                QString("%1\na été affecté au quai %2.\n%3")
                    .arg(boatDisplayLabel(bateau))
                    .arg(quaiChoisi.getNumero())
                    .arg(explication));
}
void QuaisWindow::onEditQuai(int row)
{
    if (row < 0 || row >= quais.size()) return;

    Quai& q = quais[row];

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
    makeDialogMovable(dlg, headerBand);

    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(30, 20, 30, 10);
    formLay->setSpacing(14);

    auto applyFieldState = [](QLineEdit* f, QLabel* h,
                              bool isEmpty, bool isValid,
                              const QString& idle, const QString& err, const QString& ok)
    {
        if (isEmpty) {
            f->setStyleSheet(R"(
                QLineEdit { background: #F9FAFB; border: 1.5px solid #E5E7EB;
                            border-radius: 10px; padding: 4px 14px; color: #1f2937;
                            font-family: 'Segoe UI'; font-size: 11pt; }
                QLineEdit:focus { border: 1.5px solid #378ADD; background: white; }
            )");
            h->setText("  " + idle);
            h->setStyleSheet("color: #9ca3af; background: transparent; margin-top: -3px; margin-bottom: 4px;");
        } else if (isValid) {
            f->setStyleSheet(R"(
                QLineEdit { background: white; border: 1.5px solid #1D9E75;
                            border-radius: 10px; padding: 4px 14px; color: #1f2937;
                            font-family: 'Segoe UI'; font-size: 11pt; }
                QLineEdit:focus { border: 1.5px solid #1D9E75; background: white; }
            )");
            h->setText("✓  " + ok);
            h->setStyleSheet("color: #0F6E56; background: transparent; margin-top: -3px; margin-bottom: 4px;");
        } else {
            f->setStyleSheet(R"(
                QLineEdit { background: #FEF2F2; border: 1.5px solid #E24B4A;
                            border-radius: 10px; padding: 4px 14px; color: #1f2937;
                            font-family: 'Segoe UI'; font-size: 11pt; }
                QLineEdit:focus { border: 1.5px solid #E24B4A; background: #FEF2F2; }
            )");
            h->setText("✕  " + err);
            h->setStyleSheet("color: #A32D2D; background: transparent; margin-top: -3px; margin-bottom: 4px;");
        }
    };

    auto makeField = [&](const QString& label, const QString& value,
                         QLabel*& hintLbl, const QString& idleHint) -> QLineEdit*
    {
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
        hintLbl->setStyleSheet("color: #9ca3af; background: transparent; margin-top: -3px; margin-bottom: 4px;");
        formLay->addWidget(hintLbl);

        return field;
    };

    QLabel* nomHint   = nullptr;
    QLabel* capHint   = nullptr;
    QLabel* tarifHint = nullptr;

    QLineEdit* nomField = makeField("NOM DU QUAI",
                                    QString("Quai %1").arg(q.getNumero()),
                                    nomHint, "");
    nomField->setReadOnly(true);
    nomHint->setText("");

    QLineEdit* capaciteField = makeField("CAPACITÉ",
                                         QString::number(q.getCapacite()),
                                         capHint, "Nombre entier supérieur à 0");
    connect(capaciteField, &QLineEdit::textChanged, dlg, [=](const QString& t) {
        bool ok; const int val = t.toInt(&ok);
        applyFieldState(capaciteField, capHint, t.isEmpty(), (ok && val > 0),
                        "Nombre entier supérieur à 0", "Doit être un entier > 0", "Capacité valide ✓");
    });

    QLineEdit* tarifField = makeField("TARIF (DT)",
                                      QString::number(q.getTarif()),
                                      tarifHint, "Nombre positif ex: 50.00");
    connect(tarifField, &QLineEdit::textChanged, dlg, [=](const QString& t) {
        bool ok; const double val = t.toDouble(&ok);
        applyFieldState(tarifField, tarifHint, t.isEmpty(), (ok && val > 0),
                        "Nombre positif ex: 50.00", "Doit être un nombre > 0", "Tarif valide ✓");
    });

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
        QComboBox::drop-down { border: none; width: 36px; }
        QComboBox QAbstractItemView { background: white; border: 1px solid #e2e8f0;
            border-radius: 10px; selection-background-color: #eff6ff;
            selection-color: #2563EB; padding: 6px; min-width: 180px; }
    )");
    formLay->addWidget(statutCombo);
    mainLay->addWidget(formArea, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(30, 20, 30, 30);
    btnLay->setSpacing(15);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(45);
    cancelBtn->setFont(QFont("Segoe UI", 11, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border-radius: 12px; font-weight: bold; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton("💾  Enregistrer");
    saveBtn->setFixedHeight(45);
    saveBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background: #2563EB; color: white; border-radius: 12px; font-weight: bold; }
        QPushButton:hover { background: #1D4ED8; }
    )");
    connect(saveBtn, &QPushButton::clicked, [=, &q]() {
        const QString capText   = capaciteField->text().trimmed();
        const QString tarifText = tarifField->text().trimmed();

        if (capText.isEmpty() || tarifText.isEmpty()) {
            QMessageBox::warning(dlg, "Erreur", "Veuillez remplir tous les champs !");
            return;
        }

        bool okCap = false;
        const int cap = capText.toInt(&okCap);
        if (!okCap || cap <= 0) {
            QMessageBox::warning(dlg, "Erreur",
                                 "La capacité doit être un nombre entier supérieur à zéro.");
            return;
        }

        bool okTarif = false;
        const double tarif = QString(tarifText).replace(",", ".").toDouble(&okTarif);
        if (!okTarif || tarif <= 0.0) {
            QMessageBox::warning(dlg, "Erreur", "Le tarif doit être un nombre positif valide.");
            return;
        }

        QSqlQuery query;
        query.prepare("UPDATE QUAIS SET CAPACITE = :cap, ETAT = :etat, TARIF_LOCATION = :tarif "
                      "WHERE NUMERO = :num");
        query.bindValue(":cap",   cap);
        query.bindValue(":etat",  statutCombo->currentText());
        query.bindValue(":tarif", tarif);
        query.bindValue(":num",   q.getNumero());

        if (query.exec()) {
            QSqlDatabase::database().commit();
            q.setEtat(statutCombo->currentText());
            if (isOccupiedState(statutCombo->currentText()))
                ensureAvailabilityTimerForQuai(q);
            else
                quaiAvailabilityDeadlines.remove(q.getNumero());
            dlg->accept();
            loadQuaisFromDatabase();
            populateTable(searchInput->text());
        } else {
            QMessageBox::critical(dlg, "Erreur SQL", query.lastError().text());
        }
    });

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(saveBtn);
    mainLay->addLayout(btnLay);

    dlg->exec();
}

void QuaisWindow::onDeleteQuai(int row)
{
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

    QVBoxLayout* bodyLay = new QVBoxLayout();
    bodyLay->setContentsMargins(30, 24, 30, 10);

    QLabel* msgLbl = new QLabel(
        QString("Êtes-vous sûr de vouloir supprimer le quai\n<b>%1 — Quai %1</b> ?")
            .arg(q.getNumero())
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
        query.prepare("DELETE FROM QUAIS WHERE NUMERO = :num");
        query.bindValue(":num", q.getNumero());

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur",
                                  "Impossible de supprimer le quai :\n" + query.lastError().text());
        } else {
            QSqlDatabase::database().commit();
            loadQuaisFromDatabase();
            populateTable(searchInput->text());
        }
    }

    dlg->deleteLater();
}

void QuaisWindow::onUpdateQuai(int id)
{
    Q_UNUSED(id);
}

void QuaisWindow::onLogout()
{
    if (QMessageBox::question(this, "Quitter", "Quitter l'application ?") == QMessageBox::Yes)
        close();
}

void QuaisWindow::onSort(int index)
{
    if (quais.isEmpty()) return;

    switch (index) {
    case 1: std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getNumero() < b.getNumero(); }); break;
    case 2: std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getNumero() > b.getNumero(); }); break;
    case 3: std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getTarif()  < b.getTarif();  }); break;
    case 4: std::sort(quais.begin(), quais.end(), [](const Quai& a, const Quai& b) { return a.getTarif()  > b.getTarif();  }); break;
    default: break;
    }
    populateTable(searchInput->text());
}

void QuaisWindow::afficherStatistiques()
{
    int    totalQuais        = quais.size();
    int    totalBerths       = 0;
    int    occupiedBerths    = 0;
    int    availableBerths   = 0;
    int    maintenanceBerths = 0;
    double totalRevenue      = 0.0;

    for (const Quai& q : quais) {
        const int cap = q.getCapacite();
        totalBerths += cap;
        if (q.getEtat() == "Occupé") {
            occupiedBerths += cap;
            totalRevenue   += q.getTarif() * cap;
        } else if (q.getEtat() == "Disponible") {
            availableBerths += cap;
        } else {
            maintenanceBerths += cap;
        }
    }

    const double occupancyRate  = (totalBerths > 0) ? (double)occupiedBerths / totalBerths * 100.0 : 0.0;
    const double averageRevenue = (totalQuais  > 0) ? totalRevenue / totalQuais : 0.0;
    const double maxQuais       = std::max(totalQuais, 1);
    QList<DockUsageMonitoringAnalysis> monitoringAnalyses;
    int alertDockCount = 0;
    for (const Quai& q : quais) {
        const DockUsageMonitoringAnalysis analysis = buildDockUsageMonitoringAnalysis(q, quaiAvailabilityDeadlines);
        if (analysis.anomalyScore >= 30.0)
            ++alertDockCount;
        monitoringAnalyses.append(analysis);
    }
    std::sort(monitoringAnalyses.begin(), monitoringAnalyses.end(), [](const DockUsageMonitoringAnalysis& a, const DockUsageMonitoringAnalysis& b) {
        return a.anomalyScore > b.anomalyScore;
    });

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Statistiques des Quais");
    dlg->setFixedSize(920, 760);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(0, 0, 920, 760);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

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
    makeDialogMovable(dlg, headerBand);

    QLabel* subLbl = new QLabel("Vue d'ensemble de l'occupation et des revenus du port");
    subLbl->setFont(QFont("Segoe UI", 10));
    subLbl->setStyleSheet("color: #6b7280; padding: 16px 30px 0px 30px; background: transparent;");
    mainLay->addWidget(subLbl);

    QFrame* chartsFrame = new QFrame();
    chartsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* chartsLay = new QHBoxLayout(chartsFrame);
    chartsLay->setContentsMargins(20, 20, 20, 10);
    chartsLay->setSpacing(10);

    struct ChartData { double val; double max; QString label; QString unit; QColor color; };
    const QList<ChartData> charts = {
                                     { (double)totalQuais,     maxQuais,             "Total\nQuais",           "",   QColor("#2B5EA6") },
                                     { occupancyRate,          100.0,                "Taux\nd'Occupation",     "%",  QColor("#7C3AED") },
                                     { (double)occupiedBerths, (double)totalBerths,  "Emplacements\nOccupés",  "",   QColor("#EF8C2A") },
                                     { (double)availableBerths,(double)totalBerths,  "Emplacements\nLibres",   "",   QColor("#059669") },
                                     };

    QList<CircularProgress*> progressWidgets;
    for (const ChartData& cd : charts) {
        CircularProgress* cp = new CircularProgress(cd.val, cd.max, cd.label, cd.unit, cd.color);
        chartsLay->addWidget(cp, 0, Qt::AlignCenter);
        progressWidgets.append(cp);
    }
    mainLay->addWidget(chartsFrame);

    QFrame* cardsFrame = new QFrame();
    cardsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* cardsLay = new QHBoxLayout(cardsFrame);
    cardsLay->setContentsMargins(24, 4, 24, 4);
    cardsLay->setSpacing(14);

    auto makeCard = [&](const QString& icon, const QString& val, const QString& label,
                        const QString& bg, const QString& textColor)
    {
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

    cardsLay->addWidget(makeCard("⚡",
                                 QString::number(alertDockCount),
                                 "Quais a surveiller", "#FFF7ED", "#C2410C"));

    mainLay->addWidget(cardsFrame);

    QFrame* analyzerFrame = new QFrame();
    analyzerFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* analyzerLay = new QVBoxLayout(analyzerFrame);
    analyzerLay->setContentsMargins(24, 8, 24, 0);
    analyzerLay->setSpacing(10);

    QLabel* analyzerTitle = new QLabel("Module de Monitoring Intelligent des Quais");
    analyzerTitle->setFont(QFont("Segoe UI", 13, QFont::Bold));
    analyzerTitle->setStyleSheet("color: #1f2937; background: transparent;");
    analyzerLay->addWidget(analyzerTitle);

    QLabel* analyzerSubtitle = new QLabel(
        QString("Analyse continue de l'occupation pour detecter la sous-utilisation, la surcharge frequente "
                "et les occupations incoherentes. %1 quai(x) exigent une vigilance particuliere.")
            .arg(alertDockCount));
    analyzerSubtitle->setFont(QFont("Segoe UI", 9));
    analyzerSubtitle->setStyleSheet("color: #6b7280; background: transparent;");
    analyzerSubtitle->setWordWrap(true);
    analyzerLay->addWidget(analyzerSubtitle);

    QTableWidget* analyzerTable = new QTableWidget(monitoringAnalyses.size(), 6, analyzerFrame);
    analyzerTable->setHorizontalHeaderLabels({
        "Quai", "Sessions", "Occupation moy.", "Occupation max.", "Score anomalie", "Diagnostic"
    });
    analyzerTable->horizontalHeader()->setStretchLastSection(true);
    analyzerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    analyzerTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    analyzerTable->verticalHeader()->setVisible(false);
    analyzerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    analyzerTable->setSelectionMode(QAbstractItemView::NoSelection);
    analyzerTable->setFocusPolicy(Qt::NoFocus);
    analyzerTable->setAlternatingRowColors(true);
    analyzerTable->setMinimumHeight(250);
    analyzerTable->setStyleSheet(R"(
        QTableWidget {
            background: #F9FAFB;
            border: 1px solid #E5E7EB;
            border-radius: 16px;
            alternate-background-color: #F3F4F6;
            gridline-color: #E5E7EB;
            color: #1F2937;
        }
        QHeaderView::section {
            background: #EFF6FF;
            color: #1D4ED8;
            font-weight: bold;
            border: none;
            border-bottom: 1px solid #DBEAFE;
            padding: 8px;
        }
    )");

    for (int row = 0; row < monitoringAnalyses.size(); ++row) {
        const DockUsageMonitoringAnalysis& analysis = monitoringAnalyses[row];
        const QColor tint = analysis.accentColor.lighter(185);

        auto makeAnalyzerItem = [&](const QString& text, const QString& tooltip = QString()) {
            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setBackground(tint);
            if (!tooltip.isEmpty())
                item->setToolTip(tooltip);
            return item;
        };

        analyzerTable->setItem(row, 0, makeAnalyzerItem(QString("Quai %1").arg(analysis.quaiNumber)));
        analyzerTable->setItem(row, 1, makeAnalyzerItem(QString::number(analysis.sessionCount)));
        analyzerTable->setItem(row, 2, makeAnalyzerItem(formatDurationLabel(analysis.averageOccupiedSeconds)));
        analyzerTable->setItem(row, 3, makeAnalyzerItem(formatDurationLabel(analysis.longestOccupiedSeconds)));
        analyzerTable->setItem(row, 4, makeAnalyzerItem(QString("%1%").arg(analysis.anomalyScore, 0, 'f', 1)));
        analyzerTable->setItem(row, 5, makeAnalyzerItem(
            QString("%1  |  %2  |  %3").arg(analysis.statusLabel, analysis.anomalySummary, analysis.recommendation),
            "Evaluation basee sur les donnees d'usage du quai."
        ));
        analyzerTable->setRowHeight(row, 40);
    }

    analyzerLay->addWidget(analyzerTable);
    mainLay->addWidget(analyzerFrame);
    mainLay->addStretch();

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

    if (quai.getEtat() == "Maintenance") {
        QMessageBox::warning(this, "Quai en maintenance",
                             "Ce quai est en maintenance et ne peut pas faire l'objet d'un contrat.");
        return;
    }

    ContractDialog dialog(quai, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString clientName = dialog.getClientName();
    const QString company    = dialog.getCompany();
    const QString duration   = dialog.getDuration();
    const QDate   startDate  = dialog.getStartDate();

    if (clientName.isEmpty() || company.isEmpty() || duration.isEmpty()) {
        QMessageBox::warning(this, "Champs requis",
                             "Veuillez remplir tous les champs obligatoires (nom, société, durée).");
        return;
    }

    const QRegularExpression nameExp("^[a-zA-ZÀ-ÿ\\s]+$");
    if (!nameExp.match(clientName).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Le nom ne doit contenir que des lettres.");
        return;
    }

    if (!ContractGenerator::generateContract(quai, clientName, company, duration, startDate, this))
        return;

    if (quai.getEtat() == "Disponible") {
        const int answer = QMessageBox::question(this, "Mettre à jour le statut",
                                                 "Souhaitez-vous marquer ce quai comme 'Occupé' ?",
                                                 QMessageBox::Yes | QMessageBox::No);
        if (answer == QMessageBox::Yes) {
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE QUAIS SET ETAT = 'Occupé', LOCATION = :loc WHERE NUMERO = :num");
            updateQuery.bindValue(":loc", company);
            updateQuery.bindValue(":num", quai.getNumero());
            if (updateQuery.exec()) {
                QSqlDatabase::database().commit();
                Quai occupiedQuai = quai;
                occupiedQuai.setEtat(QString::fromUtf8("OccupÃ©"));
                ensureAvailabilityTimerForQuai(occupiedQuai);
                loadQuaisFromDatabase();
                populateTable(searchInput->text());
            } else {
                QMessageBox::critical(this, "Erreur SQL", updateQuery.lastError().text());
            }
        }
    }
}







