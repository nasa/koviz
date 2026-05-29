#ifndef RUN_MONTE_CSV_H
#define RUN_MONTE_CSV_H

#include "mapvalue.h"
#include "run.h"
#include "datamodel_csv.h"
#include "datamodel_csv_run.h"

#include <QString>
#include <QStringList>
#include <QHash>

class RunMonteCsv : public Run
{
public:
    RunMonteCsv( CsvModel* csvModel,
                 uint runID,
                 const QStringList& timeNames,
                 const QStringList& runColumnNames,
                 const QHash<QString,QStringList> &varMap);
    ~RunMonteCsv() {}

    QStringList params() override;
    DataModel *dataModel(const QString& param) override;

private:
    QStringList _timeNames;
    QStringList _runColumnNames;
    const QHash<QString, QStringList> _varMap;
    CsvRunModel* _model;
    QStringList _params;
};

#endif
