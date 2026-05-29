#ifndef CSV_RUN_MODEL_H
#define CSV_RUN_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QApplication>
#include <stdexcept>

#include "datamodel.h"
#include "datamodel_csv.h"
#include "parameter.h"
#include "unit.h"

class CsvRunModel;
class CsvRunModelIterator;

class CsvRunModel : public DataModel
{
  Q_OBJECT

  friend class CsvRunModelIterator;

  public:

    explicit CsvRunModel(const CsvModel* csvModel,
                         const QStringList &timeNames,
                         const QString &runPath,
                         const QString &csvfile,
                         const QStringList &runColumnNames,
                         int runID,
                         QObject *parent = 0);
    ~CsvRunModel();

    static const QString TimeName;
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

    static bool isValid( const QString& csvFile,
                         const QStringList& timeNames,
                         const QStringList& runColumnNames);

  private:

    const CsvModel* _csvModel;
    QStringList _timeNames;
    QString _csvfile;
    QStringList _runColumnNames;
    int _runID;

    int _nrows;
    int _ncols;
    int _timeCol;

    QList<int> _runRows;  // Say it like Jetson's Astro, Run Row!

    CsvRunModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (CsvRunModelIterator *it,
                               int low, int high, double time);

    QHash<QString,int> _str2id;
};

class CsvRunModelIterator : public ModelIterator
{
  public:

    inline CsvRunModelIterator(): i(0) {}

    inline CsvRunModelIterator(int row, // iterator pos
                               const CsvRunModel* model,
                               int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
        _data = _model->_csvModel->dataPtr();
        _ncols = model->_csvModel->columnCount();
    }

    virtual ~CsvRunModelIterator() {}

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

    CsvRunModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
        int row = _model->_runRows[i];
        return _data[row*_ncols+_tcol];
    }

    inline double x() const override
    {
        int row = _model->_runRows[i];
        return _data[row*_ncols+_xcol];
    }

    inline double y() const override
    {
        int row = _model->_runRows[i];
        return _data[row*_ncols+_ycol];
    }

  private:

    int i;
    const CsvRunModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
    int _ncols;
    const double* _data;
};


#endif // CSV_RUN_MODEL_H
