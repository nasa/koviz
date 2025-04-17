#ifndef MOT_MODEL_H
#define MOT_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <stdexcept>

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

class MotModel;
class MotModelIterator;

class MotModel : public DataModel
{
  Q_OBJECT

  friend class MotModelIterator;

  public:

    explicit MotModel(const QStringList &timeNames,
                      const QString &runPath,
                      const QString &motfile,
                      QObject *parent = 0);
    ~MotModel();

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

    static bool isValid( const QString& motFile ) ;

  private:

    QStringList _timeNames;
    QString _motfile;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    MotModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (MotModelIterator *it,
                               int low, int high, double time);

    inline double _convert(const QString& s);
};

class MotModelIterator : public ModelIterator
{
  public:

    inline MotModelIterator(): i(0) {}

    inline MotModelIterator(int row, // iterator pos
                            const MotModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~MotModelIterator() {}

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

    MotModelIterator* at(int n) override
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
    const MotModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // MOTMODEL_H
