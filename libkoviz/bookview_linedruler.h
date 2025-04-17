#ifndef LINEDRULERVIEW_H
#define LINEDRULERVIEW_H

#include <QPainter>

#include "bookidxview.h"
#include "layoutitem_plottics.h"

class LinedRulerView : public BookIdxView
{
    Q_OBJECT
public:
    explicit LinedRulerView(Qt::Alignment alignment, QWidget *parent = 0);

protected:
    void setModel(QAbstractItemModel* model) override;
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;


};

#endif // LINEDRULERVIEW_H
