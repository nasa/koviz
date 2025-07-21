#ifndef LAYOUTITEM_CURVES_H
#define LAYOUTITEM_CURVES_H

#include <QPixmap>
#include <QList>
#include "layoutitem_paintable.h"
#include "bookmodel.h"
#include "unit.h"
#include "roundoff.h"
#include "coord_arrow.h"

class TimeAndIndex2
{
  public:
    TimeAndIndex2(double time, int timeIdx, const QModelIndex& modelIdx) :
        _time(time),_timeIdx(timeIdx),_modelIdx(modelIdx)
    {}

    double time() const { return _time; };
    int timeIdx() const { return _timeIdx; };
    QModelIndex modelIdx() const { return _modelIdx; };

  private:
    TimeAndIndex2() {}
    double _time;
    int _timeIdx;  // Index for duplicate timestamps
    QModelIndex _modelIdx;
};

class ScaleBias
{
  public:
    ScaleBias(bool isXLogScaleIn, bool isYLogScaleIn,
              double xsIn, double xbIn,
              double ysIn, double ybIn) :
        isXLogScale(isXLogScaleIn),
        isYLogScale(isYLogScaleIn),
        xs(xsIn), xb(xbIn), ys(ysIn), yb(ybIn)
    {}
    bool isXLogScale;
    bool isYLogScale;
    double xs;
    double xb;
    double ys;
    double yb;
  private:
    ScaleBias() {}

};

class CurvesLayoutItem : public PaintableLayoutItem
{
public:
    CurvesLayoutItem(PlotBookModel* bookModel,
                     const QModelIndex& plotIdx,
                     const QModelIndex& currIdx);
    ~CurvesLayoutItem();
    Qt::Orientations expandingDirections() const override;
    QRect  geometry() const override;
    bool  isEmpty() const override;
    QSize  maximumSize() const override;
    QSize  minimumSize() const override;
    void  setGeometry(const QRect &r) override;
    QSize  sizeHint() const override;
    void paint(QPainter* painter,
               const QRect& R, const QRect& RG,
               const QRect& C, const QRectF& M) override;

    void paintHLines(QPainter* painter,
                     const QRect &R,const QRect &RG,
                     const QRect &C, const QRectF &M);

    void paintMarkers(QPainter* painter,
                      const QRect &R, const QRect &RG,
                      const QRect &C, const QRectF &M);

private:
    PlotBookModel* _bookModel;
    QModelIndex _plotIdx;
    QModelIndex _currIdx;
    QRect _rect;

    void _printCoplot(const QTransform& T,
                      QPainter *painter, const QModelIndex &plotIdx);
    void _printErrorplot(const QTransform& T,
                         QPainter *painter, const QModelIndex &plotIdx);
    void __paintSymbol(const QPointF &p,
                       const QString &symbol, QPainter* painter);
    void _paintGrid(QPainter* painter,
                    const QRect &R,const QRect &RG,
                    const QRect &C, const QRectF &M);
    void _paintCurvesLegend(const QRect& R,
                            const QModelIndex &curvesIdx,
                            QPainter* painter);
    void __paintCurvesLegend(const QRect& R,
                             const QModelIndex &curvesIdx,
                             const QList<QPen *> &pens,
                             const QStringList &symbols,
                             const QStringList &labels,
                             QPainter* painter);

    // Helper methods for paintMarkers()
    QList<TimeAndIndex2*> _getMarkers() const;
    ScaleBias _getMarkerScaleBias(TimeAndIndex2 *marker) const;
    QPainterPath* _getMarkerPath(TimeAndIndex2 *marker) const;
    int _getMarkerPathIdx(TimeAndIndex2 *marker, const ScaleBias &sb,
                          QPainterPath* path) const;
    void _paintMarker(QPainter* painter,
                      TimeAndIndex2* marker,
                      const QRect& R, const QRect &RG,
                      const QRect &C, const QRectF &M,
                      const ScaleBias& sb,
                      QPainterPath* path, int i);
    QString _format(double d);
    int _idxAtTimeBinarySearch(QPainterPath* path,
                               int low, int high, double time) const;
};

#endif // LAYOUTITEM_CURVES_H
