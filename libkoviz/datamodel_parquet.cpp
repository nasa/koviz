#include "datamodel_parquet.h"

const QString ParquetModel::TimeName = QString(""); // No standard time name

ParquetModel::ParquetModel(const QStringList& timeNames,
                           const QString &runPath,
                           const QString& parquetFile, QObject *parent) :
    DataModel(timeNames, runPath, parquetFile, parent),
    _timeNames(timeNames),
    _parquetFile(parquetFile),
    _nrows(0), _ncols(0), _timeCol(0),_runCol(-1),_iteratorTimeIndex(0),
    _isMonte(false)
{
    _init();
}

ParquetModel::ParquetModel(const QStringList& timeNames,
                           const QStringList& runColumnNames,
                           const QString &runPath,
                           const QString& parquetFile, QObject *parent) :
    DataModel(timeNames, runPath, parquetFile, parent),
    _timeNames(timeNames),_runColumnNames(runColumnNames),
    _parquetFile(parquetFile),
    _nrows(0), _ncols(0), _timeCol(0), _iteratorTimeIndex(0)
{
    _init();
}

void ParquetModel::_init()
{
#ifdef HAS_PARQUET
    std::shared_ptr<parquet::ParquetFileReader> reader;

    reader = parquet::ParquetFileReader::OpenFile(
        _parquetFile.toStdString(),
        false // no memory map needed
    );

    auto metadata = reader->metadata();

    _ncols = metadata->num_columns();
    _nrows = metadata->num_rows();

    // Get units in metadata
    QHash<QString,QString> colName2Unit;
    auto unitMetaData = parquet::ParquetFileReader::OpenFile(
                        _parquetFile.toStdString(),
                        false)->metadata();
    auto kv = unitMetaData->key_value_metadata();
    if (kv) {
        for ( int i = 0; i < kv->size(); ++i ) {
            QString key(kv->key(i).c_str());
            if ( key == "ARROW:schema" ) continue;
            QString unit(kv->value(i).c_str());
            colName2Unit.insert(key,unit);
        }
    }

    auto schema = metadata->schema();
    for (int col = 0; col < _ncols; ++col) {
        std::string colName = schema->Column(col)->name();
        QString qname = QString::fromStdString(colName);
        if ( _timeNames.contains(qname) ) {
            _timeCol = col;
        }
        if ( _runColumnNames.contains(qname) ) {
            _runCol = col;
        }
        Parameter* p = new Parameter(qname,colName2Unit.value(qname,"--"));
        _col2param[col] = p;
        _param2column[qname] = col;
    }


    // If this is a monte carlo in a single file, the parquet file will have a
    // column for runs. This block makes a hash of runIDs -> list of rows.
    // For example, runID == 77 might be on rows 238-301
    // Do this on one pass and give to clients so they don't have to repeat
    if ( _runCol >= 0 ) {
        _isMonte = true;
        _open_parquet_reader();  // For speed, leave parquet reader open for MC
        auto arr = loadColumn(_runCol);
        int row = 0;
        for (int64_t i = 0; i < arr->length(); ++i) {
            if (!arr->IsNull(i)) {
                int64_t runID = arr->Value(i);
                _runID2rows[runID].append(row);
            }
            ++row;
        }
    }
#endif
}

void ParquetModel::_open_parquet_reader()
{
#ifdef HAS_PARQUET
    if (_reader) return;

    auto infile_result = arrow::io::ReadableFile::Open(
                                                    _parquetFile.toStdString());
    if (!infile_result.ok())
    {
        fprintf(stderr, "koviz [error]: Parquet::map() could not "
                "open file=%s\n", _parquetFile.toLatin1().constData());
        exit(-1);
    }

    _infile = std::move(infile_result).ValueOrDie();

    auto reader_result = parquet::arrow::OpenFile(_infile,
                                                  arrow::default_memory_pool());

    if (!reader_result.ok()) {
        fprintf(stderr, "koviz [error]: Parquet::map() could not "
                        "open file=%s\n", _parquetFile.toLatin1().constData());
        _infile.reset();
        exit(-1);
    }

    _reader = std::move(reader_result).ValueOrDie();
#endif
}

