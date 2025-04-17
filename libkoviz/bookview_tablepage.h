#ifndef TABLEPAGEVIEW_H
#define TABLEPAGEVIEW_H

#include <QAbstractItemView>
#include <QGridLayout>
#include <QList>
#include <QItemSelectionModel>

#include "bookidxview.h"
#include "bookview_pagetitle.h"
#include "bookview_table.h"

class TablePageView : public BookIdxView
{
    Q_OBJECT
public:
    explicit TablePageView(QWidget *parent = 0);
    ~TablePageView();
    void setModel(QAbstractItemModel *model) override;
    void setRootIndex(const QModelIndex &index) override;

private:
    QGridLayout* _grid;
    PageTitleView* _titleView;
    BookTableView* _tableView;

signals:

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;

signals:
    
public slots:
    
};

#endif // TABLEPAGEVIEW_H
