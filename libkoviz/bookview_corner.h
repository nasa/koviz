#ifndef CORNERVIEW_H
#define CORNERVIEW_H

#include <QPainter>
#include "bookidxview.h"
#include "layoutitem_plotcorner.h"

class CornerView : public BookIdxView
{
    Q_OBJECT
public:
    explicit CornerView(Qt::Corner corner, QWidget *parent = 0);
    void setModel(QAbstractItemModel *model) override;

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    Qt::Corner _corner;
};

#endif // CORNERVIEW_H
