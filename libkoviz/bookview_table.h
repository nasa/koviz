#ifndef BOOKTABLEVIEW_H
#define BOOKTABLEVIEW_H

#include <QAbstractItemView>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPalette>
#include <QBrush>
#include <QPen>
#include <QScrollBar>
#include <QList>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QKeyEvent>
#include <limits.h>

#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QGuiApplication>
#else
    #include <QtGui/QApplication>
#endif

#include "bookmodel.h"
#include "curvemodel.h"
#include "unit.h"
#include "timestamps.h"

class BookTableView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BookTableView(QWidget *parent = 0);

public:
    QModelIndex indexAt( const QPoint& point) const override;
    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index,
                  ScrollHint hint = EnsureVisible) override;

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

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
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;

private:
    PlotBookModel* _bookModel() const;
    QList<double> _timeStamps;
    int _mTop;
    int _mBot;
    int _mLft;
    int _mRgt;

    QStringList _columnLabels() const;

    QStringList _format(const QList<double>& vals);
    QStringList __format(const QList<double>& vals, const QString &format);

signals:

public slots:

};

#endif // BOOKTABLEVIEW_H
