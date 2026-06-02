#ifndef PARQUET_MODEL_H
#define PARQUET_MODEL_H

// A Parquet model can contain a single time history or can house
// a monte carlo run set of time histories.  In the monte carlo case:
//    1. The commandline option -runColumnName is used
//    2. The constructor with runID is used by Runs class

#include <QHash>
#include <QVector>
#include <QFile>
#include <QString>
#include <QStringList>

#include <limits>

#include "datamodel.h"
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

class ParquetModel;
class ParquetModelIterator;

class ParquetModel : public DataModel
{
  Q_OBJECT

  friend class ParquetModelIterator;

  public:

    explicit ParquetModel(const QStringList &timeNames,
                          const QString &runPath,
                          const QString &parquetFile,
                          QObject *parent = 0);

    // Monte carlo data model subset for runID
    explicit ParquetModel(const QStringList &timeNames,
                          const QStringList& runColumnNames,
                          const QString &runPath,
                          const QString &parquetFile,
                          QObject *parent = 0);
    ~ParquetModel();


    const QVector<int> &runRows(int runID) const;

    static const QString TimeName;

    static bool isValid( const QString& parquetFile,
                         const QStringList& timeNames);
    static bool isMonte( const QString& parquetFile,
                         const QStringList& timeNames,
                         const QStringList& runColumnNames);

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

#if HAS_PARQUET
    std::shared_ptr<arrow::DoubleArray> loadColumn(int col) const;
#endif

  private:

    QStringList _timeNames;
    QStringList _runColumnNames;
    QString _parquetFile;

    QHash<int,Parameter*> _col2param;   // ordered by column
    QHash<QString,int> _param2column;

    qint64 _nrows;
    qint32 _ncols;
    qint32 _timeCol;
    qint32 _runCol;

    ParquetModelIterator* _iteratorTimeIndex;

    QHash<int, QVector<int>> _runID2rows;

    bool _isMonte;

    int _idxAtTimeBinarySearch(ParquetModelIterator *it,
                               int low, int high, double time);
    void _init();

    void _open_parquet_reader();
    void _close_parquet_reader();

#if HAS_PARQUET
    template <typename ArrowArrayType>
    std::shared_ptr<arrow::DoubleArray>
    _convertNumericArray(const std::shared_ptr<arrow::Array>& combined) const;

    std::unique_ptr<parquet::arrow::FileReader> _reader;
    std::shared_ptr<arrow::io::ReadableFile> _infile;
    mutable QHash<int,std::shared_ptr<arrow::DoubleArray>> _col2array;
    void _unmap();
#endif

};

//
// Iterates over rows for curve data triple - (time,x,y)
//
class ParquetModelIterator : public ModelIterator
{
  public:

    inline ParquetModelIterator(): i(0) {}

    inline ParquetModelIterator(int row, // iterator pos
                              const ParquetModel* model,
                              int tcol, int xcol, int ycol):
        i(row),
        _row_count(model->rowCount())
    {
#ifdef HAS_PARQUET
        _t = model->loadColumn(tcol);
        _x = model->loadColumn(xcol);
        _y = model->loadColumn(ycol);
#else
        Q_UNUSED(tcol);
        Q_UNUSED(xcol);
        Q_UNUSED(ycol);
#endif
    }

    virtual ~ParquetModelIterator() {}

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

    ParquetModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
#ifdef HAS_PARQUET
        return _t->Value(i);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

    inline double x() const override
    {
#ifdef HAS_PARQUET
        return _x->Value(i);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

    inline double y() const override
    {
#ifdef HAS_PARQUET
        return _y->Value(i);
#else
        return std::numeric_limits<double>::quiet_NaN();
#endif
    }

  private:

    qint64 i;
    int _row_count;
#ifdef HAS_PARQUET
    std::shared_ptr<arrow::DoubleArray> _t;
    std::shared_ptr<arrow::DoubleArray> _x;
    std::shared_ptr<arrow::DoubleArray> _y;
#endif
};


#if HAS_PARQUET
template <typename ArrowArrayType>
std::shared_ptr<arrow::DoubleArray>
ParquetModel::_convertNumericArray(
    const std::shared_ptr<arrow::Array>& combined) const
{
    auto src = std::static_pointer_cast<ArrowArrayType>(combined);

    arrow::DoubleBuilder builder;

    for (int64_t i = 0; i < src->length(); i++) {
        arrow::Status st;
        if (src->IsNull(i)) {
            st = builder.AppendNull();
        } else {
            st = builder.Append(static_cast<double>(src->Value(i)));
        }
        if ( !st.ok() ) {
            return nullptr;
        }
    }

    std::shared_ptr<arrow::DoubleArray> out;

    auto st = builder.Finish(&out);

    if (!st.ok()) {
        return nullptr;
    }

    return out;
}
#endif

#endif
