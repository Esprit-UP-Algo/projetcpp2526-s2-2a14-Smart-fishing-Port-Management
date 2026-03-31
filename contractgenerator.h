#ifndef CONTRACTGENERATOR_H
#define CONTRACTGENERATOR_H

#include <QString>
#include <QObject>
#include "quai.h"

class ContractGenerator : public QObject {
    Q_OBJECT
public:
    explicit ContractGenerator(QObject *parent = nullptr) : QObject(parent) {}

    static bool generateContract(const Quai &quai, const QString &client, const QString &company,
                                 int duration, const QString &startDate, QWidget *parent);
};
#endif
