#ifndef SHAREDWINDOWSTATE_H
#define SHAREDWINDOWSTATE_H

#include <QObject>

class SharedWindowState : public QObject
{
  Q_OBJECT

  public:
    explicit SharedWindowState(QObject *parent = nullptr);

    double liveCoordTime() const;
    int liveCoordTimeIndex() const;

    void setLiveCoordTime(double t);
    void setLiveCoordTimeIndex(int i);

  private:
    double _liveCoordTime;
    int _liveCoordTimeIndex;

  signals:
    void liveCoordTimeChanged(double t);
    void liveCoordTimeIndexChanged(int i);

};

#endif // SHAREDWINDOWSTATE_H
