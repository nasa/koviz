#ifndef BOOKIDXVIEW_H
#define BOOKIDXVIEW_H

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEngine>
#include <QPen>
#include <QEasingCurve>
#include <QDropEvent>
#include <qmath.h>
#include <cmath>
#include "bookmodel.h"
#include "unit.h"
#include "layoutitem_curves.h"

struct LabelBox
{
    QPointF center; // preferred center
    QString strVal;
    QRectF  bb;     // actual center is center of bb
};

class BookIdxView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BookIdxView(QWidget *parent = 0);

public:
    void setCurvesView(QAbstractItemView* view);
    virtual void setCurrentCurveRunID(int runID);

public:
    void setModel(QAbstractItemModel *model) override;
    void setRootIndex(const QModelIndex &index) override;
    QModelIndex indexAt( const QPoint& point) const override;
    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index,
                          ScrollHint hint = EnsureVisible) override;

    // Necessary for ignoring events from middle mouse for rubberbanding
    // in PlotView.  If events are not ignored, the event filtering will
    // not catch mouse events.  This keeps from doing a
    // qApp->installEventFilter which would have to filter *all* app events
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    QTransform _coordToPixelTransform() ;

    QRectF _mathRect() ;
    QModelIndex _plotMathRectIdx(const QModelIndex& plotIdx) const;

protected:
    QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &index) const override;
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) override;
    QRegion visualRegionForSelection(
                              const QItemSelection &selection) const override;
    Qt::Alignment _alignment;

protected:
    PlotBookModel *_bookModel() const;
    QAbstractItemView* _curvesView;

    Qt::MouseButton _buttonSelectAndPan;
    Qt::MouseButton _buttonRubberBandZoom;
    Qt::MouseButton _buttonResetView;

    QPoint _mousePressPos;
    QRectF _mousePressMathRect;

    QString _curvesXUnit(const QModelIndex& plotIdx) const;
    QString _curvesUnit(const QModelIndex& plotIdx, QChar axis) const;

    void _paintCurvesLegend(const QRect &R,
                            const QModelIndex& curvesIdx, QPainter &painter);
    void __paintCurvesLegend(const QRect &R,
                             const QModelIndex& curvesIdx,
                             const QList<QPen*>& pens,
                             const QStringList& symbols,
                             const QStringList& labels,
                             QPainter& painter);
    void __paintSymbol(const QPointF &p, const QString& symbol,
                               QPainter& painter);
    void _paintGrid(QPainter& painter, const QModelIndex &plotIdx);
    void _paintHLines(QPainter& painter, const QModelIndex &plotIdx);
    void _paintMarkers(QPainter& painter, const QModelIndex &plotIdx);

    QList<QAbstractItemView*> _childViews;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;

signals:
    void signalDropEvent(QDropEvent* dropEvent, const QModelIndex& idx);

public slots:
    void slotDropEvent(QDropEvent* dropEvent, const QModelIndex& idx);

};

#endif // BOOKIDXVIEW_H
