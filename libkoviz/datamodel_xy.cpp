#include "datamodel_xy.h"

XYModel::XYModel(const QStringList& timeNames,
                 double timeMatchTolerance,
                 DataModel *xModel, const QString& xName,
                 DataModel *yModel, const QString& yName,
                 QObject *parent) :
    DataModel(timeNames, QString(), QString(), parent),
    _timeNames(timeNames),
    _timeMatchTolerance(timeMatchTolerance),
    _xModel(xModel),_xName(xName),
    _yModel(yModel),_yName(yName),
    _nrows(0), _ncols(3), _iteratorTimeIndex(0), _data(0)
{
    _init();
}

void XYModel::_init()
{
    _xParam = new Parameter;
    _xParam->setName(_xName);
    int xcol = _xModel->paramColumn(_xName);
    if ( xcol < 0 ) {
        fprintf(stderr, "koviz [error]: XYModel could not find param=%s\n",
                _xName.toLatin1().constData());
        exit(-1);
    }
    _xParam->setUnit(_xModel->param(xcol)->unit());

    _yParam = new Parameter;
    _yParam->setName(_yName);
    int ycol = _yModel->paramColumn(_yName);
    if ( ycol < 0 ) {
        fprintf(stderr, "koviz [error]: XYModel could not find param=%s\n",
                _yName.toLatin1().constData());
        exit(-1);
    }
    _yParam->setUnit(_yModel->param(ycol)->unit());

    int xTimeCol = -1;
    _tParam = new Parameter;
    bool isXTimeFound = false;
    foreach (QString timeName, _timeNames) {
        int ncols = _xModel->columnCount();
        for (int col = 0; col < ncols; ++col) {
            QString paramName = _xModel->param(col)->name();
            if ( paramName == timeName ) {
                _tParam->setName(timeName);
                _tParam->setUnit(_xModel->param(col)->unit());
                xTimeCol = col;
                isXTimeFound = true;
                break;
            }
        }
        if ( isXTimeFound ) {
            break;
        }
    }
    if ( !isXTimeFound ) {
        fprintf(stderr, "koviz [error]: Could not find associated time for "
                "x parameter \"%s\"\n", _xName.toLatin1().constData());
    }

    int yTimeCol = -1;
    QString yTimeName;
    QString yTimeUnit;
    bool isYTimeFound = false;
    foreach (QString timeName, _timeNames) {
        int ncols = _yModel->columnCount();
        for (int col = 0; col < ncols; ++col) {
            QString paramName = _yModel->param(col)->name();
            if ( paramName == timeName ) {
                yTimeName = timeName;
                yTimeUnit = _yModel->param(col)->unit();
                yTimeCol = col;
                isYTimeFound = true;
                break;
            }
        }
        if ( isYTimeFound ) {
            break;
        }
    }
    if ( !isYTimeFound ) {
        fprintf(stderr, "koviz [error]: Could not find associated time for "
                "y parameter \"%s\"\n", _yName.toLatin1().constData());
    }

    _iteratorTimeIndex = new XYModelIterator(0,this,0,1,2);

    _xModel->map();
    _yModel->map();
    double timeUnitScale = Unit::scale(yTimeUnit,_tParam->unit());
    ModelIterator* itX = _xModel->begin(xTimeCol, xcol, xcol);
    ModelIterator* itY = _yModel->begin(yTimeCol, ycol, ycol);

    // Count rows in data model
    _nrows = 0;
    while ( !itX->isDone() && !itY->isDone()) {
        double xTime = itX->t();
        double yTime = itY->t()*timeUnitScale;
        if ( qAbs(xTime-yTime) <= _timeMatchTolerance ) {
            ++_nrows;
            itX->next();
            itY->next();
        } else {
            if ( xTime < yTime ) {
                itX->next();
            } else if ( yTime < xTime ) {
                itY->next();
            } else {
                // Odd case, NANs involved most likely
                if ( qIsNaN(xTime) ) {
                    itX->next();
                }
                if ( qIsNaN(yTime) ) {
                    itY->next();
                }
                if ( !qIsNaN(xTime) && !qIsNaN(yTime) ) {
                    // Really odd case that I think can never happen
                    itX->next();
                    itY->next();
                }
            }
        }
    }

    // Allocate block to hold t,x,y model data
    _ncols = 3; // t,x,y
    _data = (double*)malloc(_nrows*_ncols*sizeof(double));

    // Load data
    itX = itX->at(0);
    itY = itY->at(0);
    int row = 0;
    while ( !itX->isDone() && !itY->isDone()) {
        double xTime = itX->t();
        double yTime = itY->t()*timeUnitScale;
        if ( qAbs(xTime-yTime) <= _timeMatchTolerance ) {
            _data[row*_ncols+0] = xTime;
            _data[row*_ncols+1] = itX->x();
            _data[row*_ncols+2] = itY->y();
            ++row;
            itX->next();
            itY->next();
        } else {
            if ( xTime < yTime ) {
                itX->next();
            } else if ( yTime < xTime ) {
                itY->next();
            } else {
                // Odd case, NANs involved most likely
                if ( qIsNaN(xTime) ) {
                    itX->next();
                }
                if ( qIsNaN(yTime) ) {
                    itY->next();
                }
                if ( !qIsNaN(xTime) && !qIsNaN(yTime) ) {
                    // Really odd case that I think can never happen
                    itX->next();
                    itY->next();
                }
            }
        }
    }

    // Clean up
    _xModel->unmap();
    _yModel->unmap();
}

void XYModel::map()
{
}

void XYModel::unmap()
{
}

int XYModel::paramColumn(const QString &paramName) const
{
    int col = -1;

    if ( paramName == _tParam->name() ) {
        col = 0;
    } else if ( paramName == _xParam->name() ) {
        col = 1;
    } else if ( paramName == _yParam->name() ) {
        col = 2;
    }

    return col;
}

ModelIterator *XYModel::begin(int tcol, int xcol, int ycol) const
{
    return new XYModelIterator(0,this,tcol,xcol,ycol);
}

XYModel::~XYModel()
{
    if ( _data ) {
        free(_data);
        _data = 0;
    }

    delete _iteratorTimeIndex;

    delete _tParam;
    delete _xParam;
    delete _yParam;
}

const Parameter* XYModel::param(int col) const
{
    Parameter* p = 0;
    if ( col == 0 ) {
        p = _tParam;
    } else if ( col == 1 ) {
        p = _xParam;
    } else if ( col == 2 ) {
        p = _yParam;
    }
    return p;
}

int XYModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int XYModel::_idxAtTimeBinarySearch (XYModelIterator* it,
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

int XYModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int XYModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}


QVariant XYModel::data(const QModelIndex &idx, int role) const
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
