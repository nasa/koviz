#ifndef PARQUET_RUN_MODEL_H
#define PARQUET_RUN_MODEL_H

// A Parquet model can contain a single time history or can house
// a monte carlo run set of time histories.  In the monte carlo case,
// this class is used to take a "run slice" of the Parquet Model
// for given runID.  The commandline option -runColumnName is used
// to identify the run column.

#include <QHash>
#include <QVector>
#include <QFile>
#include <QString>
#include <QStringList>

#include "datamodel.h"
#include "datamodel_parquet.h"
#include "parameter.h"

// This prevents collision with Parquet signal
#undef signals
#undef slots

#ifdef HAS_PARQUET
#include <memory>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/file_reader.h>
#endif

// Redefine signals/slots after including Parquet
#define signals Q_SIGNALS
#define slots Q_SLOTS

class ParquetRunModel;
class ParquetRunModelIterator;

class ParquetRunModel : public DataModel
{
  Q_OBJECT

  friend class ParquetRunModelIterator;

  public:

    // Monte carlo data model subset for runID
    explicit ParquetRunModel(ParquetModel* parquetModel,
                             const QStringList &timeNames,
                             const QStringList& runColumnNames,
                             int64_t runID,
                             const QString &runPath,
                             const QString &parquetFile,
                             QObject *parent = 0);
    ~ParquetRunModel();

    static const QString TimeName;

    const Parameter* param(int col) const override ;

    void map() override;
    void unmap() override;
    int paramColumn(const QString& param) const override;
    ModelIterator* begin(int tcol, int xcol, int ycol) const override ;
    int indexAtTime(double time) override;
    int rowCount(const QModelIndex & pidx = QModelIndex() ) const override;
    int columnCount(const QModelIndex & pidx = QModelIndex() ) const override;
    QVariant data (const QModelIndex & index,
                   int role = Qt::DisplayRole ) const override;

  private:

    ParquetModel* _parquetModel;
    QStringList _timeNames;
    QStringList _runColumnNames;
    int64_t _runID;
    QString _parquetFile;

    QHash<int,Parameter*> _col2param;   // ordered by column
    QHash<QString,int> _param2column;

    qint64 _nrows;
    qint32 _ncols;
    qint32 _timeCol;
    qint32 _runCol;

    ParquetRunModelIterator* _iteratorTimeIndex;

    int _idxAtTimeBinarySearch(ParquetRunModelIterator *it,
                               int low, int high, double time);
    void _init();
};

//
// Iterates over rows for curve data triple - (time,x,y)
//
class ParquetRunModelIterator : public ModelIterator
{
  public:

    inline ParquetRunModelIterator(): i(0) {}

    inline ParquetRunModelIterator(int row, // iterator pos
                              const ParquetRunModel* model,
                              int tcol, int xcol, int ycol):
        i(row),
        _row_count(model->rowCount())
    {
#ifdef HAS_PARQUET
        int runID = model->_runID;
        _runRows = &model->_parquetModel->runRows(runID);
        _t = model->_parquetModel->loadColumn(tcol);
        _x = model->_parquetModel->loadColumn(xcol);
        _y = model->_parquetModel->loadColumn(ycol);
#else
        Q_UNUSED(tcol);
        Q_UNUSED(xcol);
        Q_UNUSED(ycol);
#endif
    }

    virtual ~ParquetRunModelIterator() {}

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
        return ( i >= _row_count ) ;
    }

    ParquetRunModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
#ifdef HAS_PARQUET
        int row = (*_runRows)[i];
        return _t->Value(row);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

    inline double x() const override
    {
#ifdef HAS_PARQUET
        int row = (*_runRows)[i];
        return _x->Value(row);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

    inline double y() const override
    {
#ifdef HAS_PARQUET
        int row = (*_runRows)[i];
        return _y->Value(row);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

  private:

    qint64 i;
    int _row_count;
    const QVector<int>* _runRows;
#ifdef HAS_PARQUET
    std::shared_ptr<arrow::DoubleArray> _t;
    std::shared_ptr<arrow::DoubleArray> _x;
    std::shared_ptr<arrow::DoubleArray> _y;
#endif
};

#endif
