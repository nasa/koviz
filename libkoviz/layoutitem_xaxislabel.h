#ifndef LAYOUTITEM_XAXISLABEL_H
#define LAYOUTITEM_XAXISLABEL_H

#include <QFontMetrics>
#include "layoutitem_paintable.h"
#include "bookmodel.h"

class XAxisLabelLayoutItem : public PaintableLayoutItem
{
public:
    XAxisLabelLayoutItem(const QFontMetrics& fontMetrics,
                         PlotBookModel* bookModel,
                         const QModelIndex& plotIdx);
    ~XAxisLabelLayoutItem();
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
    QRect _rect;
    QString _xAxisLabelText() const;
};

#endif // LAYOUTITEM_XAXISLABEL_H
