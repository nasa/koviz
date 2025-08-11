#include "curvemodel_deriv.h"

CurveModelDerivative::CurveModelDerivative(CurveModel *curveModel,
                                           const QStringList &timeNames,
                                           double start, double stop,
                                           QString xu, double xs, double xb,
                                           QString yu, double ys, double yb) :
    _ncols(3),
    _nrows(0),
    _t(new CurveModelParameter),
    _x(new CurveModelParameter),
    _y(new CurveModelParameter),
    _iteratorTimeIndex(0)
{
    if ( !timeNames.contains(curveModel->x()->name()) ) {
        fprintf(stderr,"koviz [bad scoobs]: CurveModelDerivative given curve "
                       "with x name=%s.  It must be time. Use -timeName.\n",
                curveModel->x()->name().toLatin1().constData());
        exit(-1);
    }
    if ( !Unit::canConvert(curveModel->x()->unit(), "s") ) {
        fprintf(stderr,"koviz [bad scoobs]: CurveModelDerivative given curve "
                       "with xunit=%s.  It must be time.\n",
                curveModel->x()->unit().toLatin1().constData());
        exit(-1);
    }

    _iteratorTimeIndex = new DerivativeModelIterator(this);

    _fileName = curveModel->fileName();
    _runPath = curveModel->runPath();
    _t->setName(curveModel->t()->name());
    _t->setUnit(curveModel->t()->unit());
    _x->setName(curveModel->x()->name());
    _x->setUnit(curveModel->x()->unit());
    QString yName = "d'(" + curveModel->y()->name() + ")";
    _y->setName(yName);

    QString yUnit = yu;
    if ( yu.isEmpty() || yu == "--" ) {
        yUnit = curveModel->y()->unit();
    }
    QString dUnit = Unit::derivative(yUnit);
    _y->setUnit(dUnit);

    _init(curveModel,start,stop,xu,xs,xb,yUnit,ys,yb);
}

CurveModelDerivative::~CurveModelDerivative()
{
    // See: ~CurveModel()
}

ModelIterator* CurveModelDerivative::begin() const
{
    return new DerivativeModelIterator(this);
}

