#include "datamodel_csv.h"

QString CsvModel::_err_string;
QTextStream CsvModel::_err_stream(&CsvModel::_err_string);

CsvModel::CsvModel(const QStringList& timeNames,
                   const QString &runPath,
                   const QString& csvfile,
                   QObject *parent) :
    DataModel(timeNames, runPath, csvfile, parent),
    _timeNames(timeNames),_csvfile(csvfile),
    _nrows(0), _ncols(0),_iteratorTimeIndex(0),
    _data(0)
{
    _init();
}

void CsvModel::_init()
{
    QFile file(_csvfile);

    if (!file.open(QIODevice::ReadOnly)) {
        _err_stream << "koviz [error]: could not open "
                    << _csvfile << "\n";
        throw std::runtime_error(_err_string.toLatin1().constData());
    }
    QTextStream in(&file);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
    #else
        in.setCodec("UTF-8");
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const auto SkipEmptyParts = Qt::SkipEmptyParts ;
    #else
        const auto SkipEmptyParts = QString::SkipEmptyParts;
    #endif

    QString line0 = in.readLine();
    QStringList items = line0.split(',',SkipEmptyParts);
    int col = 0;
    foreach ( QString item, items ) {
        QString name;
        QString unit;
        if ( item.contains('{') && item.contains('}') ) {
            // Unit is between curlies
            int i = item.indexOf('{');
            int j = item.indexOf('}');
            if ( i < j ) {
                unit = item.mid(i+1,j-i-1);
                if ( !Unit::isUnit(unit) ) {
                    fprintf(stderr,"koviz [warning]: Unsupported "
                                   "unit=\"%s\" in file=\"%s\"\n",
                                   unit.toLatin1().constData(),
                                   _csvfile.toLatin1().constData());
                    unit = "--";
                }
            }
            // Assume parameter name precedes unit
            if ( i > 0 ) {
                name = item.left(i-1).trimmed();
            }
        } else {
            name = item.trimmed();
            unit = "--";
        }

        Parameter* param = new Parameter;
        param->setName(name);
        param->setUnit(unit);

        _col2param.insert(col,param);
        _paramName2col.insert(name,col);

        ++col;
    }
    _ncols = col;

    // Make sure time param exists in model and set time column
    bool isFoundTime = false;
    foreach (QString timeName, _timeNames) {
        if ( _paramName2col.contains(timeName)) {
            _timeCol = _paramName2col.value(timeName) ;
            isFoundTime = true;
            break;
        }
    }
    if ( ! isFoundTime ) {
        _err_stream << "koviz [error]: couldn't find time param \""
                    << _timeNames.join("=") << "\" in file=" << _csvfile
                    << ".  Try setting -timeName on commandline option.";
        throw std::runtime_error(_err_string.toLatin1().constData());
    }

    _iteratorTimeIndex = new CsvModelIterator(0,this,
                                              _timeCol,_timeCol,_timeCol);

    qint64 fileSize = file.size();
    if (fileSize == 0) {
        delete _iteratorTimeIndex ;
        file.close();
        return;
    }
    const char* data = reinterpret_cast<const char*>(file.map(0, fileSize));
    if (!data) {
        delete _iteratorTimeIndex ;
        file.close();
        return;
    }
    const char* eof = data + fileSize;
    for (qint64 i = 0; i < fileSize; ++i) {
        if (data[i] == '\n') {
            ++_nrows;
        }
    }
    bool isEndNewLine = false;
    if (fileSize > 0) {
        qint64 i = fileSize - 1;
        // Skip trailing spaces/tabs/CR
        while (i >= 0 && (data[i] == ' ' ||
                          data[i] == '\t' || data[i] == '\r')) {
            --i;
        }
        if (i >= 0 && data[i] == '\n') {
            isEndNewLine = true;
        }
    }
    if (!isEndNewLine && fileSize > 0) {
        ++_nrows;  // count last row without newline
    }
    if ( _nrows > 0 ) {
        --_nrows; // take off header
    }

    // Return if no data
    if ( _nrows == 0 || _ncols == 0 ) {
        delete _iteratorTimeIndex ;
        file.unmap(reinterpret_cast<uchar*>(const_cast<char*>(data)));
        file.close();
        return;
    }

    // Allocate to hold *all* parsed data
    _data = (double*)malloc(_nrows*_ncols*sizeof(double));

    // Begin Loading Progress Dialog
    QElapsedTimer timer;
    timer.start();
    QString msg("Loading ");
    msg += QFileInfo(fileName()).fileName();
    msg += "...";
    QProgressDialog* progressLoad = nullptr;
    if ( QApplication::instance() ) {
        progressLoad = new QProgressDialog(msg, "Abort", 0, _nrows-1, 0);
        progressLoad->setWindowModality(Qt::WindowModal);
    }

    // Skip over header
    qint64 pos = 0;
    while (pos < fileSize && data[pos] != '\n') {
        ++pos;
    }
    ++pos; // Skip over '\n'

    int row = 0;
    col = 0;
    const char* ptr = &data[pos];
    while (ptr < eof) {
        char* endptr;
        double val = _strtod(ptr,eof,&endptr);
        _data[row*_ncols+col] = val;
        ptr = endptr;
        while (ptr < eof && *ptr != ',' && *ptr != '\n') {
            ++ptr; // Advance pointer to delimeter (e.g. past trailing spaces)
        }
        if ( *ptr == ',') {
            ++col;
            ++ptr;
        } else if ( *ptr == '\n' ) {
            col = 0;
            ++row;
            ++ptr;
        } else if ( *ptr == '\r' ) {
            // Handle Windows line ending ^M (\r\n)
            ++ptr;
            if ( *ptr == '\n' ) {
                ++ptr;
            }
            col = 0;
            ++row;
        } else {
            break;
        }
        if ( row % 100000 == 0 ) {
            if ( progressLoad ) {
                progressLoad->setValue(row);
                int secs = qRound(timer.nsecsElapsed()/1.0e9);
                div_t d = div(secs,60);
                QString msg = QString("Loaded %1 of %2 lines "
                                      "(%3 min %4 sec)")
                        .arg(row).arg(_nrows).arg(d.quot).arg(d.rem);
                progressLoad->setLabelText(msg);
            }
        }
    }

    // End Progress Dialog
    if ( progressLoad ) {
        progressLoad->setValue(_nrows-1);
        delete progressLoad;
    }

    // Cleanup
    file.unmap(reinterpret_cast<uchar*>(const_cast<char*>(data)));
    file.close();
}

