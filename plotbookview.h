#ifndef PLOTBOOKVIEW_H
#define PLOTBOOKVIEW_H

#include <QAbstractItemView>
#include <QScrollBar>
#include <QTabWidget>
#include <QHash>
#include "plotpage.h"
#include "montemodel.h"

class PlotBookView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit PlotBookView(QWidget *parent = 0);
    ~PlotBookView();

    void setData(MonteModel* monteModel);

    virtual QRect visualRect(const QModelIndex &index) const;
    virtual void scrollTo(const QModelIndex &index,
                          ScrollHint hint = EnsureVisible);
    virtual QModelIndex indexAt(const QPoint &point) const;

protected:
    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers);

    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;

    virtual bool isIndexHidden(const QModelIndex &index) const;

    virtual void setSelection(const QRect &rect,
                              QItemSelectionModel::SelectionFlags command);
    virtual QRegion visualRegionForSelection(
                              const QItemSelection &selection) const;
    virtual QItemSelectionModel::SelectionFlags selectionCommand(
                              const QModelIndex &index,
                              const QEvent *event = 0) const;

protected:

signals:
    
public slots:

protected slots:
    void rowsInserted(const QModelIndex &pidx, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    MonteModel* _monteModel;
    QTabWidget* _nb;

    QHash<QModelIndex,QFrame*> _frames;
    QHash<QModelIndex,QGridLayout*> _grids ;
    QHash<QModelIndex,Plot*> _plots ;
};

#endif // PLOTBOOKVIEW_H