#include "quaiswindow.h"
#include "arduino.h"
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
#include <QScrollArea>
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
#include <QSpinBox>
#include <QCheckBox>

#include "addquaidialog.h"
#include "aiintegration.h"
#include "Bateauwindow.h"
#include "ortools_optimizer.h"
#include "speech.h"

QList<QuaisWindow*> QuaisWindow::s_instances;

static QHash<int, QDateTime> s_lastCollisionPopupByQuaiId;
static constexpr int kCollisionPopupCooldownSeconds = 60; // 1-minute cooldown between collision alerts
static constexpr int kCollisionDamagePerImpact = 25;
static constexpr int kMaxCollisionDamage = 100;

static QString displayQuaiNameForId(int idQuai);

struct CollisionRecoveryState {
    int damageLevel = 0;
    QDateTime startedAt;
    QDateTime recoveryDeadline;

    bool isActive() const
    {
        return damageLevel > 0
               && startedAt.isValid()
               && recoveryDeadline.isValid()
               && QDateTime::currentDateTime() < recoveryDeadline;
    }
};

static QString boatDisplayLabel(const QVariantMap& bateauInfo)
{
    const QString nom = bateauInfo.value("nom").toString();
    const QString immatriculation = bateauInfo.value("immatriculation").toString();
    if (immatriculation.isEmpty())
        return nom;
    return QString("%1 (%2)").arg(nom, immatriculation);
}

static QString manualDockingDurationSettingsKey(const QString& boatId)
{
    return QString("quais/manual_docking_duration/%1").arg(boatId);
}

static void persistManualDockingDurationMinutes(const QString& boatId, int dockingMinutes)
{
    QSettings settings("PortFlow", "PortFlow");
    settings.setValue(manualDockingDurationSettingsKey(boatId), std::max(1, dockingMinutes) * 60);
}

static void clearManualDockingDuration(const QString& boatId)
{
    QSettings settings("PortFlow", "PortFlow");
    settings.remove(manualDockingDurationSettingsKey(boatId));
}

static bool isOccupiedState(const QString& etat)
{
    return etat.contains("occup", Qt::CaseInsensitive);
}

static bool isMaintenanceState(const QString& etat)
{
    return etat.contains("maint", Qt::CaseInsensitive);
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

static QString pendingDockAssignmentSettingsKey(int quaiNumber)
{
    return QString("quais/pending_dock_assignment/%1").arg(quaiNumber);
}

static QString collisionRecoverySettingsKey(int quaiNumber)
{
    return QString("quais/collision_recovery/%1").arg(quaiNumber);
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

static QVariantMap loadPersistedPendingDockAssignment(int quaiNumber)
{
    QSettings settings("PortFlow", "PortFlow");
    return settings.value(pendingDockAssignmentSettingsKey(quaiNumber)).toMap();
}

static void persistPendingDockAssignment(int quaiNumber, const QVariantMap& assignment)
{
    QSettings settings("PortFlow", "PortFlow");
    const QString key = pendingDockAssignmentSettingsKey(quaiNumber);

    if (assignment.isEmpty())
        settings.remove(key);
    else
        settings.setValue(key, assignment);
}

static CollisionRecoveryState loadPersistedCollisionRecoveryState(int quaiNumber)
{
    QSettings settings("PortFlow", "PortFlow");
    const QVariantMap map = settings.value(collisionRecoverySettingsKey(quaiNumber)).toMap();

    CollisionRecoveryState state;
    state.damageLevel = map.value("damageLevel").toInt();
    state.startedAt = map.value("startedAt").toDateTime();
    state.recoveryDeadline = map.value("recoveryDeadline").toDateTime();
    return state;
}

static void persistCollisionRecoveryState(int quaiNumber, const CollisionRecoveryState& state)
{
    QSettings settings("PortFlow", "PortFlow");
    const QString key = collisionRecoverySettingsKey(quaiNumber);

    if (state.damageLevel <= 0 || !state.startedAt.isValid() || !state.recoveryDeadline.isValid()) {
        settings.remove(key);
        return;
    }

    QVariantMap map;
    map.insert("damageLevel", state.damageLevel);
    map.insert("startedAt", state.startedAt);
    map.insert("recoveryDeadline", state.recoveryDeadline);
    settings.setValue(key, map);
}

static int collisionRecoverySecondsForDamage(int damageLevel)
{
    const int safeDamage = std::clamp(damageLevel, 0, kMaxCollisionDamage);
    if (safeDamage <= 0)
        return 0;

    const int tiers = (safeDamage + kCollisionDamagePerImpact - 1) / kCollisionDamagePerImpact;
    const int recoveryMinutes = 2 + (tiers * 3);
    return recoveryMinutes * 60;
}

static int collisionRemainingSeconds(const CollisionRecoveryState& state)
{
    if (!state.startedAt.isValid() || !state.recoveryDeadline.isValid())
        return 0;

    return static_cast<int>(std::max<qint64>(0, QDateTime::currentDateTime().secsTo(state.recoveryDeadline)));
}

static int currentCollisionWarningLevel(const CollisionRecoveryState& state)
{
    const int totalSeconds = std::max(1, static_cast<int>(state.startedAt.secsTo(state.recoveryDeadline)));
    const int remainingSeconds = collisionRemainingSeconds(state);
    if (remainingSeconds <= 0)
        return 0;

    const double remainingRatio = static_cast<double>(remainingSeconds) / totalSeconds;
    return std::clamp(static_cast<int>(std::ceil(state.damageLevel * remainingRatio)), 0, kMaxCollisionDamage);
}

static QString collisionWarningSummary(const CollisionRecoveryState& state)
{
    const int warningLevel = currentCollisionWarningLevel(state);
    const int remainingSeconds = collisionRemainingSeconds(state);
    return QString("Maintenance apres collision (%1%%) - retour estime dans %2")
        .arg(warningLevel)
        .arg(formatRemainingTime(remainingSeconds));
}

static void registerCollisionRecoveryImpact(int quaiNumber)
{
    CollisionRecoveryState state = loadPersistedCollisionRecoveryState(quaiNumber);
    const int currentLevel = state.isActive() ? currentCollisionWarningLevel(state) : 0;
    const int nextDamageLevel = std::clamp(currentLevel + kCollisionDamagePerImpact,
                                           kCollisionDamagePerImpact,
                                           kMaxCollisionDamage);

    state.damageLevel = nextDamageLevel;
    state.startedAt = QDateTime::currentDateTime();
    state.recoveryDeadline = state.startedAt.addSecs(collisionRecoverySecondsForDamage(nextDamageLevel));
    persistCollisionRecoveryState(quaiNumber, state);
}

static bool readQuaiCollisionContext(int idQuai, int* quaiNumber, QString* quaiLabel,
                                     QString* quaiState, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(
        "SELECT NUMERO, ETAT "
        "FROM QUAIS "
        "WHERE IDQUAI = :idquai");
    query.bindValue(":idquai", idQuai);

    if (!query.exec() || !query.next()) {
        if (errorMessage)
            *errorMessage = QString("Le quai selectionne est introuvable (ID %1).").arg(idQuai);
        return false;
    }

    if (quaiNumber)
        *quaiNumber = query.value(0).toInt();
    if (quaiState)
        *quaiState = query.value(1).toString();
    if (quaiLabel)
        *quaiLabel = displayQuaiNameForId(idQuai);
    return true;
}

static QVariantMap loadBoatInfoById(const QString& boatId)
{
    QVariantMap bateau;

    QSqlQuery query;
    query.prepare(
        "SELECT IDBATEAU, NOMBATEAU, IMMATRICULATION, NVL(LONGEUR, 0) AS LONGEUR, "
        "       NVL(CAPACITE, 0) AS CAPACITE, ETAT, IDQUAI "
        "FROM BATEAUX "
        "WHERE IDBATEAU = :id"
        );
    query.bindValue(":id", boatId);

    if (!query.exec() || !query.next())
        return bateau;

    bateau.insert("id",              query.value("IDBATEAU").toString());
    bateau.insert("nom",             query.value("NOMBATEAU").toString());
    bateau.insert("immatriculation", query.value("IMMATRICULATION").toString());
    bateau.insert("longueur",        query.value("LONGEUR").toInt());
    bateau.insert("capacite",        query.value("CAPACITE").toInt());
    bateau.insert("etat",            query.value("ETAT").toString());
    bateau.insert("idquai",          query.value("IDQUAI"));
    return bateau;
}

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle);

static bool showStyledActionDialog(QWidget* parent, const QString& title, const QString& message,
                                   const QString& gradientStart, const QString& gradientEnd,
                                   const QString& icon, bool hasCancel = false,
                                   const QString& confirmText = QString(),
                                   const QString& cancelText = "Annuler")
{
    QDialog* popup = new QDialog(parent);
    popup->setFixedSize(540, 320);
    popup->setModal(true);
    popup->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(popup);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(popup);
    container->setGeometry(10, 10, 520, 300);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

    QVBoxLayout* lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

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

    QPushButton* xBtn = new QPushButton("X");
    xBtn->setFixedSize(30, 30);
    xBtn->setCursor(Qt::PointingHandCursor);
    xBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 15px; font-size: 12px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.35); }
    )");
    QObject::connect(xBtn, &QPushButton::clicked, popup, &QDialog::reject);
    hdrLay->addWidget(xBtn);
    lay->addWidget(hdr);
    makeDialogMovable(popup, hdr);

    QLabel* msgLbl = new QLabel(message);
    msgLbl->setWordWrap(true);
    msgLbl->setFont(QFont("Segoe UI", 10));
    msgLbl->setStyleSheet("color: #374151; background: transparent; line-height: 1.4;");
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setContentsMargins(24, 22, 24, 6);
    lay->addWidget(msgLbl, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(20, 8, 20, 20);
    btnLay->setSpacing(10);
    btnLay->addStretch();

    if (hasCancel) {
        QPushButton* noBtn = new QPushButton(cancelText);
        noBtn->setFixedHeight(38);
        noBtn->setMinimumWidth(90);
        noBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
        noBtn->setCursor(Qt::PointingHandCursor);
        noBtn->setStyleSheet(R"(
            QPushButton { background: #F3F4F6; color: #374151; border: none;
                          border-radius: 10px; padding: 0 16px; }
            QPushButton:hover { background: #E5E7EB; }
        )");
        QObject::connect(noBtn, &QPushButton::clicked, popup, &QDialog::reject);
        btnLay->addWidget(noBtn);
    }

    QPushButton* okBtn = new QPushButton(confirmText.isEmpty() ? (hasCancel ? "Confirmer" : "Compris") : confirmText);
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
    QObject::connect(okBtn, &QPushButton::clicked, popup, &QDialog::accept);
    btnLay->addWidget(okBtn);

    lay->addLayout(btnLay);

    const bool accepted = (popup->exec() == QDialog::Accepted);
    popup->deleteLater();
    return accepted;
}

static QString showStyledChoiceDialog(QWidget* parent, const QString& title, const QString& message,
                                      const QString& btn1Text, const QString& btn2Text)
{
    QDialog* popup = new QDialog(parent);
    popup->setFixedSize(540, 320);
    popup->setModal(true);
    popup->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(popup);
    shadow->setBlurRadius(40); shadow->setOffset(0, 8); shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(popup);
    container->setGeometry(10, 10, 520, 300);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

    QVBoxLayout* lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0); lay->setSpacing(0);

    QFrame* hdr = new QFrame();
    hdr->setFixedHeight(64);
    hdr->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E3A8A, stop:1 #3B82F6);
            border-radius: 20px 20px 0 0;
        }
    )");
    QHBoxLayout* hdrLay = new QHBoxLayout(hdr);
    hdrLay->setContentsMargins(22, 0, 16, 0);

    QLabel* hdrIcon = new QLabel("⚓");
    hdrIcon->setFont(QFont("Segoe UI", 18));
    hdrIcon->setStyleSheet("background: transparent;");
    hdrLay->addWidget(hdrIcon);

    QLabel* hdrTitle = new QLabel(title);
    hdrTitle->setFont(QFont("Segoe UI", 13, QFont::Bold));
    hdrTitle->setStyleSheet("color: white; background: transparent;");
    hdrLay->addWidget(hdrTitle, 1);

    QPushButton* xBtn = new QPushButton("✕");
    xBtn->setFixedSize(30, 30); xBtn->setCursor(Qt::PointingHandCursor);
    xBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 15px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.35); }
    )");
    QObject::connect(xBtn, &QPushButton::clicked, popup, &QDialog::reject);
    hdrLay->addWidget(xBtn);
    lay->addWidget(hdr);
    makeDialogMovable(popup, hdr);

    QLabel* msgLbl = new QLabel(message);
    msgLbl->setWordWrap(true); msgLbl->setFont(QFont("Segoe UI", 11));
    msgLbl->setStyleSheet("color: #374151; background: transparent; line-height: 1.4;");
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setContentsMargins(24, 22, 24, 6);
    lay->addWidget(msgLbl, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(20, 8, 20, 20); btnLay->setSpacing(10);
    btnLay->addStretch();

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(40); cancelBtn->setMinimumWidth(90);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none;
                      border-radius: 10px; padding: 0 16px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    QObject::connect(cancelBtn, &QPushButton::clicked, popup, &QDialog::reject);
    btnLay->addWidget(cancelBtn);

    QString* choice = new QString("");

    QPushButton* b1 = new QPushButton(btn1Text);
    b1->setFixedHeight(40); b1->setMinimumWidth(120);
    b1->setFont(QFont("Segoe UI", 10, QFont::Bold));
    b1->setCursor(Qt::PointingHandCursor);
    b1->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #1E40AF, stop:1 #3B82F6);
                      color: white; border: none; border-radius: 10px; padding: 0 16px; }
        QPushButton:hover { background: #1E40AF; }
    )");
    QObject::connect(b1, &QPushButton::clicked, popup, [popup, choice, btn1Text]() { *choice = btn1Text; popup->accept(); });
    btnLay->addWidget(b1);

    QPushButton* b2 = new QPushButton(btn2Text);
    b2->setFixedHeight(40); b2->setMinimumWidth(140);
    b2->setFont(QFont("Segoe UI", 10, QFont::Bold));
    b2->setCursor(Qt::PointingHandCursor);
    b2->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #D97706, stop:1 #F59E0B);
                      color: white; border: none; border-radius: 10px; padding: 0 16px; }
        QPushButton:hover { background: #D97706; }
    )");
    QObject::connect(b2, &QPushButton::clicked, popup, [popup, choice, btn2Text]() { *choice = btn2Text; popup->accept(); });
    btnLay->addWidget(b2);

    lay->addLayout(btnLay);

    popup->exec();
    QString result = *choice;
    delete choice;
    popup->deleteLater();
    return result;
}

