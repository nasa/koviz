#ifndef PLOTLAYOUT_H
#define PLOTLAYOUT_H

#include <QLayout>
#include <QLayoutItem>
#include <QWidgetItem>
#include <QWidget>
#include <QString>
#include <QHash>
#include <QList>

#include <stdio.h>

class PlotLayout : public QLayout
{
public:
    PlotLayout();
    ~PlotLayout();

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;
    QRect geometry() const override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    QLayout* layout() override;

    void addWidget(QWidget *widget);
    void setPlotRatio(const QString& plotRatio);
    QString plotRatio() const ;

private:
    QHash<QString,QLayoutItem*> _name2item;
    QList<QLayoutItem*> _items;
    QRect _rect;
    QString _plotRatio;
};

#endif // PLOTLAYOUT_H
