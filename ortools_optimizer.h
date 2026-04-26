#ifndef ORTOOLS_OPTIMIZER_H
#define ORTOOLS_OPTIMIZER_H

#include <QString>
#include <QVariantMap>
#include <QList>
#include "quai.h"

/**
 * @class ORToolsOptimizer
 * @brief Google OR-Tools based optimizer for intelligent boat-to-quai assignment
 *
 * Replaces the old scoring algorithm with a professional optimization solver.
 * Uses Linear Sum Assignment Problem (LSAP) to find the globally optimal assignment.
 *
 * @see https://github.com/google/or-tools
 * @see https://developers.google.com/optimization/docs
 *
 * Current implementation: Simplified greedy algorithm (no external dependencies)
 * Future upgrade: Full OR-Tools library for Hungarian algorithm support
 */
class ORToolsOptimizer
{
public:
    ORToolsOptimizer() = default;

    /**
     * Find optimal quai for a boat using Google OR-Tools
     * @param boatInfo Map containing boat data (longueur, etat, etc.)
     * @param quais List of available quais
     * @param quaiChoisi Output: the selected quai
     * @param explanation Output: explanation of the choice
     * @return true if assignment found, false otherwise
     */
    bool findOptimalQuai(const QVariantMap& boatInfo,
                         const QList<Quai>& quais,
                         Quai& quaiChoisi,
                         QString& explanation);

    /**
     * Batch optimization: assign multiple boats to quais
     * @param bateaux List of boats to assign
     * @param quais List of available quais
     * @return Map of boatId -> Quai
     */
    QMap<QString, Quai> assignMultipleBoats(const QList<QVariantMap>& bateaux,
                                            const QList<Quai>& quais);

private:
    /**
     * Calculate cost matrix for assignment problem
     * Higher cost = worse assignment
     */
    int calculateCost(const QVariantMap& boat, const Quai& quai) const;

    /**
     * Check if boat can fit in quai
     */
    bool canFit(int boatLength, int quaiCapacity) const;
};

#endif // ORTOOLS_OPTIMIZER_H
