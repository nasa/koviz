#include "sharedwindowstate.h"

SharedWindowState::SharedWindowState(QObject *parent)
    : QObject(parent), _liveCoordTime(0.0), _liveCoordTimeIndex(0)
{}

double SharedWindowState::liveCoordTime() const
{
    return _liveCoordTime;
}

int SharedWindowState::liveCoordTimeIndex() const
{
    return _liveCoordTimeIndex;
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
