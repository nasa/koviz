#ifndef PROGRAM_MODEL_H
#define PROGRAM_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QLibrary>
#include <stdexcept>
#include <math.h>

#include "datamodel.h"
#include "parameter.h"
#include "curvemodel.h"
#include "unit.h"

class ProgramModel;
class ProgramModelIterator;

typedef int (*ExternalProgram)(double*,int,double*,int);

class ProgramModel : public DataModel
{
  Q_OBJECT

  friend class ProgramModelIterator;

  public:

    explicit ProgramModel(const QList<CurveModel*>& inputCurves,
                          const QList<Parameter>& inputParams,
                          const QStringList& outputNames,
                          const QStringList &timeNames,
                          const QString &programfile,
                          QObject *parent = 0);
    ~ProgramModel();

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

    QStringList _timeNames;
    QString _programfile;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    ProgramModelIterator* _iteratorTimeIndex;

    QList<double> _timeStamps;

    double* _data;

    QLibrary* _library;
    ExternalProgram _program;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init(const QList<CurveModel *> &inputCurves,
               const QList<Parameter> &inputParams,
               const QStringList &outputNames);
    int _idxAtTimeBinarySearch (ProgramModelIterator *it,
                               int low, int high, double time);
};

class ProgramModelIterator : public ModelIterator
{
  public:

    inline ProgramModelIterator(): i(0) {}

    inline ProgramModelIterator(int row, // iterator pos
                            const ProgramModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~ProgramModelIterator() {}

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

    ProgramModelIterator* at(int n) override
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
    const ProgramModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // PROGRAMMODEL_H
