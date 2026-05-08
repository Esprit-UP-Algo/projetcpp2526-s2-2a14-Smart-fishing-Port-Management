#include "aiintegration.h"

#include <QtMath>
#include <QByteArray>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <limits>
#include <cstdlib>

namespace {

QString envOrEmpty(const char* name)
{
    const QByteArray value = qgetenv(name);
    return QString::fromUtf8(value).trimmed();
}

int envIntOrDefault(const char* name, int fallback)
{
    bool ok = false;
    const int parsed = envOrEmpty(name).toInt(&ok);
    return ok ? parsed : fallback;
}

double clampScore(double value)
{
    return qBound(0.0, value, 100.0);
}

bool postJson(const QString& urlString, const QJsonObject& payload, QJsonObject& responseObject)
{
    if (urlString.isEmpty())
        return false;

    const QUrl url(urlString);
    if (!url.isValid())
        return false;

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QString bearerToken = envOrEmpty("PORTFLOW_AI_TOKEN");
    if (!bearerToken.isEmpty())
        request.setRawHeader("Authorization", "Bearer " + bearerToken.toUtf8());

    QNetworkReply* reply = manager.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, reply, &QNetworkReply::abort);

    timeoutTimer.start(envIntOrDefault("PORTFLOW_AI_TIMEOUT_MS", 3000));
    loop.exec();

    const bool ok = (reply->error() == QNetworkReply::NoError);
    const QByteArray bytes = reply->readAll();
    reply->deleteLater();

    if (!ok)
        return false;

    const QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject())
        return false;

    responseObject = doc.object();
    return true;
}

DockAssignmentDecision recommendAssignmentWithApi(const QVariantMap& boatInfo,
                                                  const QList<DockCandidateContext>& candidates)
{
    DockAssignmentDecision decision;

    QJsonArray candidateArray;
    for (const DockCandidateContext& candidate : candidates) {
        QJsonObject item;
        item.insert("quaiNumber", candidate.quaiNumber);
        item.insert("capacity", candidate.capacity);
        item.insert("state", candidate.state);
        item.insert("tariff", candidate.tariff);
        item.insert("availabilityMinutes", candidate.availabilityMinutes);
        item.insert("anomalyScore", candidate.anomalyScore);
        item.insert("utilizationScore", candidate.utilizationScore);
        candidateArray.append(item);
    }

    QJsonObject payload;
    payload.insert("mode", "dock_assignment");
    payload.insert("boat", QJsonObject::fromVariantMap(boatInfo));
    payload.insert("candidates", candidateArray);

    QJsonObject response;
    if (!postJson(envOrEmpty("PORTFLOW_AI_ASSIGNMENT_URL"), payload, response))
        return decision;

    const int quaiNumber = response.value("selectedQuaiNumber").toInt(-1);
    if (quaiNumber < 0)
        return decision;

    decision.success = true;
    decision.usedAiApi = true;
    decision.selectedQuaiNumber = quaiNumber;
    decision.estimatedDockingMinutes = response.value("estimatedDockingMinutes").toInt(
        qMax(20, boatInfo.value("longueur").toInt() * 3));
    decision.explanation = response.value("explanation").toString();
    return decision;
}

