#ifndef SHAREDWINDOWSTATE_H
#define SHAREDWINDOWSTATE_H

#include <QObject>
#include <QRectF>
#include <cmath>

class SharedWindowState : public QObject
{
  Q_OBJECT

  public:
    explicit SharedWindowState(QObject *parent = nullptr);

    double liveCoordTime() const;
    int liveCoordTimeIndex() const;
    QRectF plotMathRect() const;

    void setLiveCoordTime(double t);
    void setLiveCoordTimeIndex(int i);
    void setPlotMathRect(const QRectF& M, const QString& xScale);

  private:
    double _liveCoordTime;
    int _liveCoordTimeIndex;
    QRectF _plotMathRect;
    QString _plotMathRectXScale;

  signals:
    void liveCoordTimeChanged(double t);
    void liveCoordTimeIndexChanged(int i);
    void plotMathRectChanged(const QRectF& M, const QString& xScale);

};

#endif // SHAREDWINDOWSTATE_H
