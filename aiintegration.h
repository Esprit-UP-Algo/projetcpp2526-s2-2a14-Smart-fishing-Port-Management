#ifndef AIINTEGRATION_H
#define AIINTEGRATION_H

#include <QList>
#include <QString>
#include <QtGlobal>
#include <QVariantMap>

struct DockCandidateContext {
    int quaiNumber = 0;
    int capacity = 0;
    QString state;
    double tariff = 0.0;
    int availabilityMinutes = 0;
    double anomalyScore = 0.0;
    double utilizationScore = 0.0;
};

struct DockAssignmentDecision {
    bool success = false;
    bool usedAiApi = false;
    bool usedLocalModel = false;
    int selectedQuaiNumber = -1;
    int estimatedDockingMinutes = 0;
    QString explanation;
};

struct DockAnalysisContext {
    int quaiNumber = 0;
    int capacity = 0;
    QString state;
    int sessionCount = 0;
    qint64 totalOccupiedSeconds = 0;
    qint64 averageOccupiedSeconds = 0;
    qint64 longestOccupiedSeconds = 0;
    bool hasEnergyWasteRisk = false;
    bool hasLongOccupationRisk = false;
};

struct DockAnalysisDecision {
    bool hasOverride = false;
    bool usedAiApi = false;
    bool usedLocalModel = false;
    double utilizationScore = 0.0;
    double anomalyScore = 0.0;
    QString statusLabel;
    QString recommendation;
    QString anomalySummary;
};

class DockIntelligenceService
{
public:
    static DockAssignmentDecision recommendAssignment(const QVariantMap& boatInfo,
                                                      const QList<DockCandidateContext>& candidates);
    static DockAnalysisDecision analyzeDock(const DockAnalysisContext& context);
};

struct EmployeeAnalysisContext {
    QString name;
    QString position;
    double salary = 0.0;
    int totalHours = 0;
    int sessionsCount = 0;
    double averageSessionHours = 0.0;
};

struct EmployeeAnalysisReport {
    bool success = false;
    QString analysis;
    QString recommendation;
    double productivityScore = 0.0;
    QString summary;
};

class EmployeeIntelligenceService
{
public:
    static EmployeeAnalysisReport generateSmartReport(const EmployeeAnalysisContext& context);
};

#endif // AIINTEGRATION_H
