#include "runfile.h"

RunFile::RunFile(const QString &run,
               const QStringList &timeNames,
               const QHash<QString, QStringList> &varMap) :
    _timeNames(timeNames),
    _varMap(varMap)
{
    QFileInfo fi(run);

    if ( ! RunFile::isValid(run,timeNames) ) {
        fprintf(stderr, "koviz [error]: Invalid run file=%s\n"
                         "It does not exist, is a non-Trick formatted csv, "
                         "a non-Tab delimited non-Acssl formatted xls, "
                         "is an unsupported format or could not find time.\n",
                run.toLatin1().constData());
        if ( fi.suffix() == "h5" || fi.suffix() == "hdf5" ) {
#ifdef HAS_HDF5
            fprintf(stderr, "koviz [error]: Run file=%s\n"
                    "has an hdf5 extension.  It could be that the hdf5 file "
                    "is not Trick hdf5, has no time history or -timeName "
                    "unspecified.\n",
                run.toLatin1().constData());
#else
            fprintf(stderr, "koviz [error]: Run file=%s,\n"
                    "but koviz was not built with hdf5 support.  "
                    "See user's guide section HDF5 and/or try installing "
                    "the hdf5 devel package and then rebuild koviz.\n",
                    run.toLatin1().constData());
#endif
        }
        exit(-1);
    }

    QString ffile = fi.absoluteFilePath();
    _models = DataModel::createDataModels(timeNames,run,ffile);
    foreach ( DataModel* model, _models ) {
        model->unmap();
        int ncols = model->columnCount();
        for ( int col = 0; col < ncols; ++col ) {
            QString param = model->param(col)->name();
            foreach (QString key, varMap.keys() ) {
                if ( param == key ) {
                    break;
                }
                QStringList vals = varMap.value(key);
                QStringList names;
                foreach ( QString val, vals ) {
                    MapValue mapval(val);
                    names.append(mapval.name());
                }
                if ( names.contains(param) ) {
                    param = key;
                    break;
                }
            }
            _param2model.insert(param,model);
            _params.append(param);
        }
    }
}

QStringList RunFile::params()
{
    return _params;
}

// Note: Since hdf5 files can have multiple models, different params
//       can have different models (like a RunDir)
DataModel *RunFile::dataModel(const QString &param)
{
    DataModel* model = 0;

    if ( _params.contains(param) ) {
        // Normal case
        model = _param2model.value(param);
    }

    if ( !model ) {
        // Look in timeNames for param
        if ( _timeNames.contains(param) ) {
            foreach ( QString timeName, _timeNames ) {
                if ( _params.contains(timeName) ) {
                    model = _param2model.value(timeName);
                    break;
                }
            }
        }
    }

    if ( !model ) {
        // Look in varmap for param
        foreach (QString key, _varMap.keys()) {
            QStringList names;
            foreach (QString val, _varMap.value(key)) {
                MapValue mapval(val);
                names.append(mapval.name());
            }
            if ( names.contains(param) ) {
                bool isFound = false;
                foreach (const QString& name, names ) {
                    if ( _param2model.keys().contains(name) ) {
                        model = _param2model.value(name);
                        isFound = true;
                        break;
                    }
                }
                if ( isFound ) break;
            }
        }
    }

    return model;
}

bool RunFile::isValid(const QString &run, const QStringList& timeNames)
{
    QFileInfo fi(run);
    if ( ! fi.exists() ) {
        return false;
    }

    QStringList suffixes = {"trk","csv","mot","h5","hdf5","xls"};
    if ( !suffixes.contains(fi.suffix()) ) {
        return false;
    }

    if ( fi.suffix() == "csv" ) {
        if ( !CsvModel::isValid(run,timeNames) &&
             !OptiTrackCsvModel::isValid(run) ) {
            return false;
        }
    } else if ( fi.suffix() == "mot" ) {
        if ( !MotModel::isValid(run) ) {
            return false;
        }
    } else if ( fi.suffix() == "h5" || fi.suffix() == "hdf5" ) {
        #ifdef HAS_HDF5
            if ( !Hdf5Model::isValid(run,timeNames) ) {
                return false;
            }
        #else
            return false;
        #endif
    } else if ( fi.suffix() == "xls" ) {
        if ( !AcsslXlsModel::isValid(run,timeNames) ) {
            return false;
        }
    }

    return true;
}