DockAssignmentDecision recommendAssignmentWithLocalModel(const QVariantMap& boatInfo,
                                                         const QList<DockCandidateContext>& candidates)
{
    DockAssignmentDecision decision;
    if (candidates.isEmpty())
        return decision;

    const int boatLength = boatInfo.value("longueur").toInt();
    const QString boatState = boatInfo.value("etat").toString();

    double bestScore = std::numeric_limits<double>::max();
    DockCandidateContext bestCandidate;

    for (const DockCandidateContext& candidate : candidates) {
        const double capacitySlack = qMax(0, candidate.capacity - boatLength);
        const double waitingPenalty = candidate.availabilityMinutes * 2.8;
        const double slackPenalty = capacitySlack * 1.6;
        const double tariffPenalty = candidate.tariff * 0.35;
        const double anomalyPenalty = candidate.anomalyScore * 1.25;
        const double loadPenalty = candidate.utilizationScore * 0.45;
        const double seaBoatBonus = (boatState == "En mer" && candidate.availabilityMinutes == 0) ? -12.0 : 0.0;
        const double score = waitingPenalty + slackPenalty + tariffPenalty + anomalyPenalty + loadPenalty + seaBoatBonus;

        if (score < bestScore) {
            bestScore = score;
            bestCandidate = candidate;
        }
    }

    const int estimatedDockingMinutes = qMax(20, static_cast<int>(qRound(boatLength * 2.9 + 18.0)));
    decision.success = true;
    decision.usedLocalModel = true;
    decision.selectedQuaiNumber = bestCandidate.quaiNumber;
    decision.estimatedDockingMinutes = estimatedDockingMinutes;
    decision.explanation = QString(
                               "Modele local actif : quai %1 retenu avec un score optimise selon la disponibilite (%2 min), "
                               "la marge de capacite (%3 m), le tarif (%4 DT) et le niveau de risque (%5%).")
                               .arg(bestCandidate.quaiNumber)
                               .arg(bestCandidate.availabilityMinutes)
                               .arg(qMax(0, bestCandidate.capacity - boatLength))
                               .arg(bestCandidate.tariff, 0, 'f', 1)
                               .arg(bestCandidate.anomalyScore, 0, 'f', 1);
    return decision;
}

DockAnalysisDecision analyzeDockWithApi(const DockAnalysisContext& context)
{
    DockAnalysisDecision decision;

    QJsonObject payload;
    payload.insert("mode", "dock_analysis");
    payload.insert("quaiNumber", context.quaiNumber);
    payload.insert("capacity", context.capacity);
    payload.insert("state", context.state);
    payload.insert("sessionCount", context.sessionCount);
    payload.insert("totalOccupiedSeconds", static_cast<double>(context.totalOccupiedSeconds));
    payload.insert("averageOccupiedSeconds", static_cast<double>(context.averageOccupiedSeconds));
    payload.insert("longestOccupiedSeconds", static_cast<double>(context.longestOccupiedSeconds));
    payload.insert("hasEnergyWasteRisk", context.hasEnergyWasteRisk);
    payload.insert("hasLongOccupationRisk", context.hasLongOccupationRisk);

    QJsonObject response;
    if (!postJson(envOrEmpty("PORTFLOW_AI_ANALYSIS_URL"), payload, response))
        return decision;

    decision.hasOverride = true;
    decision.usedAiApi = true;
    decision.utilizationScore = clampScore(response.value("utilizationScore").toDouble());
    decision.anomalyScore = clampScore(response.value("anomalyScore").toDouble());
    decision.statusLabel = response.value("statusLabel").toString();
    decision.recommendation = response.value("recommendation").toString();
    decision.anomalySummary = response.value("anomalySummary").toString();
    return decision;
}

DockAnalysisDecision analyzeDockWithLocalModel(const DockAnalysisContext& context)
{
    DockAnalysisDecision decision;

    const double averageHours = context.averageOccupiedSeconds / 3600.0;
    const double totalHours = context.totalOccupiedSeconds / 3600.0;
    const double longestHours = context.longestOccupiedSeconds / 3600.0;
    const double occupancyRatio = context.capacity > 0 ? qMin(1.0, totalHours / (context.capacity * 8.0)) : 0.0;
    const double burstFactor = qMin(1.0, context.sessionCount / 8.0);

    const double utilizationScore = clampScore((occupancyRatio * 42.0)
                                               + (averageHours * 7.5)
                                               + (burstFactor * 26.0)
                                               + (context.state.contains("occup", Qt::CaseInsensitive) ? 8.0 : 0.0));

    const double anomalyScore = clampScore((averageHours < 1.0 ? 12.0 : 0.0)
                                           + (longestHours > 10.0 ? 30.0 : 0.0)
                                           + (context.hasEnergyWasteRisk ? 24.0 : 0.0)
                                           + (context.hasLongOccupationRisk ? 28.0 : 0.0)
                                           + (occupancyRatio < 0.18 ? 10.0 : 0.0));

    QStringList anomalies;
    if (occupancyRatio < 0.18)
        anomalies << "Sous-utilisation probable";
    if (context.hasEnergyWasteRisk)
        anomalies << "Surcharge energetique";
    if (context.hasLongOccupationRisk)
        anomalies << "Occupation prolongee";
    if (anomalies.isEmpty())
        anomalies << "Signal nominal";

    decision.hasOverride = true;
    decision.usedLocalModel = true;
    decision.utilizationScore = utilizationScore;
    decision.anomalyScore = anomalyScore;
    decision.anomalySummary = anomalies.join(" | ");

    if (anomalyScore >= 75.0) {
        decision.statusLabel = "Alerte critique";
        decision.recommendation = "Modele local : reaffecter prioritairement les bateaux et declencher une revue operationnelle.";
    } else if (anomalyScore >= 45.0) {
        decision.statusLabel = "A surveiller";
        decision.recommendation = "Modele local : lisser la charge entre quais et reduire les temps d'occupation continus.";
    } else {
        decision.statusLabel = "Utilisation stable";
        decision.recommendation = "Modele local : conserver le plan actuel avec un suivi periodique des sessions.";
    }

    return decision;
}

