#ifndef RUNFILE_H
#define RUNFILE_H

#include "run.h"
#include "mapvalue.h"
#include "datamodel_csv.h"
#include "datamodel_optitrack_csv.h"
#include "datamodel_mot.h"
#include "datamodel_hdf5.h"
#include "datamodel_acssl_xls.h"
#include "datamodel_parquet.h"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QHash>

class RunFile : public Run
{
public:
    RunFile(const QString& run,
           const QStringList& timeNames,
           const QHash<QString,QStringList> &varMap);
    ~RunFile() {}

    QStringList params() override;
    DataModel *dataModel(const QString& param) override;

    static bool isValid(const QString& run, const QStringList &timeNames);
private:
    QStringList _timeNames;
    const QHash<QString, QStringList> _varMap;
    QList<DataModel*> _models;
    QHash<QString, DataModel*> _param2model;
    QStringList _params;
};

#endif // RUNFILE_H
