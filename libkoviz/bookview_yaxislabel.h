#ifndef YAXISLABELVIEW_H
#define YAXISLABELVIEW_H

#include <QPainter>
#include <QString>
#include <QWheelEvent>
#include "bookidxview.h"
#include "layoutitem_yaxislabel.h"

class YAxisLabelView : public BookIdxView
{
    Q_OBJECT
public:
    explicit YAxisLabelView(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void wheelEvent(QWheelEvent *e) override;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &pidx, int start, int end) override;


signals:
    
public slots:
    
};

#endif // YAXISLABELVIEW_H
