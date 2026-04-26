#include "quaimanager.h"

#include "Bateauwindow.h"
#include "arduino.h"
#include "quaiswindow.h"
#include "speech.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QVariant>

namespace {
QString normalizeState(QString state)
{
    state = state.trimmed().toLower();
    return state;
}

QString manualDockingDurationSettingsKey(const QString& boatId)
{
    return QString("quais/manual_docking_duration/%1").arg(boatId);
}

int loadManualDockingDurationSeconds(const QString& boatId)
{
    QSettings settings("PortFlow", "PortFlow");
    return settings.value(manualDockingDurationSettingsKey(boatId), 0).toInt();
}

void clearManualDockingDuration(const QString& boatId)
{
    QSettings settings("PortFlow", "PortFlow");
    settings.remove(manualDockingDurationSettingsKey(boatId));
}
}

QuaiManager::QuaiManager(QObject* parent)
    : QObject(parent)
{
    pollingTimer.setInterval(1000);
    connect(&pollingTimer, &QTimer::timeout, this, &QuaiManager::monitorBoats);
}

void QuaiManager::start()
{
    refreshFromDatabase();
    pollingTimer.start();
}

void QuaiManager::stop()
{
    pollingTimer.stop();
}

void QuaiManager::refreshFromDatabase()
{
    synchronizeTrackedBoats();
}

void QuaiManager::monitorBoats()
{
    synchronizeTrackedBoats();

    const QDateTime now = QDateTime::currentDateTime();
    QList<QString> overdueBoatIds;

    for (auto it = trackedBoats.begin(); it != trackedBoats.end(); ++it) {
        const TrackedBoatEntry& entry = it.value();
        if (!entry.registeredAt.isValid() || entry.allocatedDurationSeconds <= 0) {
            continue;
        }

        if (entry.registeredAt.secsTo(now) >= entry.allocatedDurationSeconds) {
            overdueBoatIds.append(it.key());
        }
    }

    for (const QString& boatId : overdueBoatIds) {
        auto it = trackedBoats.find(boatId);
        if (it == trackedBoats.end()) {
            continue;
        }

        markBoatAsAtSea(it.value());
        trackedBoats.erase(it);
    }
}

void QuaiManager::synchronizeTrackedBoats()
{
    QSqlQuery query;
    query.prepare(
        "SELECT b.IDBATEAU, b.NOMBATEAU, b.IMMATRICULATION, b.CAPACITE, b.LONGEUR, "
        "       b.AGE_BATEAU, TO_CHAR(b.DATE_DERNIERE_MAINTENANCE, 'DD/MM/YYYY') AS DATE_MAINT, "
        "       b.ID_EMPLOYE, b.IDQUAI, b.ETAT, "
        "       q.NUMERO, q.DUREE_LOCATION "
        "FROM BATEAUX b "
        "LEFT JOIN QUAIS q ON q.IDQUAI = b.IDQUAI");

    if (!query.exec()) {
        qDebug() << "QuaiManager sync failed:" << query.lastError().text();
        return;
    }

    QHash<QString, bool> seenBoatIds;

    while (query.next()) {
        const QString boatId = query.value(0).toString();
        const QString boatName = query.value(1).toString();
        const QString immatriculation = query.value(2).toString();
        const QString capacite = query.value(3).toString();
        const QString longueur = query.value(4).toString();
        const QString age = query.value(5).toString();
        const QString maintenanceDate = query.value(6).toString();
        const QString employeeId = query.value(7).toString();
        const QVariant quaiIdValue = query.value(8);
        const QString state = query.value(9).toString();
        const int quaiNumber = query.value(10).toInt();
        const QString durationText = query.value(11).toString();

        if (boatId.isEmpty()) {
            continue;
        }

        seenBoatIds.insert(boatId, true);

        if (!quaiIdValue.isValid() || quaiIdValue.isNull() || !isPortState(state)) {
            unregisterBoat(boatId);
            continue;
        }

        const int quaiId = quaiIdValue.toInt();
        const int manualDurationSeconds = loadManualDockingDurationSeconds(boatId);
        const int allocatedDurationSeconds = manualDurationSeconds > 0
                                                 ? manualDurationSeconds
                                                 : parseDurationToSeconds(durationText);
        const Bateau boat(boatId, boatName, immatriculation, capacite, longueur, age,
                          maintenanceDate, employeeId, QString::number(quaiId), state);

        registerOrUpdateBoat(boat, quaiId, quaiNumber, allocatedDurationSeconds);
    }

    QList<QString> orphanBoatIds;
    for (auto it = trackedBoats.constBegin(); it != trackedBoats.constEnd(); ++it) {
        if (!seenBoatIds.contains(it.key())) {
            orphanBoatIds.append(it.key());
        }
    }

    for (const QString& boatId : orphanBoatIds) {
        unregisterBoat(boatId);
    }
}

