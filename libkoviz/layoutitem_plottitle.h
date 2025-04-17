#ifndef LAYOUTITEM_PLOTTITLE_H
#define LAYOUTITEM_PLOTTITLE_H

#include <QFontMetrics>
#include "layoutitem_paintable.h"
#include "bookmodel.h"

class PlotTitleLayoutItem : public PaintableLayoutItem
{
public:
    PlotTitleLayoutItem(const QFontMetrics& fontMetrics,
                        PlotBookModel* bookModel,
                        const QModelIndex& plotIdx);
    ~PlotTitleLayoutItem();
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
    const QFontMetrics _fontMetrics;
    PlotBookModel* _bookModel;
    QModelIndex _plotIdx;
    QRect _rect;
    void _paintGrid(QPainter* painter,
                    const QRect& R, const QRect &RG,
                    const QRect&C, const QRectF& M);
};

#endif // LAYOUTITEM_PLOTTITLE_H
