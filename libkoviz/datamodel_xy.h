#ifndef XY_MODEL_H
#define XY_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QElapsedTimer>
#include <stdexcept>

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"
#include "mapvalue.h"

class XYModel;
class XYModelIterator;

class XYModel : public DataModel
{
  Q_OBJECT

  friend class XYModelIterator;

  public:

    explicit XYModel(const QStringList &timeNames,
                     double timeMatchTolerance,
                     const QHash<QString,QStringList>& varMap,
                     DataModel *xModel, const QString &xName,
                     DataModel *yModel, const QString &yName,
                     QObject *parent = 0);
    ~XYModel();

    virtual const Parameter* param(int col) const ;
    virtual void map();
    virtual void unmap();
    virtual int paramColumn(const QString& paramName) const ;
    virtual ModelIterator* begin(int tcol, int xcol, int ycol) const ;
    int indexAtTime(double time);

    virtual int rowCount(const QModelIndex & pidx = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex & pidx = QModelIndex() ) const;
    virtual QVariant data (const QModelIndex & index,
                           int role = Qt::DisplayRole ) const;

  private:

    QStringList _timeNames;
    double _timeMatchTolerance;
    const QHash<QString,QStringList> _varMap;
    DataModel* _xModel;
    const QString _xName;
    DataModel* _yModel;
    const QString _yName;
    const QString _xTimeName;
    const QString _yTimeName;
    Parameter* _tParam;
    Parameter* _xParam;
    Parameter* _yParam;

    int _nrows;
    int _ncols;

    int _timeCol;

    XYModelIterator* _iteratorTimeIndex;

    double* _data;

    void _init();
    int _idxAtTimeBinarySearch (XYModelIterator *it,
                               int low, int high, double time);
};

class XYModelIterator : public ModelIterator
{
  public:

    inline XYModelIterator(): i(0) {}

    inline XYModelIterator(int row, // iterator pos
                            const XYModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~XYModelIterator() {}

    virtual void start()
    {
        i = 0;
    }

    virtual void next()
    {
        ++i;
    }

    virtual bool isDone() const
    {
        return ( i >= _model->rowCount() ) ;
    }

    virtual XYModelIterator* at(int n)
    {
        i = n;
        return this;
    }

    inline double t() const
    {
        return _model->_data[i*_model->_ncols+_tcol];
    }

    inline double x() const
    {
        return _model->_data[i*_model->_ncols+_xcol];
    }

    inline double y() const
    {
        return _model->_data[i*_model->_ncols+_ycol];
    }

  private:

    int i;
    const XYModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // XYMODEL_H