int CurveModelDerivative::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int CurveModelDerivative::_idxAtTimeBinarySearch (DerivativeModelIterator* it,
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

int CurveModelDerivative::rowCount(const QModelIndex &pidx) const
{
    if ( !pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int CurveModelDerivative::columnCount(const QModelIndex &pidx) const
{
    if ( !pidx.isValid() ) {
        return 3;
    } else {
        return 0;
    }
}

// TODO CurveModel::data() --- for now, return empty QVariant
QVariant CurveModelDerivative::data (const QModelIndex & index, int role ) const

{
    Q_UNUSED(index);
    Q_UNUSED(role);
    QVariant v;
    return v;
}

void CurveModelDerivative::_init(CurveModel* curveModel,
                                 double start, double stop,
                                 QString xu, double xs, double xb,
                                 QString yu, double ys, double yb)
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
        double y =  (it->at(i)->y()*yus+yub)*ys+yb;
        if ( t >= start && t <= stop && !std::isnan(t) && !std::isnan(y) ) {
            ++_nrows;
        }
    }
    if ( _nrows <= 1 ) {
        // Empty set or single point has no derivative since no change in x
        curveModel->unmap();
        delete it;
        return;
    }

    _data = (double*)malloc(_nrows*_ncols*sizeof(double));

    int j = 0;
    for (int i = 0; i < n; ++i ) {
        double t = (it->at(i)->x()*xus+xub)*xs+xb;
        double y = (it->at(i)->y()*yus+yub)*ys+yb;
        if ( t < start || t > stop || std::isnan(t) || std::isnan(y) ) {
            continue;
        }
        _data[j*_ncols+0] = it->at(i)->t();
        _data[j*_ncols+1] = it->at(i)->t();

        double derivative;
        if ( j == 0 ) { // Initial point

            // Try to use 4 points for derivative approximation

            double x0 = t;
            double y0 = y;

            double x1 = std::numeric_limits<double>::quiet_NaN();
            double y1 = std::numeric_limits<double>::quiet_NaN();
            int k;
            for (k = i+1; k < n; ++k) {
                it = it->at(k);
                if ( !it->isDone() ) {
                    x1 = (it->at(k)->x()*xus+xub)*xs+xb;
                    y1 = (it->at(k)->y()*yus+yub)*ys+yb;
                    if ( !std::isnan(x1) && !std::isnan(y1) && x0 != x1 ) {
                        break;
                    } else {
                        x1 = std::numeric_limits<double>::quiet_NaN();
                        y1 = std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }

            double x2 = std::numeric_limits<double>::quiet_NaN();
            double y2 = std::numeric_limits<double>::quiet_NaN();
            for (k = k+1; k < n; ++k) {
                it = it->at(k);
                if ( !it->isDone() ) {
                    x2 = (it->at(k)->x()*xus+xub)*xs+xb;
                    y2 = (it->at(k)->y()*yus+yub)*ys+yb;
                    if ( !std::isnan(x2) && !std::isnan(y2) && x1 != x2 ) {
                        break;
                    } else {
                        x2 = std::numeric_limits<double>::quiet_NaN();
                        y2 = std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }

            double x3 = std::numeric_limits<double>::quiet_NaN();
            double y3 = std::numeric_limits<double>::quiet_NaN();
            for (k = k+1; k < n; ++k) {
                it = it->at(k);
                if ( !it->isDone() ) {
                    x3 = (it->at(k)->x()*xus+xub)*xs+xb;
                    y3 = (it->at(k)->y()*yus+yub)*ys+yb;
                    if ( !std::isnan(x3) && !std::isnan(y3) && x2 != x3 ) {
                        break;
                    } else {
                        x3 = std::numeric_limits<double>::quiet_NaN();
                        y3 = std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }

            if ( !std::isnan(x0) && !std::isnan(y0) &&
                 !std::isnan(x1) && !std::isnan(y1) &&
                 !std::isnan(x2) && !std::isnan(y2) &&
                 !std::isnan(x3) && !std::isnan(y3) ) {

                // >= Four points (normal case) - use quadratic extrapolation
                double m0 = (y1-y0)/(x1-x0);
                double m1 = (y2-y1)/(x2-x1);
                double m2 = (y3-y2)/(x3-x2);
                double v1 = (m0+m1)/2.0;
                double v2 = (m1+m2)/2.0;
                double m = (v2-v1)/(x2-x1);
                derivative = m*(x0-x1)+v1;
            } else if ( !std::isnan(x0) && !std::isnan(y0) &&
                        !std::isnan(x1) && !std::isnan(y1) &&
                        !std::isnan(x2) && !std::isnan(y2) ) {
                // Three points
                double m0 = (y1-y0)/(x1-x0);
                double m1 = (y2-y1)/(x2-x1);
                double v1 = (m0+m1)/2.0;
                double m = (m1-m0)/(x2-x0);
                derivative = m*(x0-x1)+v1;
            } else if ( !std::isnan(x0) && !std::isnan(y0) &&
                        !std::isnan(x1) && !std::isnan(y1) ) {
                // Two points
                double m = (y1-y0)/(x1-x0);
                derivative = m;
            } else {
                // Empty or single point, let derivative be a nan
                derivative = std::numeric_limits<double>::quiet_NaN();
            }

        } else if ( j == _nrows-1 ) {

            // Last point (try to use 4 points)
            double x3 = t;
            double y3 = y;

            double x2 = std::numeric_limits<double>::quiet_NaN();
            double y2 = std::numeric_limits<double>::quiet_NaN();
            int k;
            for (k = i; k >= 0; --k) {
                x2 = (it->at(k)->x()*xus+xub)*xs+xb;
                y2 = (it->at(k)->y()*yus+yub)*ys+yb;
                if ( !std::isnan(x2) && !std::isnan(y2) && x2 != x3 ) {
                    break;
                } else {
                    x2 = std::numeric_limits<double>::quiet_NaN();
                    y2 = std::numeric_limits<double>::quiet_NaN();
                }
            }

            double x1 = std::numeric_limits<double>::quiet_NaN();
            double y1 = std::numeric_limits<double>::quiet_NaN();
            for (k = k-1; k >= 0; --k) {
                x1 = (it->at(k)->x()*xus+xub)*xs+xb;
                y1 = (it->at(k)->y()*yus+yub)*ys+yb;
                if ( !std::isnan(x1) && !std::isnan(y1) && x1 != x2 ) {
                    break;
                } else {
                    x1 = std::numeric_limits<double>::quiet_NaN();
                    y1 = std::numeric_limits<double>::quiet_NaN();
                }
            }

            double x0 = std::numeric_limits<double>::quiet_NaN();
            double y0 = std::numeric_limits<double>::quiet_NaN();
            for (k = k-1; k >= 0; --k) {
                x0 = (it->at(k)->x()*xus+xub)*xs+xb;
                y0 = (it->at(k)->y()*yus+yub)*ys+yb;
                if ( !std::isnan(x0) && !std::isnan(y0) && x0 != x1 ) {
                    break;
                } else {
                    x0 = std::numeric_limits<double>::quiet_NaN();
                    y0 = std::numeric_limits<double>::quiet_NaN();
                }
            }

            if ( !std::isnan(x0) && !std::isnan(y0) &&
                 !std::isnan(x1) && !std::isnan(y1) &&
                 !std::isnan(x2) && !std::isnan(y2) &&
                 !std::isnan(x3) && !std::isnan(y3) ) {

                // >= Four points

                double m0 = (y1-y0)/(x1-x0);
                double m1 = (y2-y1)/(x2-x1);
                double m2 = (y3-y2)/(x3-x2);

                double v1 = (m0+m1)/2.0;
                double v2 = (m1+m2)/2.0;

                double m = (v2-v1)/(x2-x1);

                derivative = m*(x3-x2)+v2;

            } else if ( !std::isnan(x1) && !std::isnan(y1) &&
                        !std::isnan(x2) && !std::isnan(y2) &&
                        !std::isnan(x3) && !std::isnan(y3) ) {

                // Three points
                double m0 = (y2-y1)/(x2-x1);
                double m1 = (y3-y2)/(x3-x2);
                double v2 = (m0+m1)/2.0;
                double m = (m1-m0)/(x3-x1);

                derivative = m*(x3-x2)+v2;

            } else if ( !std::isnan(x2) && !std::isnan(y2) &&
                        !std::isnan(x3) && !std::isnan(y3) ) {
                // Two points
                double m = (y3-y2)/(x3-x2);
                derivative = m;
            } else {
                // Empty or single point - no derivative - set to nan
                derivative = std::numeric_limits<double>::quiet_NaN();
            }

        } else {
            // Inside/Non-edge points (avg slope before and after)

            // Middle point
            double x1 = t;
            double y1 = y;

            // Previous point
            double x0 = std::numeric_limits<double>::quiet_NaN();
            double y0 = std::numeric_limits<double>::quiet_NaN();
            int k;
            for (k = i-1; k >= 0; --k) {
                x0 = (it->at(k)->x()*xus+xub)*xs+xb;
                y0 = (it->at(k)->y()*yus+yub)*ys+yb;
                if ( !std::isnan(x0) && !std::isnan(y0) && x0 != x1 ) {
                    break;
                } else {
                    x0 = std::numeric_limits<double>::quiet_NaN();
                    y0 = std::numeric_limits<double>::quiet_NaN();
                }
            }

            // Next point
            double x2 = std::numeric_limits<double>::quiet_NaN();
            double y2 = std::numeric_limits<double>::quiet_NaN();
            for (k = i+1; k < n; ++k) {
                it = it->at(k);
                if ( !it->isDone() ) {
                    x2 = (it->at(k)->x()*xus+xub)*xs+xb;
                    y2 = (it->at(k)->y()*yus+yub)*ys+yb;
                    if ( !std::isnan(x2) && !std::isnan(y2) && x2 != x1 ) {
                        break;
                    } else {
                        x2 = std::numeric_limits<double>::quiet_NaN();
                        y2 = std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }

            if ( !std::isnan(x0) && !std::isnan(y0) &&
                 !std::isnan(x2) && !std::isnan(y2) ) {
                // Use all three points to approximate deriv at x1
                double m0 = (y1-y0)/(x1-x0);
                double m1 = (y2-y1)/(x2-x1);
                double m = (m0+m1)/2.0;
                derivative = m;
            } else if ( !std::isnan(x0) && !std::isnan(y0) ) {
                // Use (x0,y0) only
                double m0 = (y1-y0)/(x1-x0);
                derivative = m0;
            } else if ( !std::isnan(x2) && !std::isnan(y2) ) {
                // Use (x1,y1) only
                double m1 = (y2-y1)/(x2-x1);
                derivative = m1;
            } else {
                // All points about (x1,y1) nan
                derivative = std::numeric_limits<double>::quiet_NaN();
            }
        }

        _data[j*_ncols+2] = derivative;
        ++j;
    }

    delete it;
    curveModel->unmap();
}
