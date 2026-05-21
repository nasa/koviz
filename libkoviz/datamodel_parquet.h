#ifndef PARQUET_MODEL_H
#define PARQUET_MODEL_H

#include <QHash>
#include <QFile>
#include <QString>
#include <QStringList>

#include "datamodel.h"
#include "parameter.h"

// This prevents collision with Parquet signal
#undef signals
#undef slots

#include <memory>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/file_reader.h>

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
    ~ParquetModel();

    static const QString TimeName;

    static bool isValid( const QString& parquetFile,
                         const QStringList& timeNames);

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

    QStringList _timeNames;
    QString _parquetFile;

    QHash<int,Parameter*> _col2param;   // ordered by column
    QHash<QString,int> _param2column;

    qint64 _nrows;
    qint32 _ncols;
    qint32 _timeCol;

    std::unique_ptr<parquet::arrow::FileReader> _reader;
    std::shared_ptr<arrow::io::ReadableFile> _infile;
    mutable QHash<int,std::shared_ptr<arrow::DoubleArray>> _col2array;
    std::shared_ptr<arrow::DoubleArray> _loadColumn(int col) const;

    ParquetModelIterator* _iteratorTimeIndex;

    int _idxAtTimeBinarySearch(ParquetModelIterator *it,
                               int low, int high, double time);
    void _init();
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
        _t = model->_loadColumn(tcol);
        _x = model->_loadColumn(xcol);
        _y = model->_loadColumn(ycol);
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
        return _t->Value(i);
    }

    inline double x() const override
    {
        return _x->Value(i);
    }

    inline double y() const override
    {
        return _y->Value(i);
    }

  private:

    qint64 i;
    int _row_count;
    std::shared_ptr<arrow::DoubleArray> _t;
    std::shared_ptr<arrow::DoubleArray> _x;
    std::shared_ptr<arrow::DoubleArray> _y;
};


#endif
