#ifndef PAGETITLELAYOUTITEM_H
#define PAGETITLELAYOUTITEM_H

#include <QStringList>
#include <QFont>
#include "layoutitem_paintable.h"
#include "bookmodel.h"

class PageTitleLayoutItem : public PaintableLayoutItem
{
public:
    PageTitleLayoutItem(PlotBookModel* bookModel,
                        const QModelIndex& pageIdx,
                        const QFont& font);

    ~PageTitleLayoutItem();
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
    PlotBookModel* _bookModel;
    QModelIndex _pageIdx;
    QFont _font;
    QRect _rect;
    QSize _sizeLegend() const;
    QStringList _pageTitles(const QModelIndex &pageIdx) const;
    void _paintPageLegend(const QRect &R,
                          const QModelIndex &curvesIdx,
                          QPainter* painter);
    void __paintPageLegend(const QRect &R,
                           const QList<QPen *> &pens,
                           const QStringList &symbols,
                           const QStringList &labels,
                           QPainter* painter);
    void _paintLegendEntry(const QRect &R,
                           int l,  // line width
                           int s,  // spaceBetweenLineAndLabel
                           const QPen &pen,
                           const QString &symbol,
                           const QString &label,
                           QPainter* painter);
    void __paintSymbol(const QPointF& p,
                       const QString &symbol,
                       QPainter* painter);
};

#endif // PAGETITLELAYOUTITEM_H
