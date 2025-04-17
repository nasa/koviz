#ifndef PAGELAYOUT_H
#define PAGELAYOUT_H

#include <QLayout>
#include <QWidget>
#include <QWidgetItem>
#include <QLayoutItem>
#include <QString>
#include <stdio.h>
#include <stdlib.h>

#include "bookmodel.h"

class PageLayout : public QLayout
{
public:
    PageLayout();
    ~PageLayout();

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

    void addWidget(QWidget *widget);
    void setModelIndex(PlotBookModel* bookModel, const QModelIndex& pageIdx);

private:
    QList<QLayoutItem*> _items;
    PlotBookModel* _bookModel;
    QModelIndex _pageIdx;
    QRect _calcRect(int pageWidth,int pageHeight,
                    int titleHeight, const QRectF &r);

};

#endif // PAGELAYOUT_H
