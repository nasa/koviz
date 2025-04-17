#ifndef CURVE_MODEL_PAINTERPATH_H
#define CURVE_MODEL_PAINTERPATH_H

#include <QAbstractTableModel>
#include <QString>
#include <QtMath>
#include <QPainterPath>
#include "parameter.h"
#include "datamodel.h"
#include "curvemodelparameter.h"
#include "curvemodel.h"

class PainterPathModelIterator;

class CurveModelPainterPath : public CurveModel
{
  Q_OBJECT

  friend class PainterPathModelIterator;

  public:

    CurveModelPainterPath(QPainterPath* painterPath,
                          const QString& xName, const QString& xUnit,
                          const QString& yName, const QString& yUnit);

    ~CurveModelPainterPath();

    CurveModelParameter* t() override { return _t; }
    CurveModelParameter* x() override { return _x; }
    CurveModelParameter* y() override { return _y; }

    QString fileName() const override { return ""; }

    void map() override {}
    void unmap() override {}
    ModelIterator* begin() const override ;
    int indexAtTime(double time) override;

    int rowCount(const QModelIndex & pidx = QModelIndex() ) const override;
    int columnCount(const QModelIndex & pidx = QModelIndex() ) const override;
    QVariant data (const QModelIndex & index,
                   int role = Qt::DisplayRole ) const override;

  private:

    QPainterPath* _painterPath;
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
    PainterPathModelIterator* _iteratorTimeIndex;

    int _idxAtTimeBinarySearch (PainterPathModelIterator* it,
                                int low, int high, double time);
};

class PainterPathModelIterator : public ModelIterator
{
  public:

    inline PainterPathModelIterator(): i(0) {}

    inline PainterPathModelIterator(const CurveModelPainterPath* model) :
        i(0),
        _model(model)
    {
    }

    virtual ~PainterPathModelIterator() {}

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

    PainterPathModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
        QPainterPath::Element el = _model->_painterPath->elementAt(i);
        return el.x;
    }

    inline double x() const override
    {
        QPainterPath::Element el = _model->_painterPath->elementAt(i);
        return el.x;
    }

    inline double y() const override
    {
        QPainterPath::Element el = _model->_painterPath->elementAt(i);
        return el.y;
    }

  private:

    int i;
    const CurveModelPainterPath* _model;
};

#endif // CURVE_MODEL_PainterPath_H