struct DockUsageMonitoringAnalysis {
    int quaiNumber = 0;
    int sessionCount = 0;
    qint64 totalOccupiedSeconds = 0;
    qint64 averageOccupiedSeconds = 0;
    qint64 longestOccupiedSeconds = 0;
    double utilizationScore = 0.0;
    double anomalyScore = 0.0;
    bool hasEnergyWasteRisk = false;
    bool hasLongOccupationRisk = false;
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
    const bool underUtilized = (analysis.sessionCount <= 1 && averageHours < 1.0)
                               || (analysis.sessionCount <= 2 && averageHours < 0.75);
    const bool overloadPattern = (analysis.sessionCount >= 6 && averageHours > 5.0)
                                 || longestHours > 12.0;
    const bool unjustifiedOccupation = isOccupiedState(quai.getEtat()) && longestHours > 10.0;

    analysis.hasEnergyWasteRisk = overloadPattern;
    analysis.hasLongOccupationRisk = unjustifiedOccupation;

    analysis.utilizationScore = std::clamp((analysis.sessionCount * 14.0) + (averageHours * 9.0), 0.0, 100.0);
    if (underUtilized)
        analysis.anomalyScore += 16.0;
    if (overloadPattern)
        analysis.anomalyScore += 32.0;
    if (unjustifiedOccupation)
        analysis.anomalyScore += 36.0;
    analysis.anomalyScore = std::clamp(analysis.anomalyScore, 0.0, 100.0);

    QStringList anomalies;
    if (underUtilized)
        anomalies << "Sous-utilisation";
    if (overloadPattern)
        anomalies << "Surcharge";
    if (unjustifiedOccupation)
        anomalies << "Occupation injustifiee";
    if (anomalies.isEmpty())
        anomalies << "Aucune anomalie majeure";
    analysis.anomalySummary = anomalies.join(" | ");

    if (analysis.anomalyScore >= 75.0) {
        analysis.statusLabel = "Alerte critique";
        analysis.recommendation = "Reaffecter les navires, accelerer la rotation et verifier les blocages.";
        analysis.accentColor = QColor(0xDC, 0x26, 0x26);
    } else if (analysis.anomalyScore >= 45.0) {
        analysis.statusLabel = "A surveiller";
        analysis.recommendation = "Ajuster la repartition des quais et reduire les temps morts.";
        analysis.accentColor = QColor(0xD9, 0x77, 0x06);
    } else {
        analysis.statusLabel = "Utilisation stable";
        analysis.recommendation = "Maintenir la planification actuelle et suivre l'equilibre d'utilisation.";
        analysis.accentColor = QColor(0x05, 0x96, 0x69);
    }

    DockAnalysisContext aiContext;
    aiContext.quaiNumber = quai.getNumero();
    aiContext.capacity = quai.getCapacite();
    aiContext.state = quai.getEtat();
    aiContext.sessionCount = analysis.sessionCount;
    aiContext.totalOccupiedSeconds = analysis.totalOccupiedSeconds;
    aiContext.averageOccupiedSeconds = analysis.averageOccupiedSeconds;
    aiContext.longestOccupiedSeconds = analysis.longestOccupiedSeconds;
    aiContext.hasEnergyWasteRisk = analysis.hasEnergyWasteRisk;
    aiContext.hasLongOccupationRisk = analysis.hasLongOccupationRisk;

    const DockAnalysisDecision aiDecision = DockIntelligenceService::analyzeDock(aiContext);
    if (aiDecision.hasOverride) {
        analysis.utilizationScore = aiDecision.utilizationScore;
        analysis.anomalyScore = aiDecision.anomalyScore;
        if (!aiDecision.statusLabel.isEmpty())
            analysis.statusLabel = aiDecision.statusLabel;
        if (!aiDecision.recommendation.isEmpty())
            analysis.recommendation = aiDecision.recommendation;
        if (!aiDecision.anomalySummary.isEmpty())
            analysis.anomalySummary = aiDecision.anomalySummary;

        if (analysis.anomalyScore >= 75.0)
            analysis.accentColor = QColor(0xDC, 0x26, 0x26);
        else if (analysis.anomalyScore >= 45.0)
            analysis.accentColor = QColor(0xD9, 0x77, 0x06);
        else
            analysis.accentColor = QColor(0x05, 0x96, 0x69);
    }

    const CollisionRecoveryState collisionState = loadPersistedCollisionRecoveryState(quai.getNumero());
    if (collisionState.isActive()) {
        const int warningLevel = currentCollisionWarningLevel(collisionState);
        analysis.anomalyScore = std::max(analysis.anomalyScore, static_cast<double>(warningLevel));
        analysis.hasLongOccupationRisk = true;
        analysis.statusLabel = (warningLevel >= 70)
                                   ? "Collision critique"
                                   : (warningLevel >= 40 ? "Collision en recuperation"
                                                         : "Collision sous controle");
        analysis.recommendation = "Laisser le quai au repos jusqu'a la fin de la maintenance automatique.";
        analysis.anomalySummary = collisionWarningSummary(collisionState);

        if (warningLevel >= 70)
            analysis.accentColor = QColor(0xDC, 0x26, 0x26);
        else if (warningLevel >= 40)
            analysis.accentColor = QColor(0xD9, 0x77, 0x06);
        else
            analysis.accentColor = QColor(0x05, 0x96, 0x69);
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

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle);

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
                                           .arg(quai.getNumero(), 0, 10)
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
        previewCard->setFixedHeight(0);
        previewCard->hide();
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
        formLay->setContentsMargins(24, 8, 24, 10);
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
            errLblPtr->setFixedHeight(18);
            errLblPtr->setStyleSheet("color: #DC2626; background: transparent; margin-top: -4px; margin-bottom: 2px;");
            formLay->addWidget(errLblPtr);
        };

        QLabel* nameErr  = nullptr;
        QLabel* compErr  = nullptr;
        QLabel* emailErr = nullptr;
        QLabel* phoneErr = nullptr;

        addField("Nom complet du client *", clientNameField, "ex: Jean Dupont", nameErr);
        QRegularExpression nameExp("^[a-zA-ZÀ-ÿ\\s]+$");
        connect(clientNameField, &QLineEdit::textChanged, this, [nameErr, nameExp](const QString& text) {
            if (text.isEmpty() || nameExp.match(text).hasMatch()) nameErr->clear();
            else nameErr->setText("⚠ Seules les lettres sont autorisées.");
        });

        addField("Société / Organisation *", companyField, "ex: Sea Harvest Ltd", compErr);

        addField("Email", emailField, "ex: contact@entreprise.com", emailErr);
        QRegularExpression emailExp("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
        connect(emailField, &QLineEdit::textChanged, this, [emailErr, emailExp](const QString& text) {
            if (text.isEmpty() || emailExp.match(text).hasMatch()) emailErr->clear();
            else emailErr->setText("⚠ Format d'email invalide.");
        });

        addField("Téléphone", phoneField, "ex: 216XXXXXXXX", phoneErr);
        QRegularExpression phoneExp("^\\+?\\d{8,15}$");
        connect(phoneField, &QLineEdit::textChanged, this, [phoneErr, phoneExp](const QString& text) {
            if (text.isEmpty() || phoneExp.match(text).hasMatch()) phoneErr->clear();
            else phoneErr->setText("⚠ Le numéro doit contenir entre 8 et 15 chiffres.");
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
    s_instances.append(this);
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

QuaisWindow::~QuaisWindow()
{
    s_instances.removeAll(this);
}

void QuaisWindow::refreshAll()
{
    for (QuaisWindow* win : s_instances) {
        if (!win) {
            continue;
        }
        win->loadQuaisFromDatabase();
        win->populateTable();
    }
}

static QString displayQuaiNameForId(int idQuai)
{
    QSqlQuery query;
    query.prepare(
        "SELECT ordre_affichage FROM ("
        "    SELECT IDQUAI, ROW_NUMBER() OVER (ORDER BY IDQUAI) AS ordre_affichage "
        "    FROM QUAIS"
        ") WHERE IDQUAI = :idquai");
    query.bindValue(":idquai", idQuai);

    if (query.exec() && query.next())
        return QString("Quai %1").arg(query.value(0).toInt());

    return QString("Quai");
}

static bool applyMaintenanceImpactToDatabase(int idQuai, QString* errorMessage)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage)
            *errorMessage = "La base de donnees n'est pas disponible.";
        return false;
    }

    QSqlQuery existsQuery(db);
    existsQuery.prepare("SELECT COUNT(*) FROM QUAIS WHERE IDQUAI = :idquai");
    existsQuery.bindValue(":idquai", idQuai);
    if (!existsQuery.exec() || !existsQuery.next() || existsQuery.value(0).toInt() == 0) {
        if (errorMessage)
            *errorMessage = QString("Le quai selectionne est introuvable (ID %1).").arg(idQuai);
        return false;
    }

    QSqlQuery stateQuery(db);
    stateQuery.prepare(
        "SELECT COUNT(*) FROM QUAIS "
        "WHERE IDQUAI = :idquai AND UPPER(TRIM(ETAT)) = 'MAINTENANCE'");
    stateQuery.bindValue(":idquai", idQuai);
    if (!stateQuery.exec() || !stateQuery.next()) {
        if (errorMessage)
            *errorMessage = "Impossible de verifier l'etat du quai.";
        return false;
    }
    if (stateQuery.value(0).toInt() > 0) {
        if (errorMessage)
            *errorMessage = QString("%1 est deja en maintenance.").arg(displayQuaiNameForId(idQuai));
        return false;
    }

    if (!db.transaction()) {
        if (errorMessage)
            *errorMessage = "Impossible de demarrer la transaction de maintenance.";
        return false;
    }

    QSqlQuery updateBoatsQuery(db);
    updateBoatsQuery.prepare("UPDATE BATEAUX SET IDQUAI = NULL WHERE IDQUAI = :idquai");
    updateBoatsQuery.bindValue(":idquai", idQuai);
    if (!updateBoatsQuery.exec()) {
        db.rollback();
        if (errorMessage)
            *errorMessage = "Impossible de liberer les bateaux associes a ce quai.";
        return false;
    }

    QSqlQuery updateQuaiQuery(db);
    updateQuaiQuery.prepare("UPDATE QUAIS SET ETAT = 'Maintenance' WHERE IDQUAI = :idquai");
    updateQuaiQuery.bindValue(":idquai", idQuai);
    if (!updateQuaiQuery.exec()) {
        db.rollback();
        if (errorMessage)
            *errorMessage = "Impossible de passer le quai en maintenance.";
        return false;
    }

    if (!db.commit()) {
        db.rollback();
        if (errorMessage)
            *errorMessage = "Impossible de valider la maintenance du quai.";
        return false;
    }

    return true;
}

