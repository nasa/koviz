#ifndef BOOKVIEW_TABWIDGET_H
#define BOOKVIEW_TABWIDGET_H

#include <QTabWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>

#include "bookmodel.h"

class BookViewTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit BookViewTabWidget(QWidget *parent = 0);
    void setBookModel(PlotBookModel* bookModel);

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

private:
    PlotBookModel* _bookModel;

signals:
    void signalDropEvent(QDropEvent *event, const QModelIndex& idx);

protected slots:

};

#endif // BOOKVIEW_TABWIDGET_H
