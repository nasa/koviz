#include "tricktablemodel.h"

QString TrickTableModel::_err_string;
QTextStream TrickTableModel::_err_stream(&TrickTableModel::_err_string);

// Private
TrickTableModel::TrickTableModel(QObject *parent) :
      QAbstractTableModel(parent),
      _runPath(QString()),
      _rowCount(0),
      _colCount(0)
{
}


TrickTableModel::TrickTableModel(const QStringList& timeNames,
                                 const QString& runPath,
                                 const QStringList &paramList,
                                 QObject *parent)
    : QAbstractTableModel(parent),
      _runPath(runPath),
      _rowCount(0),
      _colCount(0)
{
    _colCount = paramList.count()+1; // +1 for timestamp
    _params = paramList;
    _params.prepend(timeNames.at(0));

    // If runPath is a trk, use it. If runPath is a directory,
    // get trk,csv,mot list from RUN directory.
    QStringList runPaths;
    QFileInfo fi(runPath);
    if ( fi.isFile() && fi.suffix() == "trk" ) {
        runPaths.append(fi.absoluteFilePath());
    } else if ( fi.isFile() && fi.suffix() == "csv" ) {
        runPaths.append(fi.absoluteFilePath());
    } else if ( fi.isFile() && fi.suffix() == "mot" ) {
        runPaths.append(fi.absoluteFilePath());
    } else if ( fi.isDir() ) {
        runPaths = _runPaths(runPath);
    } else {
        fprintf(stderr, "koviz [error]: runPath=%s is neither a trk, csv, mot "\
                        "file or a directory\n",
                        runPath.toLatin1().constData());
        exit(-1);
    }

    // Make list of data models in RUN that contain the table params
    // Also make hash of param->dataModel
    int nVars = paramList.count();
    int cntVars = 0;
    QString timeName;
    foreach ( QString path, runPaths ) {
        DataModel* dataModel=DataModel::createDataModel(timeNames,runPath,path);
        dataModel->map();
        foreach ( QString paramName, paramList ) {
            int cc = dataModel->columnCount();
            for ( int i = 0; i < cc; ++i ) {
                const Parameter* param = dataModel->param(i);
                if ( timeNames.contains(param->name()) ) {
                    timeName = param->name();
                    _param2model.insert(timeName,dataModel);
                    continue;
                }
                if ( param->name() == paramName ) {
                    if ( _param2model.contains(param->name()) ) continue;
                    if ( !_dataModels.contains(dataModel) ) {
                        _dataModels << dataModel;
                    }
                    _param2model.insert(param->name(),dataModel);
                    ++cntVars;
                    if ( cntVars == nVars ) break;
                }
            }
            if ( cntVars == nVars ) break;
        }
        dataModel->unmap();
        if ( cntVars == nVars ) break;
    }
    if ( cntVars != nVars ) {
        // Didn't find one of the table params in RUN
        // TODO: make error with bad param listed
    }

    // Make time stamps list
    foreach ( DataModel* dataModel, _dataModels ) {
        dataModel->map();
        int timeCol = dataModel->paramColumn(timeName);
        ModelIterator* it = dataModel->begin(timeCol,timeCol,timeCol);
        while ( !it->isDone() ) {
            double t = it->t();
            TimeStamps::insert(t,_timeStamps);
            it->next();
        }
        delete it;
        dataModel->unmap();
    }
}

TrickTableModel::~TrickTableModel()
{
}

QStringList TrickTableModel::_runPaths(const QString &runDir)
{
    QStringList paths;

    QDir dir(runDir);

    QStringList filter;
    filter << "*.trk" << "*.csv" << "*.mot";
    foreach(QString path, dir.entryList(filter, QDir::Files)) {
        paths.append(dir.absoluteFilePath(path));
    }
    if ( paths.empty() ) {
        _err_stream << "koviz [error]: no trk,csv,mot logfiles found in "
                    << runDir << "\n";
        throw std::invalid_argument(_err_string.toLatin1().constData());
    }

    return paths;
}

int TrickTableModel::rowCount(const QModelIndex &pidx) const
{
    return ( (!pidx.isValid()) ? _timeStamps.size() : 0 );
}

int TrickTableModel::columnCount(const QModelIndex &pidx) const
{
    return ( (!pidx.isValid()) ? _colCount : 0 );
}

QVariant TrickTableModel::data(const QModelIndex &idx, int role) const
{
    //Q_UNUSED(role);
    QVariant v;
    if ( _timeStamps.isEmpty() ) return v;

    if ( role == Qt::DisplayRole ) {
        if ( idx.column() == 0 ) {
            int r = idx.row();
            int rc = _timeStamps.size();
            if ( r >= 0 && r < rc ) {
                v = _timeStamps.at(r);
            }
        } else {
            QString param = _params.at(idx.column());
            DataModel* dataModel = _param2model.value(param);
            if ( dataModel ) {
                dataModel->map();
                int r = idx.row();
                double t = _timeStamps.at(r);
                int i = dataModel->indexAtTime(t);
                int tmCol = dataModel->paramColumn(param);
                if ( tmCol >= 0 ) {
                    QModelIndex tmIdx = dataModel->index(i,tmCol);
                    v = dataModel->data(tmIdx);
                }
                dataModel->unmap();
            }
        }
    }
    return v;
}

QVariant TrickTableModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    Q_UNUSED(role);

    QVariant v;
    if ( orientation == Qt::Horizontal ) {
        v = _params.at(section);
    }
    return v;
}

const Parameter *TrickTableModel::param(int col) const
{
    const Parameter* param = 0;
    QString paramName = _params.at(col);
    DataModel* dataModel = _param2model.value(paramName);
    int c = dataModel->paramColumn(paramName);
    param = dataModel->param(c);
    return param;
}
