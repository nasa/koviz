#ifndef LAYOUTITEM_PLOTTICS_H
#define LAYOUTITEM_PLOTTICS_H

#include <QFontMetrics>
#include "layoutitem_paintable.h"
#include "layoutitem_plotcorner.h"
#include "bookmodel.h"

class PlotTicsLayoutItem : public PaintableLayoutItem
{
public:
    PlotTicsLayoutItem(const QFontMetrics& fontMetrics,
                       PlotBookModel* bookModel,
                       const QModelIndex& plotIdx);
    ~PlotTicsLayoutItem();
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



private:
    QFontMetrics _fontMetrics;
    PlotBookModel* _bookModel;
    QModelIndex _plotIdx;
    PlotCornerLayoutItem _cornerLayoutItem;
    QRect _rect;
};

#endif // LAYOUTITEM_PLOTTICS_H
