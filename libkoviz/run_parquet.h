#ifndef RUN_PARQUET_H
#define RUN_PARQUET_H

#include "mapvalue.h"
#include "run.h"
#include "datamodel_parquet.h"
#include "datamodel_parquet_run.h"

#include <QString>
#include <QStringList>
#include <QHash>

class RunParquet : public Run
{
public:
    RunParquet(ParquetModel *parquetModel,
               const QString& parquetFile,
               uint runID,
               const QStringList& timeNames,
               const QStringList& runColumnNames,
               const QHash<QString,QStringList> &varMap);
    ~RunParquet();

    QStringList params() override;
    DataModel *dataModel(const QString& param) override;

private:
    QString _parquetFile;
    QStringList _timeNames;
    QStringList _runColumnNames;
    const QHash<QString, QStringList> _varMap;
    QStringList _params;
    ParquetRunModel* _model;
};

#endif