void QuaisWindow::handleMaintenanceImpact(int idQuai)
{
    QString quaiLabel;
    QString quaiState;
    QString errorMessage;
    int quaiNumber = 0;

    if (!readQuaiCollisionContext(idQuai, &quaiNumber, &quaiLabel, &quaiState, &errorMessage)) {
        QWidget* parent = s_instances.isEmpty() ? nullptr : s_instances.first();
        showStyledActionDialog(parent,
                               "Impact detecte",
                               errorMessage,
                               "#1D4ED8", "#60A5FA", "!", false, "Compris");
        return;
    }

    const CollisionRecoveryState existingState = loadPersistedCollisionRecoveryState(quaiNumber);
    if (isMaintenanceState(quaiState) && !existingState.isActive()) {
        QWidget* parent = s_instances.isEmpty() ? nullptr : s_instances.first();
        showStyledActionDialog(parent,
                               "Impact detecte",
                               QString("%1 est deja en maintenance et reste indisponible.")
                                   .arg(quaiLabel),
                               "#1D4ED8", "#60A5FA", "!", false, "Compris");
        return;
    }

    if (!isMaintenanceState(quaiState) && !applyMaintenanceImpactToDatabase(idQuai, &errorMessage)) {
        QWidget* parent = s_instances.isEmpty() ? nullptr : s_instances.first();
        showStyledActionDialog(parent,
                               "Impact detecte",
                               errorMessage,
                               "#1D4ED8", "#60A5FA", "!", false, "Compris");
        return;
    }

    registerCollisionRecoveryImpact(quaiNumber);
    persistAvailabilityDeadline(quaiNumber, QDateTime());
    persistSessionStart(quaiNumber, QDateTime());

    for (QuaisWindow* win : s_instances) {
        if (!win)
            continue;
        const int quaiIndex = win->findQuaiIndexById(idQuai);
        if (quaiIndex >= 0) {
            quaiLabel = win->quais[quaiIndex].getNomQuai();
            win->quaiAvailabilityDeadlines.remove(win->quais[quaiIndex].getNumero());
            persistAvailabilityDeadline(win->quais[quaiIndex].getNumero(), QDateTime());
            persistSessionStart(win->quais[quaiIndex].getNumero(), QDateTime());
        }
        win->loadQuaisFromDatabase();
        win->populateTable(win->searchInput ? win->searchInput->text() : QString());
    }

    BateauWindow::refreshAllTables();

    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime lastShown = s_lastCollisionPopupByQuaiId.value(idQuai);
    if (lastShown.isValid() && lastShown.secsTo(now) < kCollisionPopupCooldownSeconds) {
        return;
    }
    s_lastCollisionPopupByQuaiId.insert(idQuai, now);

    const CollisionRecoveryState collisionState = loadPersistedCollisionRecoveryState(quaiNumber);
    QWidget* parent = s_instances.isEmpty() ? nullptr : s_instances.first();
    showStyledActionDialog(parent,
                           "Impact detecte",
                           QString("Un impact a ete detecte sur %1.\n"
                                   "Le quai est place en maintenance automatique pendant %2.\n"
                                   "Les avertissements diminueront progressivement pendant le temps de repos.")
                               .arg(quaiLabel)
                               .arg(formatRemainingTime(collisionRemainingSeconds(collisionState))),
                           "#1D4ED8", "#60A5FA", "!", false, "Compris");
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

    navLayout->addWidget(createNavButton("🏠", "Tableau de bord"));
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
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #F0F4F8;
            border: none;
        }
        QScrollBar:vertical {
            background: #E2E8F0;
            width: 12px;
            border-radius: 6px;
            margin: 12px 8px 12px 0;
        }
        QScrollBar::handle:vertical {
            background: #94A3B8;
            border-radius: 6px;
            min-height: 36px;
        }
        QScrollBar:horizontal {
            background: #E2E8F0;
            height: 12px;
            border-radius: 6px;
            margin: 0 12px 8px 12px;
        }
        QScrollBar::handle:horizontal {
            background: #94A3B8;
            border-radius: 6px;
            min-width: 36px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
            height: 0px;
        }
    )");

    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #F0F4F8;");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(25);
    layout->setContentsMargins(30, 30, 30, 30);

    layout->addWidget(createHeader());
    layout->addWidget(createToolbar());
    layout->addWidget(createTableCard());
    layout->addStretch();

    scrollArea->setWidget(content);
    return scrollArea;
}

