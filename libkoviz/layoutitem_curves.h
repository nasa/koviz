#ifndef LAYOUTITEM_CURVES_H
#define LAYOUTITEM_CURVES_H

#include <QPixmap>
#include "layoutitem_paintable.h"
#include "bookmodel.h"
#include "unit.h"

class CurvesLayoutItem : public PaintableLayoutItem
{
public:
    CurvesLayoutItem(PlotBookModel* bookModel,
                     const QModelIndex& plotIdx);
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

private:
    PlotBookModel* _bookModel;
    QModelIndex _plotIdx;
    QPixmap* _pixmap;
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
};

#endif // LAYOUTITEM_CURVES_H