// This behaves like strtod().  It parses out the value pointed at by ptr,
// and then sets the endptr to character after last char of numeric
double CsvModel::_strtod(const char *ptr, const char* eof, char **endptr)
{
    if ( ptr >= eof || ptr[0] == ',' || ptr[0] == '\n' || ptr[0] == '\r' ) {
        // Empty field, return NaN
        *endptr = const_cast<char*>(ptr);
        return (std::numeric_limits<double>::quiet_NaN());
    }

    double val = strtod(ptr,endptr);
    if ( **endptr == ':' ) {
        // Timestamp hh:mm:ss
        double hh = val;
        ptr = *endptr;
        ++ptr;
        double mm = strtod(ptr,endptr);
        ptr = *endptr;
        ++ptr;
        double ss = strtod(ptr,endptr);
        val = hh*3600 + mm*60 + ss;
    } else if ( ptr == *endptr ) {
        // strtod skipped value since non-numeric,
        // so try to parse string into a value
        const char* eptr = ptr;
        while (*eptr != ',' && *eptr != '\n' && eptr != eof ) {
            ++eptr;
        }
        *endptr = const_cast<char*>(eptr);
        if ( eptr == ptr ) {
            // Empty column -> NaN
            val = std::numeric_limits<double>::quiet_NaN();
        } else {
            QString s = QString::fromUtf8(ptr,eptr-ptr);
            QString st = s.trimmed();
            if ( st.size() == 1) {
                val = st.at(0).unicode();
            } else {
                // Map string to an id
                if ( !_str2id.contains(s) ) {
                    int id = _str2id.size(); // id = 0,1,2...
                    _str2id.insert(s,id);
                }
                val = _str2id.value(s);
            }
        }
    }
    return val;
}

void CsvModel::map()
{
}

void CsvModel::unmap()
{
}

int CsvModel::paramColumn(const QString &paramName) const
{
    return _paramName2col.value(paramName,-1);
}

ModelIterator *CsvModel::begin(int tcol, int xcol, int ycol) const
{
    return new CsvModelIterator(0,this,tcol,xcol,ycol);
}

CsvModel::~CsvModel()
{
    foreach ( Parameter* param, _col2param.values() ) {
        delete param;
    }
    if ( _data ) {
        free(_data);
        _data = 0;
    }

    if ( _iteratorTimeIndex ) {
        delete _iteratorTimeIndex;
        _iteratorTimeIndex = 0;
    }
}

const Parameter* CsvModel::param(int col) const
{
    return _col2param.value(col);
}

int CsvModel::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int CsvModel::_idxAtTimeBinarySearch (CsvModelIterator* it,
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

int CsvModel::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int CsvModel::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}


QVariant CsvModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(role);
    QVariant val;

    if ( idx.isValid() && _data ) {
        int row = idx.row();
        int col = idx.column();
        val = _data[row*_ncols+col];
    }

    return val;
}

bool CsvModel::isValid(const QString &csvFile, const QStringList &timeNames)
{
    QFile file(csvFile);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QTextStream in(&file);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
    #else
        in.setCodec("UTF-8");
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const auto KeepEmptyParts = Qt::KeepEmptyParts ;
    #else
        const auto KeepEmptyParts = QString::KeepEmptyParts;
    #endif

    // Get list of variable names
    QStringList names;
    QString line0 = in.readLine();
    QStringList items = line0.split(',',KeepEmptyParts);
    foreach ( QString item, items ) {
        QString name;
        if ( item.contains('{') ) {
            // Name left of unit between curlies
            int i = item.indexOf('{');
            if ( i > 0 ) {
                name = item.left(i-1).trimmed();
            }
        } else {
            name = item.trimmed();
        }
        if ( name.isEmpty() ) {
            file.close();
            return false;
        }
        names.append(name);
    }

    // Ensure time param exists
    bool isFoundTime = false;
    foreach (QString timeName, timeNames) {
        if ( names.contains(timeName) ) {
            isFoundTime = true;
            break;
        }
    }
    if ( ! isFoundTime ) {
        file.close();
        return false;
    }

    // Sanity check second line to ensure num cols is same as num header cols
    QString line1 = in.readLine();
    if ( !line1.isEmpty() ) {  // A single line header with no data is valid
        items = line1.split(',',KeepEmptyParts);
        if ( items.size() != names.size() ) {
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}
