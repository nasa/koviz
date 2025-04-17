#ifndef PLOTVIEW_H
#define PLOTVIEW_H

#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QList>
#include <QEvent>
#include <QMouseEvent>
#include <QRubberBand>
#include <QApplication>

#include "bookidxview.h"
#include "bookview_plottitle.h"
#include "bookview_corner.h"
#include "bookview_linedruler.h"
#include "bookview_labeledruler.h"
#include "bookview_curves.h"
#include "bookview_xaxislabel.h"
#include "bookview_yaxislabel.h"
#include "plotlayout.h"

class PlotView : public BookIdxView
{
    Q_OBJECT

public:
    explicit PlotView(QWidget *parent = 0);
    void setModel(QAbstractItemModel *model) override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    PlotLayout* _grid;
    PlotTitleView* _titleView ;
    XAxisLabelView* _xAxisLabel;
    YAxisLabelView* _yAxisLabel;
    CornerView* _tlCorner;
    CornerView* _trCorner;
    CornerView* _blCorner;
    CornerView* _brCorner;
    CurvesView* _curvesView;
    LinedRulerView* _tTics;
    LinedRulerView* _bTics;
    LinedRulerView* _rTics;
    LinedRulerView* _lTics;
    LabeledRulerView* _xTicLabels;
    LabeledRulerView* _yTicLabels;

    QPoint _rubberBandOrigin;
    QRubberBand* _rubberBand;

    Qt::MouseButton _buttonRubberBandZoom;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;

protected slots:
    void _childViewCurrentChanged(const QModelIndex& currIdx,
                                  const QModelIndex& prevIdx);
};

#endif // PLOTVIEW_H