EmployeeAnalysisReport generateSmartReportWithApi(const EmployeeAnalysisContext& context)
{
    EmployeeAnalysisReport report;

    QJsonObject payload;
    payload.insert("mode", "employee_report");
    payload.insert("name", context.name);
    payload.insert("position", context.position);
    payload.insert("salary", context.salary);
    payload.insert("totalHours", context.totalHours);
    payload.insert("sessionsCount", context.sessionsCount);
    payload.insert("averageSessionHours", context.averageSessionHours);

    QJsonObject response;
    if (!postJson(envOrEmpty("PORTFLOW_AI_EMPLOYEE_URL"), payload, response))
        return report;

    report.success = true;
    report.analysis = response.value("analysis").toString();
    report.recommendation = response.value("recommendation").toString();
    report.productivityScore = clampScore(response.value("productivityScore").toDouble());
    report.summary = response.value("summary").toString();
    return report;
}

EmployeeAnalysisReport generateSmartReportWithLocalModel(const EmployeeAnalysisContext& context)
{
    EmployeeAnalysisReport report;
    report.success = true;

    double hourlyRate = context.totalHours > 0 ? context.salary / context.totalHours : 0;
    double score = clampScore((context.totalHours / 160.0) * 50.0 + (context.sessionsCount / 20.0) * 50.0);
    
    report.productivityScore = score;
    report.summary = QString("Analyse de performance pour %1 (%2)").arg(context.name, context.position);
    
    if (score > 80) {
        report.analysis = QString("Performance exceptionnelle. L'employe montre un engagement eleve et une regularite exemplaire. "
                                  "Rentabilite calculee: %1 DT/h.").arg(hourlyRate, 0, 'f', 2);
        report.recommendation = "Envisager une prime de performance ou des responsabilites accrues.";
    } else if (score > 50) {
        report.analysis = "Performance stable et conforme aux attentes du port.";
        report.recommendation = "Maintenir le suivi actuel et encourager la participation aux formations.";
    } else {
        report.analysis = "Presence irreguliere ou volume horaire en dessous de la moyenne.";
        report.recommendation = "Prevoir un entretien de suivi pour identifier les points de blocage.";
    }

    return report;
}

} // namespace

DockAssignmentDecision DockIntelligenceService::recommendAssignment(const QVariantMap& boatInfo,
                                                                    const QList<DockCandidateContext>& candidates)
{
    DockAssignmentDecision decision = recommendAssignmentWithApi(boatInfo, candidates);
    if (decision.success)
        return decision;
    return recommendAssignmentWithLocalModel(boatInfo, candidates);
}

DockAnalysisDecision DockIntelligenceService::analyzeDock(const DockAnalysisContext& context)
{
    DockAnalysisDecision decision = analyzeDockWithApi(context);
    if (decision.hasOverride)
        return decision;
    return analyzeDockWithLocalModel(context);
}

EmployeeAnalysisReport EmployeeIntelligenceService::generateSmartReport(const EmployeeAnalysisContext& context)
{
    EmployeeAnalysisReport report = generateSmartReportWithApi(context);
    if (report.success)
        return report;
    return generateSmartReportWithLocalModel(context);
}
