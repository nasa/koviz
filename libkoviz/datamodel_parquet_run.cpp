#include "datamodel_parquet_run.h"

const QString ParquetRunModel::TimeName = QString(""); // No standard time name

ParquetRunModel::ParquetRunModel(ParquetModel *parquetModel,
                                 const QStringList& timeNames,
                                 const QStringList& runColumnNames,
                                 int64_t runID,
                                 const QString &runPath,
                                 const QString& parquetFile, QObject *parent) :
    DataModel(timeNames, runPath, parquetFile, parent),
    _parquetModel(parquetModel),
    _timeNames(timeNames),_runColumnNames(runColumnNames),_runID(runID),
    _parquetFile(parquetFile),
    _nrows(0), _ncols(0), _timeCol(0), _iteratorTimeIndex(0)
{
    _init();
}

void ParquetRunModel::_init()
{
#ifdef HAS_PARQUET
    _ncols = _parquetModel->columnCount();
    _nrows = _parquetModel->runRows(_runID).size();
#endif
}

void ParquetRunModel::map()
{
#ifdef HAS_PARQUET
    _parquetModel->map();
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
    }
    _iteratorTimeIndex = new ParquetRunModelIterator(0,this,
                                                 _timeCol,_timeCol,_timeCol);
#endif
}

void ParquetRunModel::unmap()
{
#ifdef HAS_PARQUET
    _parquetModel->unmap();
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
#endif
}

int ParquetRunModel::paramColumn(const QString &param) const
{
    return _parquetModel->paramColumn(param);
}

ModelIterator *ParquetRunModel::begin(int tcol, int xcol, int ycol) const
{
    return new ParquetRunModelIterator(0,this,tcol,xcol,ycol);
}

ParquetRunModel::~ParquetRunModel()
{
    _parquetModel->unmap();
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
    foreach ( Parameter* param, _col2param.values() ) {
        delete param;
    }
}

const Parameter* ParquetRunModel::param(int col) const
{
    return _parquetModel->param(col);
}

int ParquetRunModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int ParquetRunModel::_idxAtTimeBinarySearch (ParquetRunModelIterator* it,
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

int ParquetRunModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int ParquetRunModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}

QVariant ParquetRunModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(role);
    Q_UNUSED(idx);

    fprintf(stderr, "koviz [error]: Using unimplemented "
                    "ParquetRunModel::data().  Data should be retrieved "
                    "via ParquetRunModelIterators\n");

    return std::numeric_limits<double>::quiet_NaN();
}
