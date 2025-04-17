#ifndef POINTS_MODEL_H
#define POINTS_MODEL_H

#include <QVariant>
#include <QVector>
#include <QPointF>

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

// Wraps a vector of points into a model
// x is time
// y is specified by user
//
// PointsModel takes ownership of points and deletes it

class PointsModel;
class PointsModelIterator;

class PointsModel : public DataModel
{
  Q_OBJECT

  friend class PointsModelIterator;

  public:

    explicit PointsModel(QVector<QPointF>* points,
                         const QString& xName,
                         const QString& xUnit,
                         const QString& yName,
                         const QString& yUnit,
                         QObject *parent = 0);
    ~PointsModel();

    const Parameter* param(int col) const override ;
    void map() override;
    void unmap() override;
    int paramColumn(const QString& paramName) const override ;
    ModelIterator* begin(int tcol, int xcol, int ycol) const override ;
    int indexAtTime(double time) override;

    int rowCount(const QModelIndex & pidx = QModelIndex() ) const override;
    int columnCount(const QModelIndex & pidx = QModelIndex() ) const override;
    QVariant data (const QModelIndex & index,
                           int role = Qt::DisplayRole ) const override;

  private:

    QVector<QPointF>* _points;
    QString _xName;
    QString _xUnit;
    QString _yName;
    QString _yUnit;

    Parameter* _xParam;
    Parameter* _yParam;

    PointsModelIterator* _iteratorTimeIndex;

    void _init();
    int _idxAtTimeBinarySearch (PointsModelIterator *it,
                               int low, int high, double time);
};

class PointsModelIterator : public ModelIterator
{
  public:

    inline PointsModelIterator(): i(0) {}

    inline PointsModelIterator(int row, // iterator pos
                               const PointsModel* model) :
        i(row),
        _model(model)
    {}

    virtual ~PointsModelIterator() {}

    void start() override
    {
        i = 0;
    }

    void next() override
    {
        ++i;
    }

    bool isDone() const override
    {
        return ( i >= _model->rowCount() ) ;
    }

    PointsModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
        return _model->_points->at(i).x();
    }

    inline double x() const override
    {
        return _model->_points->at(i).x();
    }

    inline double y() const override
    {
        return _model->_points->at(i).y();
    }

  private:

    int i;
    const PointsModel* _model;
};


#endif // CSVMODEL_H
