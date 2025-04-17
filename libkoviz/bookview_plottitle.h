#ifndef PLOTTITLEVIEW_H
#define PLOTTITLEVIEW_H

#include <QLabel>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QFontInfo>
#include "bookidxview.h"
#include "layoutitem_plottitle.h"

class PlotTitleView : public BookIdxView
{
    Q_OBJECT
public:
    explicit PlotTitleView(QWidget *parent = 0);

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent * event) override;

private:
    QLabel* _label;
    QVBoxLayout* _vBoxLayout;
    QPoint _mousePressPos;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;

};

#endif // PLOTTITLEVIEW_H