QFrame* QuaisWindow::createHeader()
{
    QFrame* hdr = new QFrame();
    hdr->setStyleSheet("background: transparent;");
    hdr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout* lay = new QHBoxLayout(hdr);
    lay->setContentsMargins(0, 0, 12, 0);
    lay->setSpacing(12);

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

    QPushButton* addBtn = makeBtn("➕  Nouveau Quai", "#2563EB", "#1D4ED8");
    connect(addBtn, &QPushButton::clicked, this, &QuaisWindow::onAddQuai);

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

    auto makeToolBtn = [&](const QString& label, const QString& bg, const QString& hover) {
        QPushButton* btn = new QPushButton(label);
        btn->setFont(QFont("Segoe UI", 9, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(40);
        btn->setStyleSheet(QString(
            "QPushButton{ background:%1; color:white; border:none; border-radius:10px; padding:0 15px; }"
            "QPushButton:hover{ background:%2; }").arg(bg, hover));
        return btn;
    };

QPushButton* statsBtn = makeToolBtn("📊  Statistiques", "#7C3AED", "#6D28D9");
    connect(statsBtn, &QPushButton::clicked, this, &QuaisWindow::afficherStatistiques);

    QPushButton* pdfBtn = makeToolBtn("📄  Exporter PDF", "#059669", "#047857");
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
                item->setForeground(QColor(0x9C, 0xA3, 0xAF));
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

    QPushButton* smartAssignBtn = makeToolBtn("IA  Affecter un bateau", "#F59E0B", "#D97706");
    connect(smartAssignBtn, &QPushButton::clicked, this, &QuaisWindow::onAutoAssignBoat);

    
    lay->addWidget(statsBtn);
    lay->addWidget(pdfBtn);
    lay->addWidget(smartAssignBtn);



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
        QWidget {
            background-color: #F8FAFC;
            border: 1px solid #D6E4F5;
            border-radius: 18px;
        }
        QLabel {
            color: #1F2937;
            font-family: 'Segoe UI';
            background: transparent;
            border: none;
        }
    )");
    QVBoxLayout* warningsLayout = new QVBoxLayout(warningsPanel);
    warningsLayout->setContentsMargins(18, 18, 18, 18);
    warningsLayout->setSpacing(10);

    QLabel* warningsHeader = new QLabel("Avertissements de surveillance");
    warningsHeader->setFont(QFont("Segoe UI", 12, QFont::Bold));
    warningsHeader->setStyleSheet("color: #1E3A5F;");
    warningsLayout->addWidget(warningsHeader);

    QLabel* warningsSubheader = new QLabel("Alertes operationnelles generees a partir de l'analyse d'occupation des quais, via regles locales et moteur AI/API si configure.");
    warningsSubheader->setWordWrap(true);
    warningsSubheader->setFont(QFont("Segoe UI", 9));
    warningsSubheader->setStyleSheet("color: #64748B;");
    warningsLayout->addWidget(warningsSubheader);

    warningsContentLabel = new QLabel("Aucun avertissement pour le moment");
    warningsContentLabel->setWordWrap(true);
    warningsContentLabel->setFont(QFont("Segoe UI", 10));
    warningsContentLabel->setTextFormat(Qt::RichText);
    warningsContentLabel->setStyleSheet(R"(
        QLabel {
            background: white;
            border: 1px solid #E2E8F0;
            border-radius: 14px;
            color: #334155;
            padding: 14px 16px;
        }
    )");
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
    if (!query.exec("SELECT IDQUAI, NUMERO, CAPACITE, LOCATION, ETAT, TARIF_LOCATION, DUREE_LOCATION "
                    "FROM QUAIS ORDER BY IDQUAI")) {
        qDebug() << "Database query error:" << query.lastError().text();
        return;
    }

    int ordreNom = 1;
    while (query.next()) {
        const int    idQuai        = query.value("IDQUAI").toInt();
        const int    numero        = query.value("NUMERO").toInt();
        const int    capacite      = query.value("CAPACITE").toInt();
        const QString etat         = query.value("ETAT").toString();
        const double tarifLocation = query.value("TARIF_LOCATION").toDouble();
        const QString location     = query.value("LOCATION").toString();
        const QString dureeLocation = query.value("DUREE_LOCATION").toString();

        Quai q(numero, capacite, etat, tarifLocation, location, dureeLocation);
        q.setIdQuai(idQuai);
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
        QTimer::singleShot(0, this, [this, expiredQuais]() {
            for (int numero : expiredQuais)
                markQuaiAsAvailable(numero);
        });
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
        numeroItem->setForeground(QBrush(QColor(0x5D, 0x9C, 0xEC)));
        numeroItem->setFont(QFont("Segoe UI", 11, QFont::Bold));
        numeroItem->setData(Qt::UserRole, q.getIdQuai());
        const DockUsageMonitoringAnalysis monitoringAnalysis = buildDockUsageMonitoringAnalysis(q, quaiAvailabilityDeadlines);
        if (monitoringAnalysis.anomalyScore >= 75.0) {
            numeroItem->setBackground(QColor(0xFE, 0xF2, 0xF2));
            numeroItem->setToolTip(QString("Monitoring intelligent: %1")
                                       .arg(monitoringAnalysis.anomalySummary));
        } else if (monitoringAnalysis.anomalyScore >= 45.0) {
            numeroItem->setBackground(QColor(0xFF, 0xF7, 0xED));
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

        if (monitoringAnalysis.hasEnergyWasteRisk || monitoringAnalysis.hasLongOccupationRisk)
            warningEntries << QString("Quai %1 : %2").arg(q.getNumero()).arg(monitoringAnalysis.anomalySummary);
    }

    if (warningEntries.isEmpty()) {
        warningsContentLabel->setText(
            "<div style='color:#64748B;'>"
            "<b style='color:#0F172A;'>R.A.S.</b><br/>"
            "Aucun avertissement pour le moment."
            "</div>");
    } else {
        QStringList warningLines;
        for (int warningIndex = 0; warningIndex < warningEntries.size(); ++warningIndex) {
            const QString& entry = warningEntries.at(warningIndex);
            warningLines << QString(
                                "<div style='margin-bottom:8px;'>"
                                "<span style='color:#C2410C;font-weight:700;'>Alerte</span>"
                                "<span style='color:#334155;'> - %1</span>"
                                "</div>").arg(entry.toHtmlEscaped());
        }
        warningsContentLabel->setText(warningLines.join(""));
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

int QuaisWindow::findQuaiIndexById(int idQuai) const
{
    for (int i = 0; i < quais.size(); ++i) {
        if (quais[i].getIdQuai() == idQuai)
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

bool QuaisWindow::assignBoatToQuai(const QVariantMap& bateauInfo, const Quai& quai, int dockingMinutes,
                                   bool moveBoatToPort, QString& errorMessage)
{
    QSqlDatabase db = QSqlDatabase::database();
    const bool startedTransaction = db.transaction();

    QSqlQuery oldQuaiQuery;
    oldQuaiQuery.prepare("SELECT IDQUAI FROM BATEAUX WHERE IDBATEAU = :id");
    oldQuaiQuery.bindValue(":id", bateauInfo.value("id").toString());

    if (!oldQuaiQuery.exec() || !oldQuaiQuery.next()) {
        if (startedTransaction)
            db.rollback();
        errorMessage = "Impossible de lire le quai actuel du bateau.";
        return false;
    }
    const QVariant ancienIdQuai = oldQuaiQuery.value(0);

    QSqlQuery updateBoatQuery;
    updateBoatQuery.prepare(
        "UPDATE BATEAUX "
        "SET IDQUAI = (SELECT IDQUAI FROM QUAIS WHERE NUMERO = :numero), "
        "    ETAT = :etat "
        "WHERE IDBATEAU = :id"
        );
    updateBoatQuery.bindValue(":numero", quai.getNumero());
    updateBoatQuery.bindValue(":etat", moveBoatToPort ? "Au port" : bateauInfo.value("etat").toString());
    updateBoatQuery.bindValue(":id", bateauInfo.value("id").toString());

    QSqlQuery updateQuaiQuery;
    updateQuaiQuery.prepare("UPDATE QUAIS SET ETAT = 'Occupé' WHERE NUMERO = :numero");
    updateQuaiQuery.bindValue(":numero", quai.getNumero());

    if (!updateBoatQuery.exec() || !updateQuaiQuery.exec()) {
        if (startedTransaction)
            db.rollback();
        errorMessage = updateBoatQuery.lastError().isValid()
                           ? updateBoatQuery.lastError().text()
                           : updateQuaiQuery.lastError().text();
        return false;
    }

    if (ancienIdQuai.isValid() && !ancienIdQuai.isNull()) {
        QSqlQuery countQuery;
        countQuery.prepare(
            "SELECT COUNT(*) FROM BATEAUX "
            "WHERE IDQUAI = :idquai AND IDBATEAU <> :idbateau"
            );
        countQuery.bindValue(":idquai", ancienIdQuai);
        countQuery.bindValue(":idbateau", bateauInfo.value("id").toString());

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
        errorMessage = "La transaction d'affectation n'a pas pu être validée.";
        return false;
    }

    if (moveBoatToPort)
        Arduino::triggerSoundEvent(Arduino::SoundEvent::Arrival);

    const QDateTime sessionStart = QDateTime::currentDateTime();
    const QDateTime deadline = sessionStart.addSecs(std::max(1, dockingMinutes) * 60);
    quaiAvailabilityDeadlines.insert(quai.getNumero(), deadline);
    persistAvailabilityDeadline(quai.getNumero(), deadline);
    persistSessionStart(quai.getNumero(), sessionStart);
    persistPendingDockAssignment(quai.getNumero(), QVariantMap());
    persistManualDockingDurationMinutes(bateauInfo.value("id").toString(), dockingMinutes);

    loadQuaisFromDatabase();
    populateTable(searchInput->text());
    BateauWindow::refreshAllTables();
    return true;
}

void QuaisWindow::clearBoatAssociationForQuai(int numero)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE BATEAUX "
        "SET IDQUAI = NULL "
        "WHERE IDQUAI = (SELECT IDQUAI FROM QUAIS WHERE NUMERO = :numero)"
        );
    query.bindValue(":numero", numero);

    if (!query.exec())
        qDebug() << "Failed to clear boat association for quai" << numero << ":" << query.lastError().text();
}

void QuaisWindow::processPendingDockAssignment(int numero)
{
    const QVariantMap pendingAssignment = loadPersistedPendingDockAssignment(numero);
    if (pendingAssignment.isEmpty())
        return;

    const QString boatId = pendingAssignment.value("boatId").toString();
    QVariantMap bateauInfo = loadBoatInfoById(boatId);
    if (bateauInfo.isEmpty()) {
        persistPendingDockAssignment(numero, QVariantMap());
        return;
    }

    if (bateauInfo.value("etat").toString() != "En mer"
        || (bateauInfo.value("idquai").isValid() && !bateauInfo.value("idquai").isNull())) {
        persistPendingDockAssignment(numero, QVariantMap());
        return;
    }

    const int quaiIndex = findQuaiIndexByNumero(numero);
    if (quaiIndex < 0) {
        persistPendingDockAssignment(numero, QVariantMap());
        return;
    }

    const Quai& quai = quais[quaiIndex];
    const int dockingMinutes = std::max(1, pendingAssignment.value("dockingMinutes").toInt());
    const bool confirmed = showStyledActionDialog(
        this,
        "Quai maintenant disponible",
        QString("Le quai %1 est maintenant disponible pour %2.\n"
                "Voulez-vous confirmer cette affectation maintenant ?\n"
                "Temps de dockage prevu : %3 min.")
            .arg(numero)
            .arg(boatDisplayLabel(bateauInfo))
            .arg(dockingMinutes),
        "#1D4ED8", "#60A5FA", "i", true, "Oui, affecter", "Non"
        );

    if (!confirmed) {
        persistPendingDockAssignment(numero, QVariantMap());
        return;
    }

    QString errorMessage;
    if (!assignBoatToQuai(bateauInfo, quai, dockingMinutes, true, errorMessage)) {
        persistPendingDockAssignment(numero, QVariantMap());
        showStyledActionDialog(this,
                               "Affectation impossible",
                               "Le quai est libre mais l'affectation differee n'a pas pu etre finalisee.\n"
                                   + errorMessage,
                               "#991B1B", "#EF4444", "!", false, "Compris");
        return;
    }

    showStyledActionDialog(this,
                           "Affectation confirmee",
                           QString("%1 est maintenant au port et associe au quai %2.")
                               .arg(boatDisplayLabel(bateauInfo))
                               .arg(numero),
                           "#065F46", "#34D399", "+", false, "Compris");
}

void QuaisWindow::markQuaiAsAvailable(int numero, bool showNotification)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "Failed to free quai" << numero << ": database is not available.";
        return;
    }

    QString boatId;
    QString boatName;

    QSqlQuery boatQuery(db);
    boatQuery.prepare(
        "SELECT IDBATEAU, NOMBATEAU "
        "FROM BATEAUX "
        "WHERE IDQUAI = (SELECT IDQUAI FROM QUAIS WHERE NUMERO = :numero) "
        "AND ROWNUM = 1");
    boatQuery.bindValue(":numero", numero);

    if (!boatQuery.exec()) {
        qDebug() << "Failed to read boat associated with quai" << numero << ":" << boatQuery.lastError().text();
        return;
    }

    if (boatQuery.next()) {
        boatId = boatQuery.value(0).toString();
        boatName = boatQuery.value(1).toString();
    }

    if (!db.transaction()) {
        qDebug() << "Failed to start quai release transaction for" << numero << ":" << db.lastError().text();
        return;
    }

    if (!boatId.isEmpty()) {
        QSqlQuery updateBoatQuery(db);
        updateBoatQuery.prepare(
            "UPDATE BATEAUX "
            "SET IDQUAI = NULL, "
            "    ETAT = 'En mer', "
            "    FREQUENCE_SORTIES = COALESCE(FREQUENCE_SORTIES, 0) + 1 "
            "WHERE IDBATEAU = :id");
        updateBoatQuery.bindValue(":id", boatId);

        if (!updateBoatQuery.exec()) {
            qDebug() << "Failed to update departing boat for quai" << numero << ":" << updateBoatQuery.lastError().text();
            db.rollback();
            return;
        }
    }

    QSqlQuery query(db);
    query.prepare("UPDATE QUAIS SET ETAT = 'Disponible' WHERE NUMERO = :numero");
    query.bindValue(":numero", numero);

    if (!query.exec()) {
        qDebug() << "Failed to free quai" << numero << ":" << query.lastError().text();
        db.rollback();
        return;
    }

    if (!db.commit()) {
        qDebug() << "Failed to commit quai release for" << numero << ":" << db.lastError().text();
        db.rollback();
        return;
    }

    if (!boatId.isEmpty()) {
        clearManualDockingDuration(boatId);
        speakBoat(boatName);
        Arduino::triggerSoundEvent(Arduino::SoundEvent::Departure);
    }

    const QDateTime sessionStart = loadPersistedSessionStart(numero);
    const QDateTime sessionEnd = QDateTime::currentDateTime();
    appendPersistedSessionHistory(numero, sessionStart, sessionEnd);
    quaiAvailabilityDeadlines.remove(numero);
    persistAvailabilityDeadline(numero, QDateTime());
    persistSessionStart(numero, QDateTime());

    loadQuaisFromDatabase();
    populateTable(searchInput->text());
    BateauWindow::refreshAllTables();

    if (!loadPersistedPendingDockAssignment(numero).isEmpty()) {
        processPendingDockAssignment(numero);
        return;
    }

    if (showNotification) {
        showStyledActionDialog(this,
                               "Quai disponible",
                               QString("Le quai %1 est de nouveau disponible.\n"
                                       "Le bateau precedemment associe est maintenant sur 'Aucun quai'.")
                                   .arg(numero),
                               "#1D4ED8", "#60A5FA", "i", false, "Compris");
    }
}

bool QuaisWindow::markQuaiAsMaintenance(int idQuai, QString* errorMessage)
{
    QString quaiLabel;
    QString quaiState;
    int quaiNumber = 0;
    if (!readQuaiCollisionContext(idQuai, &quaiNumber, &quaiLabel, &quaiState, errorMessage))
        return false;

    const CollisionRecoveryState existingState = loadPersistedCollisionRecoveryState(quaiNumber);
    if (isMaintenanceState(quaiState) && !existingState.isActive()) {
        if (errorMessage)
            *errorMessage = QString("%1 est deja en maintenance.").arg(quaiLabel);
        return false;
    }

    if (!isMaintenanceState(quaiState) && !applyMaintenanceImpactToDatabase(idQuai, errorMessage))
        return false;

    registerCollisionRecoveryImpact(quaiNumber);
    persistAvailabilityDeadline(quaiNumber, QDateTime());
    persistSessionStart(quaiNumber, QDateTime());

    const int quaiIndex = findQuaiIndexById(idQuai);
    if (quaiIndex >= 0) {
        quaiLabel = quais[quaiIndex].getNomQuai();
        quaiAvailabilityDeadlines.remove(quais[quaiIndex].getNumero());
        persistAvailabilityDeadline(quais[quaiIndex].getNumero(), QDateTime());
        persistSessionStart(quais[quaiIndex].getNumero(), QDateTime());
    }

    loadQuaisFromDatabase();
    populateTable(searchInput ? searchInput->text() : QString());
    BateauWindow::refreshAllTables();

    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime lastShown = s_lastCollisionPopupByQuaiId.value(idQuai);
    if (lastShown.isValid() && lastShown.secsTo(now) < kCollisionPopupCooldownSeconds) {
        return true;
    }
    s_lastCollisionPopupByQuaiId.insert(idQuai, now);

    const CollisionRecoveryState collisionState = loadPersistedCollisionRecoveryState(quaiNumber);
    showStyledActionDialog(this,
                           "Impact detecte",
                           QString("Un impact a ete detecte sur %1.\n"
                                   "Le quai est place en maintenance automatique pendant %2.\n"
                                   "Les avertissements diminueront progressivement pendant le temps de repos.")
                               .arg(quaiLabel)
                               .arg(formatRemainingTime(collisionRemainingSeconds(collisionState))),
                           "#1D4ED8", "#60A5FA", "!", false, "Compris");
    return true;
}

void QuaisWindow::showAvailabilityCountdownPopup(int numero)
{
    const int quaiIndex = findQuaiIndexByNumero(numero);
    if (quaiIndex < 0)
        return;

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
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(28, 0, 18, 0);

    QLabel* title = new QLabel(QString("Quai %1").arg(numero), headerBand);
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
    bodyLay->addWidget(countdown);

    QLabel* status = new QLabel(body);
    status->setAlignment(Qt::AlignCenter);
    status->setWordWrap(true);
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

    auto refreshPopup = [this, numero, headerBand, title, subtitle, countdown, status, &dialog]() {
        loadQuaisFromDatabase();

        const int currentQuaiIndex = findQuaiIndexByNumero(numero);
        if (currentQuaiIndex < 0) {
            subtitle->setText("Ce quai n'est plus disponible dans la liste.");
            countdown->setText("Indisponible");
            status->setText("Impossible d'afficher les informations de disponibilite.");
            return;
        }

        const Quai& quai = quais[currentQuaiIndex];
        const QString quaiState = quai.getEtat();
        const bool occupied = isOccupiedState(quaiState) || quaiAvailabilityDeadlines.contains(numero);
        const bool maintenance = isMaintenanceState(quaiState);
        const CollisionRecoveryState collisionState = loadPersistedCollisionRecoveryState(numero);

        title->setText(QString("%1 - %2").arg(quai.getNomQuai(), quaiState));

        if (maintenance) {
            headerBand->setStyleSheet(R"(
                QFrame {
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #991B1B, stop:1 #EF4444);
                    border-radius: 24px 24px 0 0;
                }
            )");
            countdown->setStyleSheet(
                "QLabel { background: #FEF2F2; color: #B91C1C; border: 2px solid #FECACA; "
                "border-radius: 18px; font: 700 26px 'Consolas'; padding: 14px 10px; }");
            status->setStyleSheet(
                "color: #991B1B; font-size: 12px; background: #FFF1F2; border-radius: 12px; padding: 12px;");

            if (collisionState.isActive()) {
                subtitle->setText("Ce quai est en maintenance automatique apres collision.");
                countdown->setText(formatRemainingTime(collisionRemainingSeconds(collisionState)));
                status->setText(QString("Niveau d'alerte actuel : %1%%.\nLe quai redeviendra disponible a la fin du repos.")
                                    .arg(currentCollisionWarningLevel(collisionState)));
            } else {
                subtitle->setText("Ce quai est actuellement en maintenance.");
                countdown->setText("00:00:00");
                status->setText("Compte a rebours indisponible. Ce quai n'est pas disponible tant que la maintenance n'est pas terminee.");
            }
            return;
        }

        if (!occupied) {
            headerBand->setStyleSheet(R"(
                QFrame {
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #065F46, stop:1 #34D399);
                    border-radius: 24px 24px 0 0;
                }
            )");
            countdown->setStyleSheet(
                "QLabel { background: #ECFDF5; color: #047857; border: 2px solid #A7F3D0; "
                "border-radius: 18px; font: 700 28px 'Consolas'; padding: 14px 10px; }");
            status->setStyleSheet(
                "color: #065F46; font-size: 12px; background: #F0FDF4; border-radius: 12px; padding: 12px;");
            subtitle->setText("Ce quai est actuellement disponible.");
            countdown->setText("00:00:00");
            status->setText("Compte a rebours termine. Vous pouvez affecter un bateau a ce quai immediatement.");
            return;
        }

        headerBand->setStyleSheet(R"(
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2B5EA6, stop:1 #5D9CEC);
                border-radius: 24px 24px 0 0;
            }
        )");
        countdown->setStyleSheet(
            "QLabel { background: #EFF6FF; color: #1D4ED8; border: 2px solid #BFDBFE; "
            "border-radius: 18px; font: 700 28px 'Consolas'; padding: 14px 10px; }");
        status->setStyleSheet(
            "color: #0f766e; font-size: 12px; background: #F8FAFC; border-radius: 12px; padding: 12px;");

        ensureAvailabilityTimerForQuai(quai);
        const int remaining = remainingAvailabilitySeconds(numero);
        subtitle->setText("Ce quai est occupe. Temps restant avant retour au statut disponible :");
        countdown->setText(formatRemainingTime(remaining));

        if (remaining <= 0) {
            markQuaiAsAvailable(numero);
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

    const int idQuai = numeroItem->data(Qt::UserRole).toInt();
    const int quaiIndex = findQuaiIndexById(idQuai);
    if (quaiIndex < 0)
        return;

    Arduino::sendCommand(QString("Q%1:%2\n").arg(idQuai).arg(quais[quaiIndex].getOrdreNom()).toLatin1());
    showAvailabilityCountdownPopup(quais[quaiIndex].getNumero());
}

void QuaisWindow::refreshAvailabilityCountdowns()
{
    QList<int> expiredQuais;
    bool refreshCollisionWarnings = false;
    for (auto it = quaiAvailabilityDeadlines.constBegin(); it != quaiAvailabilityDeadlines.constEnd(); ++it) {
        if (QDateTime::currentDateTime() >= it.value())
            expiredQuais.append(it.key());
    }

    for (int numero : expiredQuais)
        markQuaiAsAvailable(numero);

    QList<int> recoveredCollisionQuais;
    for (const Quai& quai : std::as_const(quais)) {
        const CollisionRecoveryState collisionState = loadPersistedCollisionRecoveryState(quai.getNumero());
        if (collisionState.isActive() && collisionRemainingSeconds(collisionState) % 10 == 0)
            refreshCollisionWarnings = true;
        if (collisionState.damageLevel > 0 && collisionRemainingSeconds(collisionState) <= 0)
            recoveredCollisionQuais.append(quai.getNumero());
    }

    for (int numero : recoveredCollisionQuais) {
        persistCollisionRecoveryState(numero, CollisionRecoveryState{});

        const int quaiIndex = findQuaiIndexByNumero(numero);
        if (quaiIndex < 0 || !isMaintenanceState(quais[quaiIndex].getEtat()))
            continue;

        markQuaiAsAvailable(numero, false);
        showStyledActionDialog(this,
                               "Maintenance terminee",
                               QString("Le temps de repos du quai %1 est termine.\n"
                                       "Le quai redevient automatiquement disponible.")
                                   .arg(numero),
                               "#065F46", "#34D399", "+", false, "Compris");
    }

    if (refreshCollisionWarnings)
        populateTable(searchInput ? searchInput->text() : QString());
}

bool QuaisWindow::assignerQuaiAutomatiquement(const QVariantMap& bateauInfo, Quai& quaiChoisi,
                                              int& tempsEstime, QString& explication)
{
    const int longueur       = bateauInfo.value("longueur").toInt();
    const QString etatBateau = bateauInfo.value("etat").toString();
    tempsEstime = Quai::calculerTempsEstime(longueur);

    QList<DockCandidateContext> availableCandidates;
    QList<DockCandidateContext> reserveCandidates;

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];
        if (isMaintenanceState(q.getEtat()) || !q.peutAccueillirLongueur(longueur))
            continue;

        const DockUsageMonitoringAnalysis dockAnalysis = buildDockUsageMonitoringAnalysis(q, quaiAvailabilityDeadlines);

        DockCandidateContext candidate;
        candidate.quaiNumber = q.getNumero();
        candidate.capacity = q.getCapacite();
        candidate.state = q.getEtat();
        candidate.tariff = q.getTarif();
        candidate.availabilityMinutes = (q.getEtat() == "Disponible")
                                            ? 0
                                            : estimateQuaiAvailabilityMinutes(q.getNumero());
        candidate.anomalyScore = dockAnalysis.anomalyScore;
        candidate.utilizationScore = dockAnalysis.utilizationScore;

        reserveCandidates.append(candidate);
        if (q.getEtat() == "Disponible")
            availableCandidates.append(candidate);
    }

    if (!availableCandidates.isEmpty()) {
        const DockAssignmentDecision aiDecision =
            DockIntelligenceService::recommendAssignment(bateauInfo, availableCandidates);

        if (aiDecision.success) {
            const int quaiIndex = findQuaiIndexByNumero(aiDecision.selectedQuaiNumber);
            if (quaiIndex >= 0) {
                quaiChoisi = quais[quaiIndex];
                if (aiDecision.estimatedDockingMinutes > 0)
                    tempsEstime = aiDecision.estimatedDockingMinutes;

                explication = aiDecision.explanation.isEmpty()
                                  ? QString("Quai %1 retenu par le moteur intelligent d'affectation.")
                                        .arg(quaiChoisi.getNumero())
                                  : aiDecision.explanation;
                return true;
            }
        }
    }

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

    if (!reserveCandidates.isEmpty()) {
        const DockAssignmentDecision aiDecision =
            DockIntelligenceService::recommendAssignment(bateauInfo, reserveCandidates);

        if (aiDecision.success) {
            const int quaiIndex = findQuaiIndexByNumero(aiDecision.selectedQuaiNumber);
            if (quaiIndex >= 0) {
                quaiChoisi = quais[quaiIndex];
                if (aiDecision.estimatedDockingMinutes > 0)
                    tempsEstime = aiDecision.estimatedDockingMinutes;

                explication = aiDecision.explanation.isEmpty()
                                  ? QString("Quai %1 reserve par le moteur intelligent avec une disponibilite estimee dans %2 min.")
                                        .arg(quaiChoisi.getNumero())
                                        .arg(estimateQuaiAvailabilityMinutes(quaiChoisi.getNumero()))
                                  : aiDecision.explanation;
                return true;
            }
        }
    }

    int meilleurQuaiReserve  = -1;
    int meilleurDelai        = std::numeric_limits<int>::max();
    int meilleurScoreReserve = std::numeric_limits<int>::max();

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];
        if (isMaintenanceState(q.getEtat()) || !q.peutAccueillirLongueur(longueur))
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

bool QuaisWindow::assignerQuaiAvecORTools(const QVariantMap& bateauInfo, Quai& quaiChoisi,
                                          int& tempsEstime, QString& explication)
{
    ORToolsOptimizer optimizer;
    const int longueur = bateauInfo.value("longueur").toInt();
    tempsEstime = Quai::calculerTempsEstime(longueur);

    QList<Quai> assignableQuais;
    for (const Quai& quai : std::as_const(quais)) {
        if (!isMaintenanceState(quai.getEtat()) && quai.peutAccueillirLongueur(longueur))
            assignableQuais.append(quai);
    }

    if (assignableQuais.isEmpty()) {
        explication = QString("Aucun quai disponible ou reservable n'est compatible avec un bateau de %1 m.")
                          .arg(longueur);
        return false;
    }

    if (!optimizer.findOptimalQuai(bateauInfo, assignableQuais, quaiChoisi, explication)) {
        explication = "OR-Tools: " + explication;
        return false;
    }

    explication = "OR-Tools Optimization: " + explication;
    qDebug() << "[OR-Tools] Assignment:" << explication;
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
    auto showError = [&](const QString& title, const QString& msg) {
        showStyledActionDialog(this, title, msg, "#991B1B", "#EF4444", "!", false, "Compris");
    };
    auto showWarning = [&](const QString& title, const QString& msg) {
        showStyledActionDialog(this, title, msg, "#92400E", "#F59E0B", "!", false, "Compris");
    };
    auto showInfo = [&](const QString& title, const QString& msg) {
        showStyledActionDialog(this, title, msg, "#1D4ED8", "#60A5FA", "i", false, "Compris");
    };
    auto showSuccess = [&](const QString& title, const QString& msg) {
        showStyledActionDialog(this, title, msg, "#065F46", "#34D399", "+", false, "Compris");
    };

    QSqlQuery boatQuery;
    if (!boatQuery.exec(
            "SELECT IDBATEAU, NOMBATEAU, IMMATRICULATION, NVL(LONGEUR, 0) AS LONGEUR, "
            "       NVL(CAPACITE, 0) AS CAPACITE, ETAT "
            "FROM BATEAUX "
            "WHERE ETAT IN ('Au port', 'En maintenance', 'En mer') "
            "  AND (IDQUAI IS NULL OR IDQUAI = 0) "
            "ORDER BY CASE "
            "    WHEN ETAT = 'Au port'        THEN 1 "
            "    WHEN ETAT = 'En maintenance' THEN 2 "
            "    WHEN ETAT = 'En mer'         THEN 3 "
            "    ELSE 4 END, NOMBATEAU")) {
        showError("Erreur base de donnees",
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

    QDialog* dlg = new QDialog(this);
    dlg->setFixedSize(680, 640);
    dlg->setModal(true);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(dlg);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(dlg);
    container->setGeometry(10, 10, 660, 620);
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
                stop:0 #EA580C, stop:1 #FB923C);
            border-radius: 24px 24px 0 0;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(headerBand);
    headerLay->setContentsMargins(28, 0, 20, 0);

    QLabel* titleLbl = new QLabel(QString::fromUtf8("⚡  Affectation intelligente"));
    titleLbl->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("X");
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

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            background: #F3F4F6;
            width: 10px;
            border-radius: 5px;
            margin: 8px 6px 8px 0;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E1;
            border-radius: 5px;
            min-height: 28px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    QWidget* body = new QWidget();
    body->setStyleSheet("background: transparent;");
    QVBoxLayout* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(32, 24, 32, 0);
    bodyLay->setSpacing(18);

    QLabel* subtitle = new QLabel(
        "Selectionnez un bateau et le systeme choisira automatiquement le quai\n"
        "le plus adapte en fonction de la longueur, de la disponibilite et du moteur AI/API si configure.");
    subtitle->setFont(QFont("Segoe UI", 11));
    subtitle->setStyleSheet("color: #6b7280; background: transparent; line-height: 1.4;");
    subtitle->setWordWrap(true);
    bodyLay->addWidget(subtitle);

    QLabel* selectLbl = new QLabel(QString::fromUtf8("🚢  Bateau a affecter"));
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

    QFrame* infoCard = new QFrame();
    infoCard->setStyleSheet(R"(
        QFrame {
            background: #FFF7ED;
            border: none;
            border-radius: 14px;
        }
    )");
    QGridLayout* infoGrid = new QGridLayout(infoCard);
    infoGrid->setContentsMargins(22, 18, 22, 18);
    infoGrid->setHorizontalSpacing(20);
    infoGrid->setVerticalSpacing(14);

    auto makeInfoRow = [&](int row, const QString& labelText) -> QLabel* {
        QLabel* lbl = new QLabel(labelText);
        lbl->setFont(QFont("Segoe UI", 10, QFont::Medium));
        lbl->setStyleSheet("color: #92400E; background: transparent;");

        QLabel* val = new QLabel("-");
        val->setFont(QFont("Segoe UI", 10, QFont::Bold));
        val->setStyleSheet("color: #7C2D12; background: transparent;");
        val->setWordWrap(true);

        infoGrid->addWidget(lbl,  row, 0, Qt::AlignVCenter);
        infoGrid->addWidget(val,  row, 1, Qt::AlignVCenter | Qt::AlignRight);

        return val;
    };

    QLabel* valLongueur   = makeInfoRow(0, QString::fromUtf8("📏  Longueur du bateau"));
    QLabel* valCapacite   = makeInfoRow(1, QString::fromUtf8("⚓  Longueur minimale requise du quai"));
    QLabel* valEtat       = makeInfoRow(2, QString::fromUtf8("🧭  Etat actuel"));
    QLabel* valEstimation = makeInfoRow(3, QString::fromUtf8("⏱  Temps estime de dockage"));

    bodyLay->addWidget(infoCard);

    QFrame* dockingCard = new QFrame();
    dockingCard->setStyleSheet(R"(
        QFrame {
            background: #F8FAFC;
            border: none;
            border-radius: 14px;
        }
    )");
    QVBoxLayout* dockingLay = new QVBoxLayout(dockingCard);
    dockingLay->setContentsMargins(22, 18, 22, 18);
    dockingLay->setSpacing(12);

    QLabel* dockingTitle = new QLabel(QString::fromUtf8("⏱  Temps de dockage"));
    dockingTitle->setFont(QFont("Segoe UI", 10, QFont::Medium));
    dockingTitle->setStyleSheet("color: #374151; background: transparent;");
    dockingLay->addWidget(dockingTitle);

    QCheckBox* customDockingCheck = new QCheckBox("Choisir manuellement la duree de dockage");
    customDockingCheck->setFont(QFont("Segoe UI", 10));
    customDockingCheck->setStyleSheet("color: #1f2937; background: transparent;");
    customDockingCheck->setChecked(true);
    customDockingCheck->hide();
    dockingLay->addWidget(customDockingCheck);

    QSpinBox* dockingSpin = new QSpinBox();
    dockingSpin->setRange(1, 1440);
    dockingSpin->setSingleStep(1);
    dockingSpin->setSuffix(" min");
    dockingSpin->setEnabled(true);
    dockingSpin->setFixedHeight(40);
    dockingSpin->setFont(QFont("Segoe UI", 10));
    dockingSpin->setStyleSheet(R"(
        QSpinBox {
            background: white; border: 2px solid #E5E7EB;
            border-radius: 12px; padding: 4px 12px; color: #1f2937;
        }
        QSpinBox:focus { border: 2px solid #EA580C; }
    )");

    QPushButton* confirmDockingBtn = new QPushButton("Confirmer");
    confirmDockingBtn->setFixedHeight(40);
    confirmDockingBtn->setMinimumWidth(120);
    confirmDockingBtn->setCursor(Qt::PointingHandCursor);
    confirmDockingBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    confirmDockingBtn->setStyleSheet(R"(
        QPushButton {
            background: #EA580C; color: white; border: none;
            border-radius: 12px; padding: 0 18px;
        }
        QPushButton:hover { background: #C2410C; }
        QPushButton:disabled { background: #CBD5E1; color: #F8FAFC; }
    )");

    QHBoxLayout* dockingControlsLay = new QHBoxLayout();
    dockingControlsLay->setSpacing(10);
    dockingControlsLay->addWidget(dockingSpin, 1);
    dockingControlsLay->addWidget(confirmDockingBtn);
    dockingLay->addLayout(dockingControlsLay);

    QLabel* dockingStatus = new QLabel("Aucune duree manuelle confirmee. Le temps estime sera utilise.");
    dockingStatus->setWordWrap(true);
    dockingStatus->setFont(QFont("Segoe UI", 9, QFont::Medium));
    dockingStatus->setStyleSheet("color: #64748b; background: transparent;");
    dockingLay->addWidget(dockingStatus);

    QLabel* dockingHint = new QLabel("Le champ est pre-rempli avec le temps estime. Vous pouvez le modifier directement si besoin.");
    dockingHint->setWordWrap(true);
    dockingHint->setFont(QFont("Segoe UI", 10));
    dockingHint->setStyleSheet("color: #64748b; background: transparent;");
    dockingLay->addWidget(dockingHint);

    bodyLay->addWidget(dockingCard);
    bodyLay->addStretch();
    scrollArea->setWidget(body);
    mainLay->addWidget(scrollArea, 1);

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(32, 16, 32, 28);
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

    QPushButton* assignBtn = new QPushButton("Affecter automatiquement");
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

    int confirmedDockingMinutes = 0;
    bool dockingTimeConfirmed = false;

    auto markDockingAsPending = [dockingStatus, &dockingTimeConfirmed]() {
        dockingTimeConfirmed = false;
        dockingStatus->setText("Aucune duree manuelle confirmee. Le temps estime sera utilise.");
        dockingStatus->setStyleSheet("color: #64748b; background: transparent;");
    };

    auto refreshPreview = [boatCombo, valLongueur, valCapacite, valEtat, valEstimation,
                           dockingSpin, markDockingAsPending]() {
        const QVariantMap bateau = boatCombo->currentData().toMap();
        const int longueur = bateau.value("longueur").toInt();
        const int estimation = Quai::calculerTempsEstime(longueur);
        valLongueur->setText(QString("%1 m").arg(longueur));
        valCapacite->setText(QString("%1 m minimum").arg(longueur));
        valEtat->setText(bateau.value("etat").toString());
        valEstimation->setText(QString("%1 min").arg(estimation));
        if (!dockingSpin->hasFocus()) {
            dockingSpin->setValue(estimation);
            markDockingAsPending();
        }
    };

    connect(confirmDockingBtn, &QPushButton::clicked, dlg, [dockingSpin, dockingStatus,
                                                            &confirmedDockingMinutes, &dockingTimeConfirmed]() {
        confirmedDockingMinutes = dockingSpin->value();
        dockingTimeConfirmed = true;
        dockingStatus->setText(QString("Duree manuelle confirmee : %1 min.").arg(confirmedDockingMinutes));
        dockingStatus->setStyleSheet("color: #059669; background: transparent;");
    });
    connect(dockingSpin, QOverload<int>::of(&QSpinBox::valueChanged), dlg, [markDockingAsPending](int) {
        markDockingAsPending();
    });
    refreshPreview();
    connect(boatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), dlg, [refreshPreview](int) {
        refreshPreview();
    });

    if (dlg->exec() != QDialog::Accepted)
        return;

    const QVariantMap bateau = boatCombo->currentData().toMap();
    if (bateau.value("longueur").toInt() <= 0) {
        showWarning("Longueur invalide",
                    "Le bateau selectionne n'a pas de longueur valide.\nVeuillez en choisir un autre.");
        return;
    }

    Quai quaiChoisi;
    int tempsEstime = 0;
    QString explication;
    if (!assignerQuaiAvecORTools(bateau, quaiChoisi, tempsEstime, explication)) {
        showWarning("Aucun quai compatible", explication);
        return;
    }

    const int dockingMinutes = (customDockingCheck->isChecked() && dockingTimeConfirmed)
                                   ? confirmedDockingMinutes
                                   : tempsEstime;
    const bool delayedAssignment = (quaiChoisi.getEtat() != "Disponible");

    if (delayedAssignment && bateau.value("etat").toString() == "En mer") {
        const QVariantMap existingPending = loadPersistedPendingDockAssignment(quaiChoisi.getNumero());
        if (!existingPending.isEmpty() && existingPending.value("boatId").toString() != bateau.value("id").toString()) {
            showWarning("Reservation deja en attente",
                        QString("Le quai %1 possede deja une affectation differee en attente.")
                            .arg(quaiChoisi.getNumero()));
            return;
        }

        QVariantMap pendingAssignment;
        pendingAssignment.insert("boatId", bateau.value("id").toString());
        pendingAssignment.insert("boatLabel", boatDisplayLabel(bateau));
        pendingAssignment.insert("dockingMinutes", dockingMinutes);
        persistPendingDockAssignment(quaiChoisi.getNumero(), pendingAssignment);
        ensureAvailabilityTimerForQuai(quaiChoisi);

        showInfo("Affectation differee enregistree",
                 QString("%1 sera repropose pour le quai %2 lorsqu'il redeviendra disponible.\n"
                         "Temps de dockage prevu : %3 min.\n%4")
                     .arg(boatDisplayLabel(bateau))
                     .arg(quaiChoisi.getNumero())
                     .arg(dockingMinutes)
                     .arg(explication));
        return;
    }

    QString errorMessage;
    if (!assignBoatToQuai(bateau, quaiChoisi, dockingMinutes, bateau.value("etat").toString() == "En mer", errorMessage)) {
        showError("Erreur d'affectation", errorMessage);
        return;
    }

    showSuccess("Affectation reussie",
                QString("%1\na ete affecte au quai %2.\nTemps de dockage : %3 min.\n%4")
                    .arg(boatDisplayLabel(bateau))
                    .arg(quaiChoisi.getNumero())
                    .arg(dockingMinutes)
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

        QString newEtat = statutCombo->currentText();
        QString oldEtat = q.getEtat();

        // [NOUVEAU] Si le quai devient disponible alors qu'il était occupé
        if (newEtat == "Disponible" && isOccupiedState(oldEtat)) {
            QSqlQuery boatQuery;
            boatQuery.prepare("SELECT IDBATEAU, NOMBATEAU FROM BATEAUX WHERE IDQUAI = :idq");
            boatQuery.bindValue(":idq", q.getIdQuai());
            
            if (boatQuery.exec() && boatQuery.next()) {
                QString boatId = boatQuery.value(0).toString();
                QString boatName = boatQuery.value(1).toString();
                QString boatNewEtat = showStyledChoiceDialog(dlg, "Départ du bateau", 
                    QString("Le quai %1 devient disponible. Quel est le nouvel état du bateau '%2' ?").arg(q.getNumero()).arg(boatName),
                    "En mer", "En maintenance");

                if (boatNewEtat.isEmpty()) return; // Action annulée
                
                QSqlQuery updateBoatQuery;
                if (boatNewEtat == "En mer") {
                    updateBoatQuery.prepare("UPDATE BATEAUX SET IDQUAI = NULL, ETAT = :etat, FREQUENCE_SORTIES = COALESCE(FREQUENCE_SORTIES, 0) + 1 WHERE IDBATEAU = :id");
                } else {
                    updateBoatQuery.prepare("UPDATE BATEAUX SET IDQUAI = NULL, ETAT = :etat WHERE IDBATEAU = :id");
                }
                updateBoatQuery.bindValue(":etat", boatNewEtat);
                updateBoatQuery.bindValue(":id", boatId);
                
                if (!updateBoatQuery.exec()) {
                    QMessageBox::critical(dlg, "Erreur", "Impossible de mettre à jour le bateau : " + updateBoatQuery.lastError().text());
                    return;
                }
            }
        }

        QSqlQuery query;
        query.prepare("UPDATE QUAIS SET CAPACITE = :cap, ETAT = :etat, TARIF_LOCATION = :tarif "
                      "WHERE NUMERO = :num");
        query.bindValue(":cap",   cap);
        query.bindValue(":etat",  newEtat);
        query.bindValue(":tarif", tarif);
        query.bindValue(":num",   q.getNumero());

        if (query.exec()) {
            QSqlDatabase::database().commit();
            q.setEtat(newEtat);
            if (isOccupiedState(newEtat))
                ensureAvailabilityTimerForQuai(q);
            else {
                quaiAvailabilityDeadlines.remove(q.getNumero());
                persistAvailabilityDeadline(q.getNumero(), QDateTime());
                persistSessionStart(q.getNumero(), QDateTime());
            }
            dlg->accept();
            loadQuaisFromDatabase();
            populateTable(searchInput->text());
            BateauWindow::refreshAllTables(); // Rafraîchir la liste des bateaux
        } else {
            showStyledActionDialog(dlg, "Erreur SQL", query.lastError().text(), "#991B1B", "#EF4444", "!", false, "Compris");
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

    if (showStyledActionDialog(this, "Suppression Quai",
                               QString("Êtes-vous sûr de vouloir supprimer le quai <b>%1 — Quai %1</b> ?<br><br>⚠️ Cette action est irréversible.")
                               .arg(q.getNumero()),
                               "#EF4444", "#B91C1C", "🗑️", true, "Supprimer")) {
        
        // Vérifier si des bateaux sont encore affectés à ce quai
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM BATEAUX WHERE IDQUAI = :idq");
        checkQuery.bindValue(":idq", q.getIdQuai());
        
        if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
            showStyledActionDialog(this, "Suppression impossible", 
                                 "Impossible de supprimer ce quai car il y a encore des bateaux qui y sont affectés.<br>Veuillez d'abord déplacer ou supprimer ces bateaux.",
                                 "#991B1B", "#EF4444", "!", false, "Compris");
            return;
        }

        QSqlQuery query;
        query.prepare("DELETE FROM QUAIS WHERE NUMERO = :num");
        query.bindValue(":num", q.getNumero());

        if (!query.exec()) {
            showStyledActionDialog(this, "Erreur",
                                   "Impossible de supprimer le quai :<br>" + query.lastError().text(),
                                   "#991B1B", "#EF4444", "!", false, "Fermer");
        } else {
            QSqlDatabase::database().commit();
            loadQuaisFromDatabase();
            populateTable(searchInput->text());
            BateauWindow::refreshAllTables();
            showStyledActionDialog(this, "Succès", "Le quai a été supprimé avec succès.", "#10B981", "#059669", "✅", false, "OK");
        }
    }
}

void QuaisWindow::onUpdateQuai(int id)
{
    Q_UNUSED(id);
}

void QuaisWindow::onLogout()
{
    if (showStyledActionDialog(this, "Quitter", "Voulez-vous vraiment fermer la gestion des quais ?", "#1E3A5F", "#3B82F6", "🚪", true, "Oui, Quitter", "Annuler")) {
        this->close();
    }
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
    int    occupiedQuais     = 0;
    int    availableQuais    = 0;
    int    maintenanceQuais  = 0;
    double totalRevenue      = 0.0;

    for (int quaiIndex = 0; quaiIndex < quais.size(); ++quaiIndex) {
        const Quai& q = quais.at(quaiIndex);
        if (isOccupiedState(q.getEtat())) {
            ++occupiedQuais;
            totalRevenue += q.getTarif();
        } else if (q.getEtat() == "Disponible") {
            ++availableQuais;
        } else {
            ++maintenanceQuais;
        }
    }

    const double occupancyRate  = (totalQuais > 0) ? (double)occupiedQuais / totalQuais * 100.0 : 0.0;
    const double averageRevenue = (totalQuais  > 0) ? totalRevenue / totalQuais : 0.0;
    const double maxQuais       = std::max(totalQuais, 1);
    QList<DockUsageMonitoringAnalysis> monitoringAnalyses;
    int alertDockCount = 0;
    for (int quaiIndex = 0; quaiIndex < quais.size(); ++quaiIndex) {
        const Quai& q = quais.at(quaiIndex);
        const DockUsageMonitoringAnalysis analysis = buildDockUsageMonitoringAnalysis(q, quaiAvailabilityDeadlines);
        if (analysis.anomalyScore >= 45.0)
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

    // ─── Scroll Area ─────────────────────────────────────────────────────────────
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            width: 6px; background: transparent; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E1; border-radius: 3px; min-height: 40px;
        }
        QScrollBar::handle:vertical:hover { background: #94A3B8; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLay = new QVBoxLayout(scrollContent);
    scrollLay->setContentsMargins(0, 0, 0, 0);
    scrollLay->setSpacing(0);

    // ─── Subtitle ─────────────────────────────────────────────────────────────
    QLabel* subLbl = new QLabel("Vue d'ensemble de l'utilisation des quais, des scores d'analyse et de l'aide à la décision portuaire");
    subLbl->setFont(QFont("Segoe UI", 10));
    subLbl->setStyleSheet("color: #64748B; padding: 14px 32px 0px 32px; background: transparent; letter-spacing: 0.1px;");
    scrollLay->addWidget(subLbl);

    // ─── Circular Progress Charts ──────────────────────────────────────────────
    QFrame* chartsFrame = new QFrame();
    chartsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* chartsLay = new QHBoxLayout(chartsFrame);
    chartsLay->setContentsMargins(28, 22, 28, 10);
    chartsLay->setSpacing(16);

    struct ChartData {
        double val;
        double max;
        QString label;
        QString unit;
        QColor color;
    };
    const QList<ChartData> charts = {
                                     { (double)totalQuais,      maxQuais,            "Total\nQuais",          "",  QColor(0x25, 0x63, 0xEB) },
                                     { occupancyRate,           100.0,               "Taux\nd'Occupation",    "%", QColor(0x7C, 0x3A, 0xED) },
                                     { (double)occupiedQuais,   maxQuais,            "Quais\nOccupés",        "",  QColor(0xF5, 0x9E, 0x0B) },
                                     { (double)availableQuais,  maxQuais,            "Quais\nLibres",         "",  QColor(0x10, 0xB9, 0x81) },
                                     };

    QList<CircularProgress*> progressWidgets;
    for (int chartIndex = 0; chartIndex < charts.size(); ++chartIndex) {
        const ChartData& cd = charts.at(chartIndex);
        CircularProgress* cp = new CircularProgress(cd.val, cd.max, cd.label, cd.unit, cd.color);
        chartsLay->addWidget(cp, 0, Qt::AlignCenter);
        progressWidgets.append(cp);
    }
    scrollLay->addWidget(chartsFrame);

    // ─── KPI Cards ────────────────────────────────────────────────────────────
    QFrame* cardsFrame = new QFrame();
    cardsFrame->setStyleSheet("background: transparent;");
    QHBoxLayout* cardsLay = new QHBoxLayout(cardsFrame);
    cardsLay->setContentsMargins(28, 6, 28, 6);
    cardsLay->setSpacing(14);

    auto makeCard = [&](const QString& icon, const QString& val, const QString& label,
                        const QString& accentHex, const QString& bgHex) -> QFrame*
    {
        QFrame* card = new QFrame();
        card->setFixedHeight(96);
        card->setStyleSheet(QString(R"(
            QFrame {
                background: %1;
                border-radius: 18px;
                border: none;
            }
        )").arg(bgHex));

        // Subtle left accent bar
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(18, 12, 18, 12);
        cl->setSpacing(14);

        // Icon bubble
        QLabel* iconBubble = new QLabel(icon);
        iconBubble->setFont(QFont("Segoe UI Emoji", 20));
        iconBubble->setFixedSize(48, 48);
        iconBubble->setAlignment(Qt::AlignCenter);
        iconBubble->setStyleSheet(QString(R"(
            background: white;
            border-radius: 14px;
            border: none;
        )"));
        cl->addWidget(iconBubble);

        QVBoxLayout* vl = new QVBoxLayout();
        vl->setSpacing(2);

        QLabel* valLbl = new QLabel(val);
        valLbl->setFont(QFont("Segoe UI", 17, QFont::Bold));
        valLbl->setStyleSheet(QString("color: %1; background: transparent;").arg(accentHex));

        QLabel* labLbl = new QLabel(label);
        labLbl->setFont(QFont("Segoe UI", 9));
        labLbl->setStyleSheet("color: #6B7280; background: transparent; letter-spacing: 0.2px;");

        vl->addWidget(valLbl);
        vl->addWidget(labLbl);
        cl->addLayout(vl, 1);
        return card;
    };

    cardsLay->addWidget(makeCard("💰",
                                 QString("%1 DT").arg(totalRevenue, 0, 'f', 0),
                                 "Chiffre d'Affaires", "#1D4ED8", "#EFF6FF"));
    cardsLay->addWidget(makeCard("📈",
                                 QString("%1 DT").arg(averageRevenue, 0, 'f', 0),
                                 "Revenu Moyen / Quai", "#6D28D9", "#F5F3FF"));
    cardsLay->addWidget(makeCard("🔧",
                                 QString::number(maintenanceQuais),
                                 "En Maintenance", "#B91C1C", "#FEF2F2"));
    cardsLay->addWidget(makeCard("⚡",
                                 QString::number(alertDockCount),
                                 "Quais à surveiller", "#C2410C", "#FFF7ED"));

    scrollLay->addWidget(cardsFrame);

    // ─── Analyzer Module ──────────────────────────────────────────────────────
    QFrame* analyzerFrame = new QFrame();
    analyzerFrame->setStyleSheet(R"(
        QFrame {
            background: #FFFFFF;
            border: none;
            border-radius: 22px;
        }
    )");

    // Drop shadow
    QGraphicsDropShadowEffect* analyzerShadow = new QGraphicsDropShadowEffect(analyzerFrame);
    analyzerShadow->setBlurRadius(24);
    analyzerShadow->setOffset(0, 4);
    analyzerShadow->setColor(QColor(15, 23, 42, 28));
    analyzerFrame->setGraphicsEffect(analyzerShadow);

    QVBoxLayout* analyzerLay = new QVBoxLayout(analyzerFrame);
    analyzerLay->setContentsMargins(28, 26, 28, 26);
    analyzerLay->setSpacing(10);

    // Header row
    QHBoxLayout* analyzerHeaderRow = new QHBoxLayout();
    analyzerHeaderRow->setSpacing(12);

    QFrame* analyzerIconBadge = new QFrame();
    analyzerIconBadge->setFixedSize(44, 44);
    analyzerIconBadge->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1E3A5F, stop:1 #2563EB);
            border-radius: 13px;
        }
    )");
    QLabel* analyzerIconLbl = new QLabel("⚙", analyzerIconBadge);
    analyzerIconLbl->setFont(QFont("Segoe UI Emoji", 18));
    analyzerIconLbl->setAlignment(Qt::AlignCenter);
    analyzerIconLbl->setGeometry(0, 0, 44, 44);
    analyzerIconLbl->setStyleSheet("background: transparent; color: white;");

    QVBoxLayout* analyzerTitleCol = new QVBoxLayout();
    analyzerTitleCol->setSpacing(2);

    QLabel* analyzerTitle = new QLabel("Module d'Analyse Continue de l'Utilisation des Quais");
    analyzerTitle->setFont(QFont("Segoe UI", 13, QFont::Bold));
    analyzerTitle->setStyleSheet("color: #0F172A; background: transparent;");

    QLabel* analyzerSubtitle = new QLabel(
        QString("Analyse en continu chaque quai : sessions, occupation, anomalies et recommandations. "
                "<b style='color:#DC2626;'>%1 quai(s)</b> requièrent une vigilance particulière.")
            .arg(alertDockCount));
    analyzerSubtitle->setFont(QFont("Segoe UI", 9));
    analyzerSubtitle->setStyleSheet("color: #64748B; background: transparent;");
    analyzerSubtitle->setWordWrap(true);

    analyzerTitleCol->addWidget(analyzerTitle);
    analyzerTitleCol->addWidget(analyzerSubtitle);
    analyzerHeaderRow->addWidget(analyzerIconBadge, 0, Qt::AlignTop);
    analyzerHeaderRow->addLayout(analyzerTitleCol, 1);
    analyzerLay->addLayout(analyzerHeaderRow);

    // Hint pill
    QFrame* hintPill = new QFrame();
    hintPill->setStyleSheet(R"(
        QFrame {
            background: #EFF6FF;
            border-radius: 10px;
            border: none;
        }
    )");
    QHBoxLayout* hintPillLay = new QHBoxLayout(hintPill);
    hintPillLay->setContentsMargins(14, 8, 14, 8);
    hintPillLay->setSpacing(8);

    QLabel* hintIcon = new QLabel("💡");
    hintIcon->setFont(QFont("Segoe UI Emoji", 11));
    hintIcon->setStyleSheet("background: transparent;");

    QLabel* analyzerHint = new QLabel("Cliquez sur une cellule du diagnostic pour afficher l'analyse complète dans une fiche détaillée.");
    analyzerHint->setFont(QFont("Segoe UI", 9, QFont::Medium));
    analyzerHint->setStyleSheet("color: #1D4ED8; background: transparent;");
    analyzerHint->setWordWrap(true);

    hintPillLay->addWidget(hintIcon);
    hintPillLay->addWidget(analyzerHint, 1);
    analyzerLay->addWidget(hintPill);

    // ─── Analyzer Table ───────────────────────────────────────────────────────
    QTableWidget* analyzerTable = new QTableWidget(monitoringAnalyses.size(), 8, analyzerFrame);
    analyzerTable->setHorizontalHeaderLabels({
        "Quai", "Sessions", "Temps total", "Durée moy.", "Durée max.", "Score utilisation", "Score anomalie", "Diagnostic"
    });
    analyzerTable->horizontalHeader()->setStretchLastSection(true);
    analyzerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    analyzerTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    analyzerTable->verticalHeader()->setVisible(false);
    analyzerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    analyzerTable->setSelectionMode(QAbstractItemView::NoSelection);
    analyzerTable->setFocusPolicy(Qt::NoFocus);
    analyzerTable->setAlternatingRowColors(true);
    analyzerTable->setWordWrap(true);
    analyzerTable->setMinimumHeight(290);
    analyzerTable->setShowGrid(false);
    analyzerTable->setStyleSheet(R"(
        QTableWidget {
            background: #FAFCFF;
            border: 1px solid #E8EDF5;
            border-radius: 16px;
            alternate-background-color: #F8FAFC;
            color: #1E293B;
            outline: none;
        }
        QTableWidget::item {
            padding: 11px 14px;
            border-bottom: 1px solid #F1F5F9;
        }
        QTableWidget::item:hover {
            background: #EFF6FF;
        }
        QHeaderView::section {
            background: #F1F5F9;
            color: #475569;
            font-weight: 700;
            font-size: 9pt;
            letter-spacing: 0.5px;
            text-transform: uppercase;
            border: none;
            border-bottom: 2px solid #E2E8F0;
            padding: 10px 14px;
        }
        QHeaderView::section:first { border-top-left-radius: 16px; }
        QHeaderView::section:last  { border-top-right-radius: 16px; }
        QScrollBar:vertical {
            width: 6px; background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E1; border-radius: 3px; min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");

    for (int row = 0; row < monitoringAnalyses.size(); ++row) {
        const DockUsageMonitoringAnalysis& analysis = monitoringAnalyses[row];
        const QColor tint = analysis.accentColor.lighter(192);

        auto makeAnalyzerItem = [&](const QString& text, const QString& tooltip = QString()) {
            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setBackground(tint);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            item->setForeground(QColor(0x1E, 0x29, 0x3B));
            if (!tooltip.isEmpty())
                item->setToolTip(tooltip);
            return item;
        };

        analyzerTable->setItem(row, 0, makeAnalyzerItem(QString("Quai %1").arg(analysis.quaiNumber)));
        analyzerTable->setItem(row, 1, makeAnalyzerItem(QString::number(analysis.sessionCount)));
        analyzerTable->setItem(row, 2, makeAnalyzerItem(formatDurationLabel(analysis.totalOccupiedSeconds)));
        analyzerTable->setItem(row, 3, makeAnalyzerItem(formatDurationLabel(analysis.averageOccupiedSeconds)));
        analyzerTable->setItem(row, 4, makeAnalyzerItem(formatDurationLabel(analysis.longestOccupiedSeconds)));
        analyzerTable->setItem(row, 5, makeAnalyzerItem(QString("%1%").arg(analysis.utilizationScore, 0, 'f', 1)));
        analyzerTable->setItem(row, 6, makeAnalyzerItem(QString("%1%").arg(analysis.anomalyScore, 0, 'f', 1)));

        const QString fullDiagnostic = QString("%1  |  %2  |  %3")
                                           .arg(analysis.statusLabel, analysis.anomalySummary, analysis.recommendation);
        QString previewDiagnostic = fullDiagnostic;
        if (previewDiagnostic.size() > 82)
            previewDiagnostic = previewDiagnostic.left(82).trimmed() + "…";

        QTableWidgetItem* diagnosticItem = makeAnalyzerItem(
            previewDiagnostic,
            "Cliquez pour afficher l'analyse complète de ce quai."
            );
        diagnosticItem->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        diagnosticItem->setData(Qt::UserRole,     fullDiagnostic);
        diagnosticItem->setData(Qt::UserRole + 1, analysis.statusLabel);
        diagnosticItem->setData(Qt::UserRole + 2, analysis.anomalySummary);
        diagnosticItem->setData(Qt::UserRole + 3, analysis.recommendation);
        diagnosticItem->setData(Qt::UserRole + 4, analysis.accentColor);
        diagnosticItem->setSizeHint(QSize(0, 64));
        analyzerTable->setItem(row, 7, diagnosticItem);
        analyzerTable->setRowHeight(row, 64);
    }

    analyzerTable->resizeRowsToContents();

    // ─── Diagnostic Detail Dialog (on cell click) ─────────────────────────────
    connect(analyzerTable, &QTableWidget::cellClicked, dlg, [analyzerTable, dlg](int row, int column) {
        if (column != 7) return;

        QTableWidgetItem* item = analyzerTable->item(row, column);
        if (!item) return;

        const QString statusLabel    = item->data(Qt::UserRole + 1).toString();
        const QString anomalySummary = item->data(Qt::UserRole + 2).toString();
        const QString recommendation = item->data(Qt::UserRole + 3).toString();
        const QColor  accentColor    = item->data(Qt::UserRole + 4).value<QColor>();
        const QString fullDiagnostic = item->data(Qt::UserRole).toString();
        const QString quaiLabel      = analyzerTable->item(row, 0) ? analyzerTable->item(row, 0)->text() : "Quai";
        const QString accentHex      = accentColor.isValid() ? accentColor.name() : "#8B5CF6";

        QDialog detailsDialog(dlg);
        detailsDialog.setModal(true);
        detailsDialog.setFixedSize(720, 560);
        detailsDialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        detailsDialog.setAttribute(Qt::WA_TranslucentBackground);

        QGraphicsDropShadowEffect* detailShadow = new QGraphicsDropShadowEffect(&detailsDialog);
        detailShadow->setBlurRadius(52);
        detailShadow->setOffset(0, 12);
        detailShadow->setColor(QColor(15, 23, 42, 80));

        QWidget* detailContainer = new QWidget(&detailsDialog);
        detailContainer->setGeometry(14, 14, 692, 532);
        detailContainer->setGraphicsEffect(detailShadow);
        detailContainer->setStyleSheet("QWidget { background: #FFFFFF; border-radius: 24px; }");

        QVBoxLayout* detailLay = new QVBoxLayout(detailContainer);
        detailLay->setContentsMargins(0, 0, 0, 0);
        detailLay->setSpacing(0);

        // Header
        QFrame* detailHeader = new QFrame();
        detailHeader->setFixedHeight(92);
        detailHeader->setStyleSheet(R"(
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #7C3AED, stop:0.55 #A78BFA, stop:1 #38BDF8);
                border-radius: 24px 24px 0 0;
            }
        )");

        QHBoxLayout* detailHeaderLay = new QHBoxLayout(detailHeader);
        detailHeaderLay->setContentsMargins(28, 0, 20, 0);
        detailHeaderLay->setSpacing(14);

        // Small quai icon badge in header
        QFrame* headerBadge = new QFrame();
        headerBadge->setFixedSize(42, 42);
        headerBadge->setStyleSheet("QFrame { background: rgba(255,255,255,0.22); border-radius: 13px; }");
        QLabel* headerBadgeLbl = new QLabel("🛳", headerBadge);
        headerBadgeLbl->setFont(QFont("Segoe UI Emoji", 18));
        headerBadgeLbl->setGeometry(0, 0, 42, 42);
        headerBadgeLbl->setAlignment(Qt::AlignCenter);
        headerBadgeLbl->setStyleSheet("background: transparent;");
        detailHeaderLay->addWidget(headerBadge);

        QVBoxLayout* detailTitleLay = new QVBoxLayout();
        detailTitleLay->setSpacing(3);

        QLabel* detailTitle = new QLabel(QString("Diagnostic — %1").arg(quaiLabel));
        detailTitle->setFont(QFont("Segoe UI", 14, QFont::Bold));
        detailTitle->setStyleSheet("color: white; background: transparent;");

        QLabel* detailSubtitle = new QLabel("Statut, anomalies détectées et recommandation opérationnelle");
        detailSubtitle->setFont(QFont("Segoe UI", 9));
        detailSubtitle->setStyleSheet("color: rgba(255,255,255,0.70); background: transparent;");

        detailTitleLay->addWidget(detailTitle);
        detailTitleLay->addWidget(detailSubtitle);
        detailHeaderLay->addLayout(detailTitleLay, 1);

        QPushButton* detailCloseBtn = new QPushButton("✕");
        detailCloseBtn->setFixedSize(36, 36);
        detailCloseBtn->setCursor(Qt::PointingHandCursor);
        detailCloseBtn->setStyleSheet(R"(
            QPushButton {
                background: rgba(255,255,255,0.14);
                color: white; border: none;
                border-radius: 18px;
                font-size: 12px; font-weight: bold;
            }
            QPushButton:hover { background: rgba(255,255,255,0.28); }
        )");
        connect(detailCloseBtn, &QPushButton::clicked, &detailsDialog, &QDialog::accept);
        detailHeaderLay->addWidget(detailCloseBtn);
        detailLay->addWidget(detailHeader);
        makeDialogMovable(&detailsDialog, detailHeader);

        QScrollArea* detailScroll = new QScrollArea();
        detailScroll->setWidgetResizable(true);
        detailScroll->setFrameShape(QFrame::NoFrame);
        detailScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        detailScroll->setStyleSheet(R"(
            QScrollArea { background: transparent; border: none; }
            QScrollBar:vertical {
                background: #F1F5F9;
                width: 8px;
                border-radius: 4px;
                margin: 10px 8px 10px 0;
            }
            QScrollBar::handle:vertical {
                background: #CBD5E1;
                border-radius: 4px;
                min-height: 30px;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        )");

        // Body
        QWidget* detailBody = new QWidget();
        detailBody->setStyleSheet("background: transparent;");
        QVBoxLayout* detailBodyLay = new QVBoxLayout(detailBody);
        detailBodyLay->setContentsMargins(26, 22, 26, 8);
        detailBodyLay->setSpacing(14);

        // Status card
        QFrame* statusCard = new QFrame();
        const QString lightAccent = accentColor.isValid() ? accentColor.lighter(190).name() : "#F5F3FF";
        statusCard->setStyleSheet(QString(R"(
            QFrame {
                background: %1;
                border: none;
                border-radius: 16px;
            }
        )").arg(lightAccent, accentColor.isValid() ? accentColor.lighter(160).name() : "#DDD6FE"));

        QHBoxLayout* statusLay = new QHBoxLayout(statusCard);
        statusLay->setContentsMargins(18, 14, 18, 14);
        statusLay->setSpacing(12);

        QFrame* statusDot = new QFrame();
        statusDot->setFixedSize(10, 10);
        statusDot->setStyleSheet(QString("QFrame { background: %1; border-radius: 5px; }").arg(accentHex));

        QVBoxLayout* statusTextLay = new QVBoxLayout();
        statusTextLay->setSpacing(4);

        QLabel* statusTitle = new QLabel(statusLabel);
        statusTitle->setFont(QFont("Segoe UI", 11, QFont::Bold));
        statusTitle->setStyleSheet(QString("color: %1; background: transparent;").arg(accentHex));

        QLabel* summaryLabel = new QLabel(QString("Anomalies détectées : %1").arg(anomalySummary));
        summaryLabel->setFont(QFont("Segoe UI", 9, QFont::Medium));
        summaryLabel->setStyleSheet("color: #334155; background: transparent;");
        summaryLabel->setWordWrap(true);

        statusTextLay->addWidget(statusTitle);
        statusTextLay->addWidget(summaryLabel);
        statusLay->addWidget(statusDot, 0, Qt::AlignTop | Qt::AlignVCenter);
        statusLay->addLayout(statusTextLay, 1);
        detailBodyLay->addWidget(statusCard);

        // Full analysis card
        QFrame* textCard = new QFrame();
        textCard->setStyleSheet(R"(
            QFrame {
                background: #FCFCFF;
                border: none;
                border-radius: 16px;
            }
        )");
        QVBoxLayout* textLay = new QVBoxLayout(textCard);
        textLay->setContentsMargins(20, 18, 20, 18);
        textLay->setSpacing(10);

        QLabel* fullTitle = new QLabel("Analyse complète");
        fullTitle->setFont(QFont("Segoe UI", 10, QFont::Bold));
        fullTitle->setStyleSheet("color: #0F172A; background: transparent;");

        QLabel* fullText = new QLabel(fullDiagnostic);
        fullText->setWordWrap(true);
        fullText->setFont(QFont("Segoe UI", 9));
        fullText->setStyleSheet("color: #475569; background: transparent;");
        fullText->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        QLabel* recommendationTitle = new QLabel("Recommandation");
        recommendationTitle->setFont(QFont("Segoe UI", 10, QFont::Bold));
        recommendationTitle->setStyleSheet("color: #0F172A; background: transparent;");

        QLabel* recommendationLabel = new QLabel(recommendation);
        recommendationLabel->setWordWrap(true);
        recommendationLabel->setFont(QFont("Segoe UI", 9));
        recommendationLabel->setStyleSheet("color: #334155; background: transparent;");

        textLay->addWidget(fullTitle);
        textLay->addWidget(fullText);
        textLay->addWidget(recommendationTitle);
        textLay->addWidget(recommendationLabel);

        detailBodyLay->addWidget(textCard, 1);
        detailBodyLay->addStretch();
        detailScroll->setWidget(detailBody);
        detailLay->addWidget(detailScroll, 1);

        // Footer
        QHBoxLayout* detailBottomLay = new QHBoxLayout();
        detailBottomLay->setContentsMargins(26, 10, 26, 22);
        detailBottomLay->addStretch();

        QPushButton* detailOkBtn = new QPushButton("  Fermer");
        detailOkBtn->setFixedHeight(42);
        detailOkBtn->setMinimumWidth(130);
        detailOkBtn->setCursor(Qt::PointingHandCursor);
        detailOkBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
        detailOkBtn->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #7C3AED, stop:1 #38BDF8);
                color: white; border: none;
                border-radius: 12px; padding: 0 20px;
            }
            QPushButton:hover { background: #8B5CF6; }
        )");
        connect(detailOkBtn, &QPushButton::clicked, &detailsDialog, &QDialog::accept);
        detailBottomLay->addWidget(detailOkBtn);
        detailLay->addLayout(detailBottomLay);

        detailsDialog.exec();
    });

    analyzerLay->addWidget(analyzerTable);

    // Wrap analyzerFrame with side margins
    QHBoxLayout* analyzerWrapLay = new QHBoxLayout();
    analyzerWrapLay->setContentsMargins(28, 6, 28, 16);
    analyzerWrapLay->addWidget(analyzerFrame);
    scrollLay->addLayout(analyzerWrapLay);
    scrollLay->addStretch();

    scrollArea->setWidget(scrollContent);
    mainLay->addWidget(scrollArea, 1);

    // ─── Footer Close Button ──────────────────────────────────────────────────
    QHBoxLayout* bottomLay = new QHBoxLayout();
    bottomLay->setContentsMargins(28, 4, 28, 22);

    QPushButton* okBtn = new QPushButton("  Fermer");
    okBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    okBtn->setFixedHeight(44);
    okBtn->setFixedWidth(150);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #0F172A, stop:1 #2563EB);
            color: white; border: none;
            border-radius: 13px; padding: 0 20px;
            letter-spacing: 0.3px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #1E3A5F, stop:1 #3B82F6);
        }
        QPushButton:pressed { background: #0F172A; }
    )");
    connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    bottomLay->addStretch();
    bottomLay->addWidget(okBtn);
    mainLay->addLayout(bottomLay);

    // ─── Staggered animation on progress widgets ──────────────────────────────
    for (int i = 0; i < progressWidgets.size(); ++i) {
        CircularProgress* cp = progressWidgets[i];
        QTimer::singleShot(i * 140, cp, [cp]() { cp->animateTo(); });
    }

    dlg->exec();
}

// ─────────────────────────────────────────────────────────────────────────────
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
                occupiedQuai.setEtat(QString::fromUtf8("Occupé"));
                ensureAvailabilityTimerForQuai(occupiedQuai);
                loadQuaisFromDatabase();
                populateTable(searchInput->text());
            } else {
                QMessageBox::critical(this, "Erreur SQL", updateQuery.lastError().text());
            }
        }
    }
}




