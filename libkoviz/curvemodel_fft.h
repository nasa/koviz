#ifndef CURVE_MODEL_FFT_H
#define CURVE_MODEL_FFT_H

#include <QAbstractTableModel>
#include <QString>
#include <QtMath>
#include "parameter.h"
#include "datamodel.h"
#include "curvemodelparameter.h"
#include "curvemodel.h"
#include "fft.h"

class CurveModelFFT;
class FFTModelIterator;

class CurveModelFFT : public CurveModel
{
  Q_OBJECT

  friend class FFTModelIterator;

  public:

    explicit CurveModelFFT(CurveModel* curveModel, double xb, double xs,
                           double begX, double endX);

    ~CurveModelFFT();

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
                   int role = Qt::DisplayRole ) const override;

  private:

    QString _fileName;
    double _begX;
    double _endX;
    double _xb;
    double _xs;
    double* _data;
    int _ncols;
    int _nrows;
    int _tcol;
    int _xcol;
    int _ycol;
    CurveModelParameter* _t;
    CurveModelParameter* _x;
    CurveModelParameter* _y;

    FFTModelIterator* _iteratorTimeIndex;

    void _init(CurveModel *curveModel);
    int _idxAtTimeBinarySearch (FFTModelIterator *it,
                                int low, int high, double time);
};

class FFTModelIterator : public ModelIterator
{
  public:

    inline FFTModelIterator():
        i(0),
        _tcol(0),
        _xcol(1),
        _ycol(2)
    {
    }

    inline FFTModelIterator(const CurveModelFFT* model) :
        i(0),
        _tcol(0),
        _xcol(1),
        _ycol(2),
        _model(model)
    {
    }

    virtual ~FFTModelIterator() {}

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

    FFTModelIterator* at(int n) override
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
    const CurveModelFFT* _model;
};

#endif // CURVE_MODEL_H
