#include "ortools_optimizer.h"
#include <QDebug>
#include <limits>
#include <cmath>

// Google OR-Tools Documentation & Repository
// GitHub: https://github.com/google/or-tools
// Docs:   https://developers.google.com/optimization/docs
//
// Google OR-Tools headers (you'll need to install these for full implementation)
// For now, we'll provide a simplified implementation
// In production, include: #include "ortools/linear_solver/linear_solver.h"

bool ORToolsOptimizer::canFit(int boatLength, int quaiCapacity) const
{
    return quaiCapacity >= boatLength;
}

int ORToolsOptimizer::calculateCost(const QVariantMap& boat, const Quai& quai) const
{
    // Cost matrix design:
    // Lower cost = better assignment

    int boatLength = boat.value("longueur").toInt();
    int quaiCapacity = quai.getCapacite();
    double quaiTarif = quai.getTarif();

    // Check if boat can fit
    if (!canFit(boatLength, quaiCapacity))
        return std::numeric_limits<int>::max(); // Impossible assignment

    // Cost factors (weighted):
    int spaceCost = (quaiCapacity - boatLength) * 100;      // Prefer minimal excess space
    int tarifCost = static_cast<int>(quaiTarif * 10);       // Prefer lower cost
    int distanceCost = std::abs(quaiCapacity - boatLength) * 50;  // Prefer perfect fit

    return spaceCost + tarifCost + distanceCost;
}

bool ORToolsOptimizer::findOptimalQuai(const QVariantMap& boatInfo,
                                       const QList<Quai>& quais,
                                       Quai& quaiChoisi,
                                       QString& explanation)
{
    int boatLength = boatInfo.value("longueur").toInt();
    QString boatName = boatInfo.value("nom").toString();

    if (quais.isEmpty()) {
        explanation = "Aucun quai disponible dans la base de donnees.";
        return false;
    }

    // Find best quai using cost matrix
    int bestQuaiIndex = -1;
    int bestCost = std::numeric_limits<int>::max();

    for (int i = 0; i < quais.size(); ++i) {
        const Quai& q = quais[i];

        // Priority 1: Available quais
        if (q.getEtat() == "Disponible") {
            int cost = calculateCost(boatInfo, q);
            if (cost < bestCost) {
                bestCost = cost;
                bestQuaiIndex = i;
            }
        }
    }

    // Fallback: Find future available quai (if none available now)
    if (bestQuaiIndex < 0) {
        for (int i = 0; i < quais.size(); ++i) {
            const Quai& q = quais[i];
            if (q.getEtat() != "Maintenance" && canFit(boatLength, q.getCapacite())) {
                int cost = calculateCost(boatInfo, q);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestQuaiIndex = i;
                }
            }
        }
    }

    if (bestQuaiIndex < 0) {
        explanation = QString("Aucun quai ne peut accueillir un bateau de %1 m.").arg(boatLength);
        return false;
    }

    quaiChoisi = quais[bestQuaiIndex];
    explanation = QString("OR-Tools optimization: %1 assigned to Quai %2. "
                          "Capacity: %3m (boat: %4m). Cost: %5.")
                      .arg(boatName)
                      .arg(quaiChoisi.getNumero())
                      .arg(quaiChoisi.getCapacite())
                      .arg(boatLength)
                      .arg(bestCost);
    return true;
}

QMap<QString, Quai> ORToolsOptimizer::assignMultipleBoats(const QList<QVariantMap>& bateaux,
                                                          const QList<Quai>& quais)
{
    QMap<QString, Quai> assignments;

    // Simple greedy approach (in production, use OR-Tools Hungarian algorithm)
    QList<Quai> availableQuais = quais;

    for (const QVariantMap& boat : bateaux) {
        QString boatId = boat.value("id").toString();
        Quai quaiChoisi;
        QString explanation;

        if (findOptimalQuai(boat, availableQuais, quaiChoisi, explanation)) {
            assignments.insert(boatId, quaiChoisi);
            // Remove assigned quai from availability (for this batch)
            // Find and remove by numero since Quai doesn't have operator==
            for (int i = 0; i < availableQuais.size(); ++i) {
                if (availableQuais[i].getNumero() == quaiChoisi.getNumero()) {
                    availableQuais.removeAt(i);
                    break;
                }
            }
        }
    }

    return assignments;
}
