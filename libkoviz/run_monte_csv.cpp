#include "run_monte_csv.h"


RunMonteCsv::RunMonteCsv(CsvModel *csvModel, uint runID,
                         const QStringList &timeNames,
                         const QStringList &runColumnNames,
                         const QHash<QString, QStringList> &varMap) :
    _timeNames(timeNames),
    _runColumnNames(runColumnNames),
    _varMap(varMap)
{
    int ncols = csvModel->columnCount();
    for (int col = 0; col < ncols; ++col) {
        const Parameter* param = csvModel->param(col);
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

    _model = new CsvRunModel(csvModel, timeNames, QString::number(runID),
                             csvModel->fileName(), _runColumnNames, runID);

}

QStringList RunMonteCsv::params()
{
    return _params;
}

DataModel *RunMonteCsv::dataModel(const QString &param)
{
    if ( !_params.contains(param) ) {
        return nullptr;
    }

    return _model;
}
