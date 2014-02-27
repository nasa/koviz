#include "trickmodel.h"
#include <stdio.h>
#include <stdexcept>

TrickModel::TrickModel(const QString& trkfile,
                      const QString& tableName,
                      double startTime, double stopTime, QObject *parent) :
    SnapTable(tableName,parent),
    _trkfile(trkfile),
    _tableName(tableName),
    _startTime(startTime),
    _stopTime(stopTime),
    _nrows(0), _row_size(0), _ncols(0), _pos_beg_data(0),
    _mem(0), _data(0), _fd(-1), _iteratorTimeIndex(0),
    _err_stream(&_err_string)
{
    _load_trick_header();
    map();
    _iteratorTimeIndex = new TrickModelIterator(0,this,0,0,0);
}

bool TrickModel::_load_trick_header()
{
    bool ret = true;

    QFile file(_trkfile);

    if (!file.open(QIODevice::ReadOnly)) {
        _err_stream << "snap [error]: could not open "
                    << _trkfile << "\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }
    QDataStream in(&file);

    //
    // Trick-version-endian (10 bytes)
    //
    char data[1024];
    in.readRawData(data,6); // Trick-
    in.readRawData(data,2); // version
    if ( data[0] == '1' && data[1] == '0' ) {
        _trick_version = TrickVersion10;
    } else if ( data[0] == '0' && data[1] == '7' ) {
        _trick_version = TrickVersion07;
    } else {
        _err_stream << "snap [error]: unrecognized file or Trick version: "
                    << _trkfile << "\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }

    in.readRawData(data,1) ; // -
    in.readRawData(data,1) ; // L or B (endian)
    if ( data[0] == 'L' ) {
        _endianess = LittleEndian;
        in.setByteOrder(QDataStream::LittleEndian);
    } else {
        _endianess = BigEndian;
        in.setByteOrder(QDataStream::BigEndian);
    }

    //
    // Number of parameters (4 byte integer)
    //
    in >> _ncols;

    //
    // Read param header info (name,unit,type,bytesize)
    //
    _row_size = 0 ;
    for ( int cc = 0; cc < _ncols; ++cc) {
        _col_headers.insert(cc,new QVariant);
        Role* role = _createColumnRole();
        _col_roles.insert(cc,role);
        _col2offset[cc] = _row_size;
        _row_size += _load_binary_param(in,cc);
        int paramtype = headerData(cc,Qt::Horizontal,Param::Type).toInt();
        _paramtypes.push_back(paramtype);
    }

    // Sanity check.  The bytes remaining should be a multiple of the record size
    qint64 nbytes = file.bytesAvailable();
    if ( nbytes % _row_size != 0 ) {
        _err_stream << "snap [error]: trk file \""
                    << file.fileName() << "\" is corrupt!\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }

    int maxRows = nbytes/_row_size;

    // Sanity check.  First column must be sys.exec.out.time
    int timeCol = 0 ;
    QString timeName = headerData(timeCol,Qt::Horizontal).toString();
    if ( timeName != "sys.exec.out.time" ) {
        _err_stream << "snap [error]: first element in trk file "
                    << "was not sys.exec.out.time.  Aborting.\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }

    // Save address of begin location of data for map()
    // Also, calc number of rows (timestamped records between start & stop time)
    _pos_beg_data = file.pos();
    _nrows = nbytes/(qint64)_row_size;
    if  (_startTime != 0.0 || _stopTime != 1.0e20 ) {
        bool isStart = false;
        double inputStopTime = _stopTime;
        _nrows = 0 ;
        for ( int i = 0 ; i < maxRows ; ++i ) {
            double timeStamp;
            in >> timeStamp;
            in.skipRawData(_row_size-sizeof(double));
            if ( !isStart && timeStamp >= _startTime-1.0e-9 ) {
                _pos_beg_data = file.pos()-_row_size;
                _startTime = timeStamp;
                isStart = true;
            }
            if ( isStart && timeStamp <= inputStopTime+1.0e-9 ) {
                _nrows++;
                _stopTime = timeStamp;
            } else if ( timeStamp > _stopTime ) {
                if ( isStart && _nrows == 0 ) {
                    _stopTime = timeStamp;
                    _nrows++; // odd case!
                }
                break;
            }
        }

        if ( !isStart ) {
            _err_stream << "snap [error]: startTime of "
                        << _startTime
                        << " specified by user "
                        << "exceeded all timestamps in file:\n    "
                        << _trkfile;
            throw std::range_error(_err_string.toAscii().constData());
        }
    }


    file.close();

    return ret;
}

// Returns byte size of parameter
qint32 TrickModel::_load_binary_param(QDataStream& in, int col)
{
    qint32 sz;

    // Param name
    in >> sz;
    char* param_name = new char[sz+1];
    in.readRawData(param_name,sz);
    param_name[sz] = '\0';
    setHeaderData(col,Qt::Horizontal,param_name,Param::Name);
    setHeaderData(col,Qt::Horizontal,param_name,Qt::EditRole);
    _param2column.insert(QString(param_name),col);
    delete[] param_name;

    // Param unit
    in >> sz;
    char* param_unit = new char[sz+1];
    in.readRawData(param_unit,sz);
    param_unit[sz] = '\0';
    setHeaderData(col,Qt::Horizontal,param_unit,Param::Unit);
    delete[] param_unit;

    // Param type
    qint32 param_type;
    in >> param_type;
    setHeaderData(col,Qt::Horizontal,param_type,Param::Type);

    // Param bytesize
    in >> sz;
    setHeaderData(col,Qt::Horizontal,sz,Param::Size);

    return sz;
}

void TrickModel::map()
{
    if ( _data ) return; // already mapped

    _fd = open(_trkfile.toAscii().constData(), O_RDONLY);
    if ( _fd < 0 ) {
        _err_stream << "snap [error]: TrickModel could not open "
                    << _trkfile << "\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }
    fstat(_fd, &_fstat);

    _mem = (char*)mmap(NULL,_fstat.st_size,PROT_READ,MAP_SHARED,_fd,0);

    if (_mem == MAP_FAILED) {
        _err_stream << "snap [error]: TrickModel couldn't allocate memory for : "
                    << _trkfile << "\n";
        throw std::runtime_error(_err_string.toAscii().constData());
    }

    _data = &_mem[_pos_beg_data];
}

void TrickModel::unmap()
{
    munmap(_mem, _fstat.st_size);
    close(_fd);
    _data = 0 ;
}

TrickModel::~TrickModel()
{
    delete _iteratorTimeIndex;
    unmap();
}

TrickModelIterator TrickModel::begin(int tcol, int xcol, int ycol) const
{
    return TrickModelIterator(0,this,tcol,xcol,ycol);
}

TrickModelIterator TrickModel::end(int tcol, int xcol, int ycol) const
{
    return TrickModelIterator(this->rowCount(),this,tcol,xcol,ycol);
}

int TrickModel::indexAtTime(double time, int tcol)
{
    return _idxAtTimeBinarySearch(*_iteratorTimeIndex,0,rowCount()-1,time);
}


int TrickModel::_idxAtTimeBinarySearch (TrickModelIterator& it,
                                       int low, int high, double time)
{
        if (high <= 0 ) {
                return 0;
        }
        if (low >= high) {
                return ( it[high].t() > time ) ? high-1 : high;
        } else {
                int mid = (low + high)/2;
                if (time == it[mid].t()) {
                        return mid;
                } else if ( time < it[mid].t() ) {
                        return _idxAtTimeBinarySearch(it,
                                                      low, mid-1, time);
                } else {
                        return _idxAtTimeBinarySearch(it,
                                                      mid+1, high, time);
                }
        }
}

int TrickModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int TrickModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}


// TODO: byteswap like
//      dp = (double *)(address) ;
//      value = _swap ? trick_byteswap_double(*dp) : *dp ;
QVariant TrickModel::data(const QModelIndex &idx, int role) const
{
    QVariant val;

    if ( idx.isValid() ) {
        int row = idx.row();
        int col = idx.column();

        if ( role == Qt::DisplayRole ) {
            qint64 _pos_data = row*_row_size + _col2offset.value(col);
            const char* addr = &(_data[_pos_data]);
            int paramtype =  _paramtypes.at(col);
            val = _toDouble(addr,paramtype);
        }
    }

    return val;
}

// mmap is read only for now
bool TrickModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    Q_UNUSED(idx);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

// mmap is read only for now
bool TrickModel::insertRows(int row, int count, const QModelIndex &pidx)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(pidx);
    return false;
}

// mmap is read only for now
bool TrickModel::removeRows(int row, int count, const QModelIndex &pidx)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(pidx);
    return false;
}

// mmap is read only for now
bool TrickModel::insertColumns(int column, int count,
                                   const QModelIndex &pidx)
{
    Q_UNUSED(column);
    Q_UNUSED(count);
    Q_UNUSED(pidx);
    return false;
}

// mmap is read only for now
bool TrickModel::removeColumns(int column, int count, const QModelIndex &pidx)
{
    Q_UNUSED(column);
    Q_UNUSED(count);
    Q_UNUSED(pidx);
    return false;
}

Role* TrickModel::_createColumnRole()
{
    Param* param = new Param;
    return param;
}
