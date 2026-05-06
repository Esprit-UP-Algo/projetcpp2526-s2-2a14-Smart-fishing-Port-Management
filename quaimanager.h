#ifndef QUAIMANAGER_H
#define QUAIMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QString>
#include <QTimer>

#include "bateau.h"

class QuaiManager : public QObject
{
    Q_OBJECT

public:
    explicit QuaiManager(QObject* parent = nullptr);

    void start();
    void stop();
    void refreshFromDatabase();

private slots:
    void monitorBoats();

private:
    struct TrackedBoatEntry {
        Bateau boat;
        int quaiId = 0;
        int quaiNumber = 0;
        int allocatedDurationSeconds = 0;
        QDateTime registeredAt;
        QDateTime lastSeenAt;
    };

    void synchronizeTrackedBoats();
    void registerOrUpdateBoat(const Bateau& boat, int quaiId, int quaiNumber, int allocatedDurationSeconds);
    void unregisterBoat(const QString& boatId);
    void markBoatAsAtSea(TrackedBoatEntry& entry);
    bool isPortState(const QString& state) const;
    int parseDurationToSeconds(const QString& durationText) const;
    int defaultAllocatedDurationSeconds() const;

    QHash<QString, TrackedBoatEntry> trackedBoats;
    QTimer pollingTimer;
};

#endif // QUAIMANAGER_H
