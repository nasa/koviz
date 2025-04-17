#ifndef CURVE_MODEL_INTEG_H
#define CURVE_MODEL_INTEG_H

#include <QAbstractTableModel>
#include <QString>
#include <QtMath>
#include <cmath>
#include "parameter.h"
#include "datamodel.h"
#include "curvemodelparameter.h"
#include "curvemodel.h"
#include "unit.h"

class CurveModelIntegral;
class IntegralModelIterator;

class CurveModelIntegral : public CurveModel
{
  Q_OBJECT

  friend class IntegralModelIterator;

  public:

    explicit CurveModelIntegral(CurveModel* curveModel, double initial_value);

    ~CurveModelIntegral();

    CurveModelParameter* t() override { return _t; }
    CurveModelParameter* x() override { return _x; }
    CurveModelParameter* y() override { return _y; }

    QString fileName() const override { return _fileName; }

    void map() override {}
    void unmap() override {}
    ModelIterator* begin() const override ;
    int indexAtTime(double time) override;

    int rowCount(const QModelIndex & pidx = QModelIndex() ) const override;
    int columnCount(const QModelIndex & pidx = QModelIndex() ) const override;
    QVariant data (const QModelIndex & index,
                   int role = Qt::DisplayRole ) const override ;

  private:

    QString _fileName;
    double* _data;
    int _ncols;
    int _nrows;
    int _tcol;
    int _xcol;
    int _ycol;
    CurveModelParameter* _t;
    CurveModelParameter* _x;
    CurveModelParameter* _y;

    IntegralModelIterator* _iteratorTimeIndex;

    void _init(CurveModel *curveModel, double initial_value);
    int _idxAtTimeBinarySearch (IntegralModelIterator *it,
                                int low, int high, double time);
};

class IntegralModelIterator : public ModelIterator
{
  public:

    inline IntegralModelIterator():
        i(0),
        _tcol(0),
        _xcol(1),
        _ycol(2)
    {
    }

    inline IntegralModelIterator(const CurveModelIntegral* model) :
        i(0),
        _tcol(0),
        _xcol(1),
        _ycol(2),
        _model(model)
    {
    }

    virtual ~IntegralModelIterator() {}

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

    IntegralModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
        return _model->_data[i*_model->_ncols+_tcol];
    }

    inline double x() const override
    {
        return _model->_data[i*_model->_ncols+_xcol];
    }

    inline double y() const override
    {
        return _model->_data[i*_model->_ncols+_ycol];
    }

  private:

    int i;
    int _tcol;
    int _xcol;
    int _ycol;
    const CurveModelIntegral* _model;
};

#endif // CURVE_MODEL_INTEG
