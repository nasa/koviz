#include "sharedwindowstate.h"

SharedWindowState::SharedWindowState(QObject *parent)
    : QObject(parent), vidView(0), _liveCoordTime(0.0), _liveCoordTimeIndex(0)
{}

double SharedWindowState::liveCoordTime() const
{
    return _liveCoordTime;
}

int SharedWindowState::liveCoordTimeIndex() const
{
    return _liveCoordTimeIndex;
}

QRectF SharedWindowState::plotMathRect() const
{
    return _plotMathRect;
}

void SharedWindowState::setLiveCoordTime(double t)
{
    if (_liveCoordTime == t) {
            return;
    }
    _liveCoordTime = t;
    emit liveCoordTimeChanged(t);
}


void SharedWindowState::setLiveCoordTimeIndex(int i)
{
    if (_liveCoordTimeIndex == i) {
            return;
    }
    _liveCoordTimeIndex = i;
    emit liveCoordTimeIndexChanged(i);
}

void SharedWindowState::setPlotMathRect(const QRectF &M, const QString &xScale)
{
    if (_plotMathRect.left() == M.left() &&
        _plotMathRect.right() == M.right()  &&
        _plotMathRectXScale == xScale) {
        return;
    }
    if ( xScale == "log" && _plotMathRectXScale == "linear" ) {
        if ( qFuzzyCompare(M.left(),log10(_plotMathRect.left())) &&
             qFuzzyCompare(M.right(),log10(_plotMathRect.right())) ) {
            return;
        }
    }
    if ( xScale == "linear" && _plotMathRectXScale == "log" ) {
        if ( qFuzzyCompare(M.left(),pow(10,_plotMathRect.left())) &&
             qFuzzyCompare(M.right(),pow(10,_plotMathRect.right())) ) {
            return;
        }
    }
    _plotMathRect = M;
    _plotMathRectXScale = xScale;
    emit plotMathRectChanged(M,xScale);
}

void SharedWindowState::onVideoWindowDestroyed()
{
    vidView = nullptr;
}
