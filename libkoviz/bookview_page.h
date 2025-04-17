#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QAbstractItemView>
#include <QList>
#include <QItemSelectionModel>
#include <QEvent>
#include <QMouseEvent>
#include <QPalette>

#include "bookidxview.h"
#include "bookview_pagetitle.h"
#include "bookview_plot.h"
#include "pagelayout.h"

class PageView : public BookIdxView
{
    Q_OBJECT
public:
    explicit PageView(QWidget *parent = 0);
    void setModel(QAbstractItemModel *model) override;
    void setRootIndex(const QModelIndex &index) override;

protected:
    void paintEvent(QPaintEvent * event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    PageLayout* _grid;
    bool _toggleSingleView;
    bool _isMouseDoubleClick;
    QModelIndex _mouseIdx1;
    QModelIndex _mouseIdx2;

private:
    void _toggleView(QObject* obj);

signals:

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent,
                              int start, int end) override;

protected slots:
    void _plotViewCurrentChanged(const QModelIndex& currIdx,
                                 const QModelIndex& prevIdx);

signals:
    
public slots:
    
};

#endif // PAGEVIEW_H
