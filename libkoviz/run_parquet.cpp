#include "run_parquet.h"


RunParquet::RunParquet(ParquetModel *parquetModel,
                       const QString& parquetFile,
                       uint runID,
                       const QStringList &timeNames,
                       const QStringList &runColumnNames,
                       const QHash<QString, QStringList> &varMap) :
    _parquetFile(parquetFile),
    _timeNames(timeNames),
    _runColumnNames(runColumnNames),
    _varMap(varMap)
{
    _model = new ParquetRunModel(parquetModel,timeNames,runColumnNames,
                                 runID,parquetFile,parquetFile);

    int ncols = _model->columnCount();
    for (int col = 0; col < ncols; ++col) {
        const Parameter* param = _model->param(col);
        QString paramName = param->name();
        foreach (QString key, varMap.keys() ) {
            // If paramName is in map, use var map key as paramName
            if ( paramName == key ) {
                break;
            }
            QStringList vals = varMap.value(key);
            QStringList names;
            foreach ( QString val, vals ) {
                MapValue mapval(val);
                names.append(mapval.name());
            }
            if ( names.contains(paramName) ) {
                paramName = key;
                break;
            }
        }
        _params.append(paramName);
    }
}

RunParquet::~RunParquet()
{
    delete _model;
}

QStringList RunParquet::params()
{
    return _params;
}

DataModel *RunParquet::dataModel(const QString &param)
{
    if ( !_params.contains(param) ) {
        return nullptr;
    }

    return _model;
}
