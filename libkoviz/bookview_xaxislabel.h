#ifndef XAXISLABELVIEW_H
#define XAXISLABELVIEW_H

#include <QPainter>
#include <QString>
#include <QWheelEvent>
#include <QList>
#include <QMimeData>
#include <QMessageBox>
#include "bookidxview.h"
#include "layoutitem_xaxislabel.h"

class XAxisLabelView : public BookIdxView
{
    Q_OBJECT
public:
    explicit XAxisLabelView(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void wheelEvent(QWheelEvent* e) override;
    void dropEvent(QDropEvent *event) override;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;

    void rowsInserted(const QModelIndex &pidx, int start, int end) override;
};

#endif // XAXISLABELVIEW_H
