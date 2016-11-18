#ifndef PLOTBOOKMODEL_H
#define PLOTBOOKMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QHash>
#include <QPainterPath>
#include <QVector>
#include "libsnap/montemodel.h"
#include "libsnap/unit.h"
#include "libsnap/utils.h"

#include <QList>
#include <QColor>

class PlotBookModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit PlotBookModel( MonteModel* monteModel,
                            QObject *parent = 0);
    explicit PlotBookModel( MonteModel* monteModel,
                            int rows, int columns, QObject * parent = 0 );
    ~PlotBookModel();

    virtual bool setData(const QModelIndex &idx,
                         const QVariant &value, int role=Qt::EditRole);

public:
    double xScale(const QModelIndex& curveIdx) const;
    double yScale(const QModelIndex& curveIdx) const;
    QRectF calcCurvesBBox(const QModelIndex& curvesIdx) const;

    QStandardItem* addChild(QStandardItem* parentItem,
                            const QString& childTitle,
                            const QVariant &childValue=QVariant());

    bool isIndex(const QModelIndex& idx,
                  const QString& itemText) const;

    bool isChildIndex(const QModelIndex& pidx,
                      const QString &expectedParentItemText,
                      const QString& childItemText) const;

    QModelIndex getIndex(const QModelIndex& startIdx,
                        const QString& searchItemText,
                        const QString &expectedStartIdxText=QString()) const;

    QModelIndex getDataIndex(const QModelIndex& startIdx,
                          const QString& searchItemText,
                          const QString &expectedStartIdxText=QString()) const;

    QString getDataString(const QModelIndex& startIdx,
                          const QString& searchItemText,
                          const QString &expectedStartIdxText=QString()) const;
    double getDataDouble(const QModelIndex& startIdx,
                         const QString& searchItemText,
                         const QString &expectedStartIdxText=QString()) const;
    int getDataInt(const QModelIndex& startIdx,
                         const QString& searchItemText,
                         const QString &expectedStartIdxText=QString()) const;

    // Convenience for getting QPen line style (see Qt doc)
    QVector<qreal> getLineStylePattern(const QModelIndex& curveIdx) const;

    TrickCurveModel* getTrickCurveModel(const QModelIndex& curvesIdx,
                                        int i) const;
    TrickCurveModel* getTrickCurveModel(const QModelIndex& curveIdx) const;

    QPainterPath* getCurvePainterPath(const QModelIndex& curveIdx) const;
    QPainterPath* getCurvesErrorPath(const QModelIndex& curvesIdx);
    QString getCurvesXUnit(const QModelIndex& curvesIdx);
    QString getCurvesYUnit(const QModelIndex& curvesIdx);

    QModelIndexList getIndexList(const QModelIndex& startIdx,
                        const QString& searchItemText,
                        const QString &expectedStartIdxText=QString()) const;

    // Convenience wrappers for getIndexList
    QModelIndexList pageIdxs() const ;
    QModelIndexList plotIdxs(const QModelIndex& pageIdx) const ;
    QModelIndexList curveIdxs(const QModelIndex& curvesIdx) const ;

    // Curve Colors
    QList<QColor> createCurveColors(int nCurves);

    // Line style

signals:
    
public slots:

private:
    MonteModel* _monteModel;
    void _initModel();
    QModelIndex _pageIdx(const QModelIndex& idx) const ;
    QModelIndex _plotIdx(const QModelIndex& idx) const ;
    QModelIndex _ancestorIdx(const QModelIndex &startIdx,
                        const QString& ancestorText,
                        const QString &expectedStartIdxText=QString()) const;

    QHash<TrickCurveModel*,QPainterPath*> _curve2path;
    QPainterPath* _createPainterPath(TrickCurveModel *curveModel);
    QPainterPath* _createCurvesErrorPath(const QModelIndex& curvesIdx) const;
};

#endif // PLOTBOOKMODEL_H