void QuaiManager::registerOrUpdateBoat(const Bateau& boat, int quaiId, int quaiNumber, int allocatedDurationSeconds)
{
    const QString boatId = boat.getIdBateau();
    const QDateTime now = QDateTime::currentDateTime();

    auto it = trackedBoats.find(boatId);
    if (it == trackedBoats.end()) {
        TrackedBoatEntry entry;
        entry.boat = boat;
        entry.quaiId = quaiId;
        entry.quaiNumber = quaiNumber;
        entry.allocatedDurationSeconds = allocatedDurationSeconds > 0
                                             ? allocatedDurationSeconds
                                             : defaultAllocatedDurationSeconds();
        entry.registeredAt = now;
        entry.lastSeenAt = now;
        trackedBoats.insert(boatId, entry);

        qDebug() << "QuaiManager registered boat" << boat.getNomBateau()
                 << "at quai" << quaiNumber
                 << "for" << entry.allocatedDurationSeconds << "seconds";
        return;
    }

    TrackedBoatEntry& entry = it.value();
    const bool quaiChanged = entry.quaiId != quaiId;
    const bool durationChanged = allocatedDurationSeconds > 0
                                 && entry.allocatedDurationSeconds != allocatedDurationSeconds;

    entry.boat = boat;
    entry.lastSeenAt = now;
    entry.quaiId = quaiId;
    entry.quaiNumber = quaiNumber;

    if (quaiChanged || durationChanged) {
        entry.allocatedDurationSeconds = allocatedDurationSeconds > 0
                                             ? allocatedDurationSeconds
                                             : defaultAllocatedDurationSeconds();
        entry.registeredAt = now;
    }
}

void QuaiManager::unregisterBoat(const QString& boatId)
{
    trackedBoats.remove(boatId);
    clearManualDockingDuration(boatId);
}

void QuaiManager::markBoatAsAtSea(TrackedBoatEntry& entry)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "QuaiManager cannot process departure: database is not available.";
        return;
    }

    if (!db.transaction()) {
        qDebug() << "QuaiManager failed to start departure transaction:" << db.lastError().text();
        return;
    }

    QSqlQuery updateBoatQuery(db);
    updateBoatQuery.prepare(
        "UPDATE BATEAUX "
        "SET ETAT = :etat, IDQUAI = NULL, FREQUENCE_SORTIES = COALESCE(FREQUENCE_SORTIES, 0) + 1 "
        "WHERE IDBATEAU = :id");
    updateBoatQuery.bindValue(":etat", "En mer");
    updateBoatQuery.bindValue(":id", entry.boat.getIdBateau().toInt());

    if (!updateBoatQuery.exec()) {
        qDebug() << "QuaiManager failed to update boat departure:"
                 << updateBoatQuery.lastError().text();
        db.rollback();
        return;
    }

    QSqlQuery updateQuaiQuery(db);
    updateQuaiQuery.prepare(
        "UPDATE QUAIS "
        "SET ETAT = 'Disponible' "
        "WHERE IDQUAI = :idquai");
    updateQuaiQuery.bindValue(":idquai", entry.quaiId);

    if (!updateQuaiQuery.exec()) {
        qDebug() << "QuaiManager failed to free quai:"
                 << updateQuaiQuery.lastError().text();
        db.rollback();
        return;
    }

    if (!db.commit()) {
        qDebug() << "QuaiManager failed to commit departure transaction:" << db.lastError().text();
        db.rollback();
        return;
    }

    entry.boat.setEtat("En mer");
    entry.boat.setIdQuai("");
    clearManualDockingDuration(entry.boat.getIdBateau());
    speakBoat(entry.boat.getNomBateau());
    Arduino::triggerSoundEvent(Arduino::SoundEvent::Departure);
    BateauWindow::refreshAllTables();
    QuaisWindow::refreshAll();

    qDebug() << "Boat" << entry.boat.getNomBateau()
             << "left quai" << entry.quaiNumber
             << "after" << entry.allocatedDurationSeconds << "seconds"
             << "- status updated to En mer and quai released";
}

bool QuaiManager::isPortState(const QString& state) const
{
    const QString normalized = normalizeState(state);
    return normalized == "au port" || normalized == "en port" || normalized == "in port";
}

int QuaiManager::parseDurationToSeconds(const QString& durationText) const
{
    const QString trimmed = durationText.trimmed();
    if (trimmed.isEmpty()) {
        return defaultAllocatedDurationSeconds();
    }

    QRegularExpression hhmmRegex("^(\\d{1,2}):(\\d{1,2})(?::(\\d{1,2}))?$");
    const QRegularExpressionMatch hhmmMatch = hhmmRegex.match(trimmed);
    if (hhmmMatch.hasMatch()) {
        const int hours = hhmmMatch.captured(1).toInt();
        const int minutes = hhmmMatch.captured(2).toInt();
        const int seconds = hhmmMatch.captured(3).isEmpty() ? 0 : hhmmMatch.captured(3).toInt();
        return (hours * 3600) + (minutes * 60) + seconds;
    }

    QRegularExpression numberRegex("(\\d+)");
    const QRegularExpressionMatch numberMatch = numberRegex.match(trimmed);
    if (!numberMatch.hasMatch()) {
        return defaultAllocatedDurationSeconds();
    }

    const int value = numberMatch.captured(1).toInt();
    const QString normalized = trimmed.toLower();

    if (normalized.contains("sec")) {
        return value;
    }

    if (normalized.contains("hour") || normalized.contains("heure") || normalized.contains('h')) {
        return qMax(60, value * 3600);
    }

    return qMax(60, value * 60);
}

int QuaiManager::defaultAllocatedDurationSeconds() const
{
    return 60;
}
