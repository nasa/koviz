#ifndef PAGETITLEVIEW_H
#define PAGETITLEVIEW_H

#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QPainter>
#include <QSize>
#include <QSizePolicy>

#include "layoutitem_pagetitle.h"
#include "bookidxview.h"

class PageTitleView : public BookIdxView
{
    Q_OBJECT
public:
    explicit PageTitleView(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize sizeHint() const override;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
};

#endif // PAGETITLEVIEW_H