void ParquetModel::_close_parquet_reader()
{
#ifdef HAS_PARQUET
    _reader.reset();
    _infile.reset();
#endif
}

void ParquetModel::map()
{
#ifdef HAS_PARQUET
    if ( !_isMonte ) {
        // If MC not all in one file, there could be many runs so need to
        // map/unmap to keep Parquet from having too many file descriptors open
        _open_parquet_reader();
    }
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
    }
    _iteratorTimeIndex = new ParquetModelIterator(0,this,
                                                 _timeCol,_timeCol,_timeCol);
#endif
}

void ParquetModel::unmap()
{
#ifdef HAS_PARQUET
    _unmap();
#endif
}

void ParquetModel::_unmap()
{
#ifdef HAS_PARQUET
    if ( !_isMonte ) {
        _close_parquet_reader();
    }
    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
#endif
}


int ParquetModel::paramColumn(const QString &param) const
{
    return _param2column.value(param,-1);
}

ModelIterator *ParquetModel::begin(int tcol, int xcol, int ycol) const
{
    return new ParquetModelIterator(0,this,tcol,xcol,ycol);
}

ParquetModel::~ParquetModel()
{
    _unmap();
    foreach ( Parameter* param, _col2param.values() ) {
        delete param;
    }
}

// This returns a reference to vector instead of a copy
const QVector<int>& ParquetModel::runRows(int runID) const
{
    auto it = _runID2rows.find(runID);
    if (it == _runID2rows.end()) {
        static const QVector<int> empty;
        return empty;
    }
    return it.value();
}

bool ParquetModel::isValid(const QString& parquetFile,
                          const QStringList& timeNames)
{
#ifdef HAS_PARQUET
    try {
        auto reader = parquet::ParquetFileReader::OpenFile(
            parquetFile.toStdString(),
            false // no mmap needed
        );

        auto metadata = reader->metadata();
        auto schema = metadata->schema();

        int ncols = metadata->num_columns();

        for (int col = 0; col < ncols; ++col) {

            QString colName =
                            QString::fromStdString(schema->Column(col)->name());

            if (timeNames.contains(colName)) {
                return true;
            }
        }

        return false;

    } catch (const std::exception& e) {
        return false;
    }

#else

    Q_UNUSED(parquetFile);
    Q_UNUSED(timeNames);

    return false;
#endif
}

bool ParquetModel::isMonte(const QString &parquetFile,
                           const QStringList &timeNames,
                           const QStringList &runColumnNames)
{
#ifdef HAS_PARQUET
    if ( runColumnNames.isEmpty() ) {
        return false;
    }
    if ( !ParquetModel::isValid(parquetFile,timeNames) ) {
        return false;
    }

    try {
        auto reader = parquet::ParquetFileReader::OpenFile(
                                                      parquetFile.toStdString(),
                                                      false);
        auto metadata = reader->metadata();
        auto schema   = metadata->schema();

        int ncols = metadata->num_columns();

        for (int col = 0; col < ncols; ++col) {
            QString colName = QString::fromStdString(
                                                   schema->Column(col)->name());
            if (runColumnNames.contains(colName)) {
                return true;
            }
        }
    } catch (...) {
        return false;
    }

#else
    Q_UNUSED(parquetFile);
    Q_UNUSED(timeNames);
    Q_UNUSED(runColumnNames);
#endif

    return false;
}

const Parameter* ParquetModel::param(int col) const
{
    return _col2param.value(col);
}

int ParquetModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int ParquetModel::_idxAtTimeBinarySearch (ParquetModelIterator* it,
                                       int low, int high, double time)
{
        if (high <= 0 ) {
                return 0;
        }
        if (low >= high) {
                // Time not found, choose closest near high
                double t1 = it->at(high-1)->t();
                double t2 = it->at(high)->t();
                double t3 = t2;
                it = it->at(high+1);
                if ( !it->isDone() ) {
                    t3 = it->at(high+1)->t();
                }

                int i;
                if ( qAbs(time-t1) < qAbs(time-t2) ) {
                    if ( qAbs(time-t1) < qAbs(time-t3) ) {
                        i = high-1;
                    } else {
                        if ( !it->isDone() ) {
                            i = high+1;
                        } else {
                            i = high;
                        }
                    }
                } else {
                    if ( qAbs(time-t2) < qAbs(time-t3) ) {
                        i = high;
                    } else {
                        if ( !it->isDone() ) {
                            i = high+1;
                        } else {
                            i = high;
                        }
                    }
                }
                return i;
        } else {
                int mid = (low + high)/2;
                if (time == it->at(mid)->t()) {
                        return mid;
                } else if ( time < it->at(mid)->t() ) {
                        return _idxAtTimeBinarySearch(it,
                                                      low, mid-1, time);
                } else {
                        return _idxAtTimeBinarySearch(it,
                                                      mid+1, high, time);
                }
        }
}

int ParquetModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int ParquetModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}

QVariant ParquetModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(role);
    Q_UNUSED(idx);

    fprintf(stderr, "koviz [error]: Using unimplemented ParquetModel::data()."
            "Data should be retrieved via ParquetModelIterators\n");

    return std::numeric_limits<double>::quiet_NaN();
}

#ifdef HAS_PARQUET
std::shared_ptr<arrow::DoubleArray> ParquetModel::loadColumn(int col) const
{
    if (_col2array.contains(col)) {
        return _col2array[col];
    }

    std::shared_ptr<arrow::ChunkedArray> chunked;

    auto status = _reader->ReadColumn(col, &chunked);

    if (!status.ok()) {
        fprintf(stderr, "koviz [error]: ParquetModel::loadColumn failed "
                "to read column=%d msg=%s\n",col,status.ToString().c_str());
        return nullptr;
    }

    //
    // Flatten chunked array into one contiguous array
    //
    auto result = arrow::Concatenate(chunked->chunks(),
                                     arrow::default_memory_pool());
    if (!result.ok()) {
        fprintf(stderr, "koviz [error]: ParquetModel::loadColumn concat "\
                        "failed. msg=%s\n",result.status().ToString().c_str());
        return nullptr;
    }
    auto combined = std::move(result).ValueOrDie();
    if (combined->type_id() == arrow::Type::DOUBLE) {
        auto arr = std::static_pointer_cast<arrow::DoubleArray>(combined);
        _col2array[col] = arr;
    } else {

    }

    std::shared_ptr<arrow::DoubleArray> arr;

    switch (combined->type_id()) {
    case arrow::Type::DOUBLE: {
        arr = std::static_pointer_cast<arrow::DoubleArray>(combined);
        break;
    }
    case arrow::Type::FLOAT:
    {
        arr = _convertNumericArray<arrow::FloatArray>(combined);
        break;
    }
    case arrow::Type::INT32:
    {
        arr = _convertNumericArray<arrow::Int32Array>(combined);
        break;
    }
    case arrow::Type::INT64:
    {
        arr = _convertNumericArray<arrow::Int64Array>(combined);
        break;
    }
    case arrow::Type::UINT32:
    {
        arr = _convertNumericArray<arrow::UInt32Array>(combined);
        break;
    }
    case arrow::Type::BOOL:
    {
        arr = _convertNumericArray<arrow::BooleanArray>(combined);
        break;
    }
    default:
        arrow::DoubleBuilder builder;
        for (int64_t i = 0; i < combined->length(); i++) {
            auto st = builder.Append(std::numeric_limits<double>::quiet_NaN());
            if ( !st.ok() ) {
                fprintf(stderr, "koviz [error]: ParquetModel::loadColumn()!"
                        "  Unsupported type and trouble with loading NaNs\n");
                exit(-1);
            }
        }

        auto st = builder.Finish(&arr);

        if (!st.ok()) {
            fprintf(stderr,
                "koviz [error]: ParquetModel::loadColumn() failed to build "\
                "NaN fallback column col=%d\n", col);
            exit(-1);
        }
    }

    _col2array[col] = arr;

    return arr;
}
#endif
