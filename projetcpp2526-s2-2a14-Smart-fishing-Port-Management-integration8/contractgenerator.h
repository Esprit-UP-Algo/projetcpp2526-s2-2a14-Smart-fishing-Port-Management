#ifndef CONTRACTGENERATOR_H
#define CONTRACTGENERATOR_H

#include "quai.h"
#include <QString>
#include <QDate>
#include <QWidget>

class ContractGenerator {
public:
    static bool generateContract(const Quai& quai, const QString& clientName,
                                 const QString& clientCompany, const QString& duration,
                                 const QDate& startDate, QWidget* parent);
};

#endif // CONTRACTGENERATOR_H
