#ifndef LAYOUTITEM_PLOTCORNER_H
#define LAYOUTITEM_PLOTCORNER_H

#include <QFontMetrics>
#include "layoutitem_paintable.h"


class PlotCornerLayoutItem : public PaintableLayoutItem
{
public:
    PlotCornerLayoutItem(const QFontMetrics& fontMetrics,
                         Qt::Corner corner);
    ~PlotCornerLayoutItem();
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
    Qt::Corner _corner;
    QRect _rect;
};

#endif // LAYOUTITEM_PLOTCORNER_H
