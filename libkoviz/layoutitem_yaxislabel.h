#ifndef LAYOUTITEM_YAXISLABEL_H
#define LAYOUTITEM_YAXISLABEL_H

#include <QFontMetrics>
#include "layoutitem_paintable.h"
#include "bookmodel.h"

class YAxisLabelLayoutItem : public PaintableLayoutItem
{
public:
    YAxisLabelLayoutItem(const QFontMetrics& fontMetrics,
                         PlotBookModel* bookModel,
                         const QModelIndex& plotIdx);

    ~YAxisLabelLayoutItem();
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
    QString _yAxisLabelText() const;
};

#endif // LAYOUTITEM_YAXISLABEL_H
