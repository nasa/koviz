#include "datamodel_points.h"

PointsModel::PointsModel(QVector<QPointF>* points,
                         const QString& xName,
                         const QString& xUnit,
                         const QString& yName,
                         const QString& yUnit,
                         QObject *parent) :
    DataModel(QStringList() << xName,
              QString(), QString(), parent),
    _points(points),
    _xName(xName),
    _xUnit(xUnit),
    _yName(yName),
    _yUnit(yUnit),
    _xParam(new Parameter(xName,xUnit)),
    _yParam(new Parameter(yName,yUnit)),
    _iteratorTimeIndex(0)
{
    _init();
}

void PointsModel::_init()
{
    _iteratorTimeIndex = new PointsModelIterator(0,this);
}

void PointsModel::map()
{
}

void PointsModel::unmap()
{
}

int PointsModel::paramColumn(const QString &paramName) const
{
    if ( paramName == _xName ) {
        return 0;
    } else if ( paramName == _yName ) {
        return 1;
    } else {
        return -1;
    }
}

ModelIterator *PointsModel::begin(int tcol, int xcol, int ycol) const
{
    Q_UNUSED(tcol);
    Q_UNUSED(xcol);
    Q_UNUSED(ycol);
    return new PointsModelIterator(0,this);
}

PointsModel::~PointsModel()
{

    delete _xParam;
    delete _yParam;
    delete _points;
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
}

const Parameter* PointsModel::param(int col) const
{
    if ( col == 0 ) {
        return _xParam;
    } else if ( col == 1 ) {
        return _yParam;
    } else {
        return 0;
    }
}

int PointsModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int PointsModel::_idxAtTimeBinarySearch (PointsModelIterator* it,
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

int PointsModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _points->size();
    } else {
        return 0;
    }
}

int PointsModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return 2;
    } else {
        return 0;
    }
}


QVariant PointsModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(role);
    QVariant val;

    if ( idx.isValid() && _points ) {
        int row = idx.row();
        int col = idx.column();
        if ( col == 0 ) {
            val = _points->at(row).x();
        } else if ( col == 1 ) {
            val = _points->at(row).y();
        }
    }

    return val;
}
