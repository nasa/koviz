#ifndef SHAREDWINDOWSTATE_H
#define SHAREDWINDOWSTATE_H

#include <QObject>
#include <QRectF>
#include <cmath>

#include "libkoviz/videowindow.h"

class SharedWindowState : public QObject
{
  Q_OBJECT

  public:
    explicit SharedWindowState(QObject *parent = nullptr);

    double liveCoordTime() const;
    int liveCoordTimeIndex() const;
    QRectF plotMathRect() const;

    VideoWindow* vidView;

  public slots:
    void setLiveCoordTime(double t);
    void setLiveCoordTimeIndex(int i);
    void setPlotMathRect(const QRectF& M, const QString& xScale);
    void onVideoWindowDestroyed();

  signals:
    void liveCoordTimeChanged(double t);
    void liveCoordTimeIndexChanged(int i);
    void plotMathRectChanged(const QRectF& M, const QString& xScale);

  private:
    double _liveCoordTime;
    int _liveCoordTimeIndex;
    QRectF _plotMathRect;
    QString _plotMathRectXScale;


};

#endif // SHAREDWINDOWSTATE_H
