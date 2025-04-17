#ifndef LABELEDRULERVIEW_H
#define LABELEDRULERVIEW_H

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QFontInfo>
#include "bookidxview.h"
#include "layoutitem_ticlabels.h"


class LabeledRulerView : public BookIdxView
{
    Q_OBJECT
public:
    explicit LabeledRulerView(Qt::Alignment alignment, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void wheelEvent(QWheelEvent *e) override;
};

#endif // LABELEDRULERVIEW_H
