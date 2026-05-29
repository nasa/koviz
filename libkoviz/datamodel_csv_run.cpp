#include "datamodel_csv_run.h"

QString CsvRunModel::_err_string;
QTextStream CsvRunModel::_err_stream(&CsvRunModel::_err_string);

// Csv is normally Trick, but can be any csv, so this is a guess
const QString CsvRunModel::TimeName = QString("sys.exec.out.time");

CsvRunModel::CsvRunModel(const CsvModel* csvModel,
                         const QStringList& timeNames,
                         const QString &runPath,
                         const QString& csvfile,
                         const QStringList &runColumnNames,
                         int runID,
                         QObject *parent) :
    DataModel(timeNames, runPath, csvfile, parent),
    _csvModel(csvModel),
    _timeNames(timeNames),_csvfile(csvfile),
    _runColumnNames(runColumnNames),_runID(runID),
    _nrows(0), _ncols(0),_iteratorTimeIndex(0),
    _data(0)
{
    _init();
}

void CsvRunModel::_init()
{
    _ncols = _csvModel->columnCount();

    //
    // Time Column
    //
    _timeCol = -1;
    int col = 0;
    foreach ( QString timeName, _timeNames ) {
        if ( _csvModel->paramColumn(timeName) >= 0 ) {
            _timeCol = col;
            break;
        }
        ++col;
    }
    if ( _timeCol < 0 ) {
        // Should not happen since isValid should check this
        fprintf(stderr, "koviz [error]: CsvRunModel::_init: could not find "
                "time column in file=%s\n", _csvfile.toLatin1().constData());
        exit(-1);
    }

    //
    // Run column
    //
    int runCol = -1;
    for ( int col = 0; col < _ncols; ++col ) {
        const Parameter* param = _csvModel->param(col);
        if ( _runColumnNames.contains(param->name()) ) {
            runCol = col;
            break;
        }
    }
    if ( runCol < 0 ) {
        fprintf(stderr, "koviz [error]: CsvRunModel::_init: could not find "
                "run column=%s in file=%s\n",
                _runColumnNames.join(",").toLatin1().constData(),
                _csvfile.toLatin1().constData());
        exit(-1);
    }

    _iteratorTimeIndex = new CsvRunModelIterator(0,this,
                                                 _timeCol,_timeCol,_timeCol);

    ModelIterator* it = _csvModel->begin(_timeCol,_timeCol,runCol);
    _nrows = 0;
    int row = 0;
    while ( !it->isDone() ) {
        if ( it->y() == (double)_runID ) {
            _runRows.append(row);
            ++_nrows;
        }
        it->next();
        ++row;
    }
    delete it;
}

void CsvRunModel::map()
{
}

void CsvRunModel::unmap()
{
}

int CsvRunModel::paramColumn(const QString &paramName) const
{
    return _csvModel->paramColumn(paramName);
}

ModelIterator *CsvRunModel::begin(int tcol, int xcol, int ycol) const
{
    return new CsvRunModelIterator(0,this,tcol,xcol,ycol);
}

CsvRunModel::~CsvRunModel()
{
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
}

const Parameter* CsvRunModel::param(int col) const
{
    return _csvModel->param(col);
}

int CsvRunModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int CsvRunModel::_idxAtTimeBinarySearch (CsvRunModelIterator* it,
                                       int low, int high, double time)
{
        if (high <= 0 ) {
                return 0;
        }
        if (low >= high) {
                // Time not found, choose closest near high
                double t1 = it->at(high-1)->t();
                double t2 = it->at(high)->t();
                double t3 = t2;
                it = it->at(high+1);
                if ( !it->isDone() ) {
                    t3 = it->at(high+1)->t();
                }

                int i;
                if ( qAbs(time-t1) < qAbs(time-t2) ) {
                    if ( qAbs(time-t1) < qAbs(time-t3) ) {
                        i = high-1;
                    } else {
                        if ( !it->isDone() ) {
                            i = high+1;
                        } else {
                            i = high;
                        }
                    }
                } else {
                    if ( qAbs(time-t2) < qAbs(time-t3) ) {
                        i = high;
                    } else {
                        if ( !it->isDone() ) {
                            i = high+1;
                        } else {
                            i = high;
                        }
                    }
                }
                return i;
        } else {
                int mid = (low + high)/2;
                if (time == it->at(mid)->t()) {
                        return mid;
                } else if ( time < it->at(mid)->t() ) {
                        return _idxAtTimeBinarySearch(it,
                                                      low, mid-1, time);
                } else {
                        return _idxAtTimeBinarySearch(it,
                                                      mid+1, high, time);
                }
        }
}

int CsvRunModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int CsvRunModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}


QVariant CsvRunModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(role);
    QVariant val;

    if ( idx.isValid() && _data ) {
        int row = idx.row();
        int col = idx.column();
        val = _data[row*_ncols+col];
    }

    return val;
}

bool CsvRunModel::isValid(const QString &csvFile,
                          const QStringList &timeNames,
                          const QStringList &runColumnNames)
{
    // This checks if time name exists and does sanity check on second line
    if ( !CsvModel::isValid(csvFile,timeNames) ) {
        return false;
    }

    if ( runColumnNames.isEmpty() ) {
        return false;
    }

    // Check to see if run column exists
    QFile file(csvFile);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QTextStream in(&file);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
    #else
        in.setCodec("UTF-8");
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const auto KeepEmptyParts = Qt::KeepEmptyParts ;
    #else
        const auto KeepEmptyParts = QString::KeepEmptyParts;
    #endif

    // Get list of variable names
    QStringList names;
    QString line0 = in.readLine();
    QStringList items = line0.split(',',KeepEmptyParts);
    foreach ( QString item, items ) {
        QString name;
        if ( item.contains('{') ) {
            // Name left of unit between curlies
            int i = item.indexOf('{');
            if ( i > 0 ) {
                name = item.left(i-1).trimmed();
            }
        } else {
            name = item.trimmed();
        }
        if ( name.isEmpty() ) {
            file.close();
            return false;
        }
        names.append(name);
    }

    // Ensure time param exists (double check actually since CsvModel::isValid
    // already did this)
    bool isFoundTime = false;
    foreach (QString timeName, timeNames) {
        if ( names.contains(timeName) ) {
            isFoundTime = true;
            break;
        }
    }
    if ( ! isFoundTime ) {
        file.close();
        return false;
    }

    // Ensure run column exists
    bool isFoundRunColumn = false;
    foreach (QString runColumnName, runColumnNames) {
        if ( names.contains(runColumnName) ) {
            isFoundRunColumn = true;
            break;
        }
    }
    if ( ! isFoundRunColumn ) {
        file.close();
        return false;
    }

    // Sanity check second line to ensure num cols is same as num header cols
    QString line1 = in.readLine();
    if ( !line1.isEmpty() ) {  // A single line header with no data is valid
        items = line1.split(',',KeepEmptyParts);
        if ( items.size() != names.size() ) {
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}
