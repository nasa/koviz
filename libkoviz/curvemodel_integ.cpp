#include "curvemodel_integ.h"

CurveModelIntegral::CurveModelIntegral(CurveModel *curveModel,
                                       const QStringList &timeNames,
                                       double start, double stop,
                                       QString xu, double xs, double xb,
                                       QString yu, double ys, double yb,
                                       double initial_value) :
    _ncols(3),
    _nrows(0),
    _t(new CurveModelParameter),
    _x(new CurveModelParameter),
    _y(new CurveModelParameter),
    _iteratorTimeIndex(0)
{
    if ( !timeNames.contains(curveModel->x()->name()) ) {
        fprintf(stderr,"koviz [bad scoobs]: CurveModelIntegral given curve "
                       "with x name=%s.  It must be time. Use -timeName.\n",
                curveModel->x()->name().toLatin1().constData());
        exit(-1);
    }
    if ( !Unit::canConvert(curveModel->x()->unit(),"s") ) {
        fprintf(stderr,"koviz [bad scoobs]: CurveModelIntegral given curve "
                       "with xunit=%s.  It must be time.\n",
                curveModel->x()->unit().toLatin1().constData());
        exit(-1);
    }

    _iteratorTimeIndex = new IntegralModelIterator(this);

    _fileName = curveModel->fileName();
    _runPath = curveModel->runPath();
    _t->setName(curveModel->t()->name());
    _t->setUnit(curveModel->t()->unit());
    _x->setName(curveModel->x()->name());
    _x->setUnit(curveModel->x()->unit());

    QString yName = curveModel->y()->name();
    QChar integSymbol(8747);
    QString iYName;
    if (  yName.startsWith(integSymbol) ) {
        iYName = QString("%1%2").arg(integSymbol).arg(yName);
    } else {
        iYName = QString("%1(%2)").arg(integSymbol).arg(yName);
    }
    _y->setName(iYName);

    QString yUnit = yu;
    if ( yu.isEmpty() || yu == "--" ) {
        yUnit = curveModel->y()->unit();
    }
    QString integUnit = Unit::integral(yUnit);
    _y->setUnit(integUnit);

    _init(curveModel,start,stop,xu,xs,xb,yUnit,ys,yb,initial_value);
}

CurveModelIntegral::~CurveModelIntegral()
{
    // See: ~CurveModel()
}

ModelIterator* CurveModelIntegral::begin() const
{
    return new IntegralModelIterator(this);
}

int CurveModelIntegral::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int CurveModelIntegral::_idxAtTimeBinarySearch (IntegralModelIterator* it,
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
                        return _idxAtTimeBinarySearch(it, mid+1, high, time);
                }
        }
}

int CurveModelIntegral::rowCount(const QModelIndex &pidx) const
{
    if ( !pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int CurveModelIntegral::columnCount(const QModelIndex &pidx) const
{
    if ( !pidx.isValid() ) {
        return 3;
    } else {
        return 0;
    }
}

// TODO CurveModel::data() --- for now, return empty QVariant
QVariant CurveModelIntegral::data (const QModelIndex & index, int role ) const

{
    Q_UNUSED(index);
    Q_UNUSED(role);
    QVariant v;
    return v;
}

// Use trapezoids to estimate integral
void CurveModelIntegral::_init(CurveModel* curveModel,
                               double start, double stop,
                               const QString& xu, double xs, double xb,
                               const QString& yu, double ys, double yb,
                               double initial_value)
{
    curveModel->map();
    ModelIterator* it = curveModel->begin();

    double xus = 1.0;
    double xub = 0.0;
    if ( !xu.isEmpty() ) {
        xus = Unit::scale(curveModel->x()->unit(),xu);
        xub = Unit::bias(curveModel->x()->unit(),xu);
    }
    double yus = 1.0;
    double yub = 0.0;
    if ( !yu.isEmpty() ) {
        yus = Unit::scale(curveModel->y()->unit(),yu);
        yub = Unit::bias(curveModel->y()->unit(),yu);
    }

    _nrows = 0;
    int n = curveModel->rowCount();
    for (int i = 0; i < n; ++i ) {
        double t =  (it->at(i)->x()*xus+xub)*xs+xb;
        if ( t >= start && t <= stop ) {
            ++_nrows;
        }
    }
    if ( _nrows == 0 ) {
        curveModel->unmap();
        delete it;
        return;
    }

    _data = (double*)malloc(_nrows*_ncols*sizeof(double));

    int j = 0;
    for (int i = 0; i < n; ++i ) {
        double t = (it->at(i)->x()*xus+xub)*xs+xb;
        if ( t < start || t > stop ) {
            continue;
        }
        _data[j*_ncols+0] = it->at(i)->t();
        _data[j*_ncols+1] = it->at(i)->x();
        if ( j == 0 ) {
            _data[j*_ncols+2] = initial_value;
        } else {
            double x0 = (it->at(i-1)->x()*xus+xub)*xs+xb;
            double y0 = (it->at(i-1)->y()*yus+yub)*ys+yb;
            double x1 = (it->at(i)->x()*xus+xub)*xs+xb;
            double y1 = (it->at(i)->y()*yus+yub)*ys+yb;
            for (int k = i-1; k >= 0; --k) {
                x0 = (it->at(k)->x()*xus+xub)*xs+xb;
                y0 = (it->at(k)->y()*yus+yub)*ys+yb;
                if ( !std::isnan(x0) && !std::isnan(y0) && x0 != x1 ) {
                    break;
                }
            }
            double area = (y0+y1)*(x1-x0)/2.0;
            if ( x1 == x0 ) {
                area = 0.0;
            }
            _data[j*_ncols+2] = _data[(j-1)*_ncols+2] + area;
        }
        ++j;
    }

    delete it;
    curveModel->unmap();
}
