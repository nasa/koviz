#include "datamodel_hdf5.h"

QString Hdf5Model::_err_string;
QTextStream Hdf5Model::_err_stream(&Hdf5Model::_err_string);
const QString Hdf5Model::TimeName = QString("time");

Hdf5Model::Hdf5Model(const QStringList &timeNames,
                     const QString &runPath,
                     const QString &hdf5file,
                     const QString &signalPath,
                     QObject *parent) :
    DataModel(timeNames, runPath, hdf5file, parent),
    _timeNames(timeNames),_hdf5file(hdf5file),_signalPath(signalPath),
    _nrows(0), _ncols(0),_iteratorTimeIndex(0), _data(0)
{
#ifdef HAS_HDF5
    QStringList paths = _getTimeSignalPaths(timeNames,hdf5file);
    if ( !paths.contains(signalPath) ) {
        fprintf(stderr, "koviz [bad scoobs]: Hdf5Model() constructor: "
                "file=%s has no signalPath=%s\n",
                hdf5file.toLatin1().constData(),
                signalPath.toLatin1().constData());
        exit(-1);
    }

    _init();
#endif
}

void Hdf5Model::_init()
{
#ifdef HAS_HDF5
    QList<Parameter*> params = _getParameters(_hdf5file,_signalPath,_timeNames);
    int col = 0;
    foreach ( Parameter* param, params ) {
        _col2param.insert(col,param);
        _paramName2col.insert(param->name(),col);
        if ( _timeNames.contains(param->name()) ) {
            _timeCol = col;
        }
        ++col;
    }
    _ncols = params.size();
    _nrows = _getNumRows(_hdf5file, _signalPath);
    _iteratorTimeIndex = new Hdf5ModelIterator(0,this,
                                              _timeCol,_timeCol,_timeCol);

    // Read data!
    if ( _nrows*_ncols > 0 ) {
        _data = (double*)malloc(_nrows*_ncols*sizeof(double));
        H5::H5File file( _hdf5file.toStdString(), H5F_ACC_RDONLY);
        if ( _isTrickHdf5(_hdf5file) ) { // Trick hdf5
            for (int col = 0; col < _ncols; ++col) {
                Parameter* param = _col2param.value(col);
                QString path = param->name();
                H5::DataSet dataset = file.openDataSet(path.toStdString());
                H5::DataType dtype = dataset.getDataType();
                H5T_class_t tclass = dtype.getClass();
                std::vector<double> values(_nrows, qQNaN());
                if ( tclass == H5T_INTEGER || tclass == H5T_FLOAT ) {
                    dataset.read( values.data(), H5::PredType::NATIVE_DOUBLE);
                }
                for (int row = 0; row < _nrows; ++row) {
                    _data[row*_ncols+col] = values[row];
                }
            }
        } else { // Generic hdf5 with compound field array
            H5::DataSet dataset = file.openDataSet(_signalPath.toStdString());
            H5::Group root = file.openGroup("/");
            std::vector<DatasetInfo> datasets;
            _getDatasets(root,"",datasets);
            std::vector<FieldInfo> fields;
            for (const auto& dataset : datasets ) {
                if ( dataset.path == _signalPath ) {
                    fields = dataset.fields;
                    break;
                }
            }
            for (int col = 0; col < _ncols; ++col) {
                H5::CompType memType(sizeof(double));
                Parameter* param = _col2param.value(col);
                QString fieldName = param->name();
                QString prefix = _signalPath + "/";
                if ( fieldName.startsWith(prefix) ) {
                    fieldName = fieldName.mid(prefix.length());
                }
                std::vector<double> values(_nrows, qQNaN());
                memType.insertMember(fieldName.toStdString(),0,
                                     H5::PredType::NATIVE_DOUBLE);
                FieldInfo fieldInfo = fields.at(col);
                if ( fieldInfo.type == H5T_FLOAT ||
                     fieldInfo.type == H5T_INTEGER ) {
                    dataset.read( values.data(), memType);
                } else {
                    // non-numeric field (use NaNs)
                }
                for (int row = 0; row < _nrows; ++row) {
                    _data[row*_ncols+col] = values[row];
                }
            }
        }
    }
#endif
}

void Hdf5Model::map()
{
}

void Hdf5Model::unmap()
{
}

int Hdf5Model::paramColumn(const QString &paramName) const
{
    return _paramName2col.value(paramName,-1);
}

ModelIterator *Hdf5Model::begin(int tcol, int xcol, int ycol) const
{
    return new Hdf5ModelIterator(0,this,tcol,xcol,ycol);
}

Hdf5Model::~Hdf5Model()
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

const Parameter* Hdf5Model::param(int col) const
{
    return _col2param.value(col);
}

int Hdf5Model::indexAtTime(double time)
{
    return _idxAtTimeBinarySearch(_iteratorTimeIndex,0,rowCount()-1,time);
}

int Hdf5Model::_idxAtTimeBinarySearch (Hdf5ModelIterator* it,
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

int Hdf5Model::rowCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _nrows;
    } else {
        return 0;
    }
}

int Hdf5Model::columnCount(const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return _ncols;
    } else {
        return 0;
    }
}


QVariant Hdf5Model::data(const QModelIndex &idx, int role) const
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

bool Hdf5Model::isValid(const QString &hdf5File, const QStringList &timeNames)
{
#ifdef HAS_HDF5
    QFileInfo fi(hdf5File);

    if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
        return false;
    }

    try {
        // If there are time signal paths, this is a valid hdf5 file
        QStringList paths = _getTimeSignalPaths(timeNames,hdf5File);
        if ( !paths.isEmpty() ) {
            return true;
        }
    } catch (...) {
        return false;
    }

    return false;
#else
    Q_UNUSED(hdf5File)
    Q_UNUSED(timeNames)
    return false;
#endif
}

QList<DataModel *> Hdf5Model::dataModels(const QStringList &timeNames,
                                         const QString &runPath,
                                         const QString &hdf5file)
{
#ifndef HAS_HDF5
    Q_UNUSED(timeNames)
    Q_UNUSED(runPath)
    Q_UNUSED(hdf5file)
#endif

    QList<DataModel*> models;

#ifdef HAS_HDF5
    QStringList signalPaths = _getTimeSignalPaths(timeNames,hdf5file);
    for (const auto& signalPath : signalPaths ) {
        DataModel* m = new Hdf5Model(timeNames,runPath,hdf5file,signalPath);
        models.append(m);
    }
#endif

    return models;
}

#ifdef HAS_HDF5

void Hdf5Model::_getDatasets(const H5::Group& group,
                             const QString& base,
                             std::vector<DatasetInfo>& out)
{
    hsize_t nObjs = group.getNumObjs();

    for (hsize_t i = 0; i < nObjs; ++i) {

        QString name = QString::fromStdString(group.getObjnameByIdx(i));
        QString fullPath = base + "/" + name;

        H5G_obj_t type = group.getObjTypeByIdx(i);

        if (type == H5G_GROUP) {

            H5::Group sub = group.openGroup(name.toStdString());
            _getDatasets(sub, fullPath, out);

        } else if (type == H5G_DATASET) {

            H5::DataSet ds = group.openDataSet(name.toStdString());
            H5::DataSpace space = ds.getSpace();

            int rank = space.getSimpleExtentNdims();

            hsize_t dims[8];
            space.getSimpleExtentDims(dims);

            H5T_class_t tclass = ds.getTypeClass();

            DatasetInfo info;
            info.path = fullPath;
            info.rank = rank;
            info.size = (rank == 1) ? dims[0] : 0;
            info.type = tclass;

            if ( info.rank == 1 && info.type == H5T_COMPOUND ) {
                 _getFields(ds, info);
            }

            out.push_back(info);
        }
    }
}

void Hdf5Model::_getFields(H5::DataSet& dataset, DatasetInfo& info)
{
    H5::CompType comp = dataset.getCompType();
    int n = comp.getNmembers();

    for (int i = 0; i < n; ++i) {

        FieldInfo f;

        f.name = QString::fromStdString(comp.getMemberName(i));
        f.type = comp.getMemberDataType(i).getClass();
        f.offset = comp.getMemberOffset(i);

        info.fields.push_back(f);
    }
}

QStringList Hdf5Model::_getTimeSignalPaths(const QStringList &timeNames,
                                           const QString &fileName)
{
    QStringList paths;

    QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
        return paths; // empty list of paths
    }

    H5::H5File file = H5::H5File(fileName.toStdString(), H5F_ACC_RDONLY);
    H5::Group root = file.openGroup("/");
    std::vector<DatasetInfo> datasets;
    _getDatasets(root,"",datasets);

    // Search for Trick path=/sys.exec.out.time
    for (const auto& dataset : datasets) {
        foreach ( const QString& timeName, timeNames ) {
            if ( dataset.path.endsWith(timeName) &&
                 dataset.rank == 1 &&
                 dataset.type == H5T_FLOAT ) {
                paths.append(dataset.path);
            }
        }
    }

    // Search for paths with compound arrays
    for (const auto& dataset : datasets) {

        if (dataset.rank != 1) continue;
        if (dataset.size == 0) continue;
        if (dataset.type != H5T_COMPOUND) continue;

        bool hasTime = false;
        bool hasSignal = false;

        for (const auto& field : dataset.fields) {

            if (timeNames.contains(field.name)) {
                hasTime = true;
            }

            if (!timeNames.contains(field.name) &&
                (field.type == H5T_FLOAT || field.type == H5T_INTEGER)) {
                hasSignal = true;
            }
        }

        if (hasTime && hasSignal) {
            paths.append(dataset.path);
        }
    }

    return paths;
}

bool Hdf5Model::_isTrickHdf5(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
        return false;
    }

    try {
        H5::H5File file = H5::H5File(fileName.toStdString(), H5F_ACC_RDONLY);
        H5::Group root = file.openGroup("/");
        std::vector<DatasetInfo> datasets;
        _getDatasets(root,"",datasets);

        // Search for Trick path=/sys.exec.out.time
        for (const auto& dataset : datasets) {
            if ( dataset.path.endsWith("sys.exec.out.time") &&
                 dataset.rank == 1 &&
                 dataset.type == H5T_FLOAT ) {
                return true; // It's Trick!
            }
        }
    } catch (H5::Exception& e) {
        return false;
    }

    return false;
}

QList<Parameter *> Hdf5Model::_getParameters(const QString &fileName,
                                             const QString &path,
                                             const QStringList& timeNames)
{
    QList<Parameter*> params;

    if ( _isTrickHdf5(fileName) && path == "/sys.exec.out.time" ) {
        params = __getTrickParameters(fileName);
    } else {
        params = __getTimeSignalParameters(fileName,path,timeNames);
    }

    return params;
}

QList<Parameter *> Hdf5Model::__getTrickParameters(const QString &fileName)
{
    QList<Parameter*> params;

    H5::H5File file = H5::H5File(fileName.toStdString(), H5F_ACC_RDONLY);
    H5::Group root = file.openGroup("/");
    std::vector<DatasetInfo> datasets;
    _getDatasets(root,"",datasets);

    // Get params/units datasets
    bool isFoundParams = false;
    bool isFoundUnits = false;
    H5::DataSet paramNamesDataSet;
    H5::DataSet paramUnitsDataSet;
    for (const auto& dataset : datasets) {
        if ( dataset.path == "/header/param_names" &&
             dataset.rank == 1 &&
             dataset.type == H5T_STRING ) {

            paramNamesDataSet = file.openDataSet("/header/param_names");
            isFoundParams = true;
        }
        if ( dataset.path == "/header/param_units" &&
             dataset.rank == 1 &&
             dataset.type == H5T_STRING ) {

            paramUnitsDataSet = file.openDataSet("/header/param_units");
            isFoundUnits = true;
        }
        if ( isFoundParams && isFoundUnits ) {
            break;
        }
    }
    if ( !isFoundParams || !isFoundUnits ) {
        fprintf(stderr, "koviz [bad scoobs]: Hdf5Model::__getTrickParameters: "
                " Could not find path=/header/param_names|units in file=%s\n",
                fileName.toLatin1().constData());
        exit(-1);
    }

    // Params
    H5::DataSpace space = paramNamesDataSet.getSpace();
    hsize_t dims[1];
    space.getSimpleExtentDims(dims);
    hsize_t nParams = dims[0];
    H5::StrType strType = paramNamesDataSet.getStrType();
    bool isVar = H5Tis_variable_str(strType.getId());
    if ( isVar ) {
        fprintf(stderr, "koviz [bad scoobs]:1: Unexpected Trick hdf5 variable "
                        "length strings");
        exit(-1);
    }
    size_t fixedStringLength = strType.getSize();
    char* paramsBuffer = new char[nParams*fixedStringLength];
    paramNamesDataSet.read(paramsBuffer, strType);

    // Units
    space = paramNamesDataSet.getSpace();
    space.getSimpleExtentDims(dims);
    hsize_t nUnits = dims[0];
    strType = paramUnitsDataSet.getStrType();
    isVar = H5Tis_variable_str(strType.getId());
    if ( isVar ) {
        fprintf(stderr, "koviz [bad scoobs]:2: Unexpected Trick hdf5 variable "
                        "length strings");
        exit(-1);
    }
    fixedStringLength = strType.getSize();
    char* unitsBuffer = new char[nUnits*fixedStringLength];
    paramUnitsDataSet.read(unitsBuffer, strType);

    if ( nParams != nUnits ) {
        fprintf(stderr,
                "koviz [bad scoobs]: hdf5 nparams=%llu != nunits=%llu\n",
                nParams,nUnits);
        exit(-1);
    }

    for (hsize_t i = 0; i < nParams; ++i) {

        char* name = paramsBuffer + i*fixedStringLength;
        char* unit = unitsBuffer + i*fixedStringLength;
        Parameter* param = new Parameter;
        param->setName(name);
        param->setUnit(unit);
        params.append(param);
    }

    delete[] paramsBuffer;
    delete[] unitsBuffer;

    return params;
}

QList<Parameter*> Hdf5Model::__getTimeSignalParameters(const QString &fileName,
                                                   const QString &path,
                                                   const QStringList &timeNames)
{
    QList<Parameter*> params;

    H5::H5File file = H5::H5File(fileName.toStdString(), H5F_ACC_RDONLY);
    H5::Group root = file.openGroup("/");
    std::vector<DatasetInfo> datasets;
    _getDatasets(root,"",datasets);

    for ( const auto& dataset : datasets ) {
        if ( dataset.path == path ) {
            for (const auto& field: dataset.fields) {
                // Fields are the parameters (no units available afaik)
                Parameter* param = new Parameter;
                if ( timeNames.contains(field.name) ) {
                    // Use time name that may be shared across multiple paths
                    param->setName(field.name);
                } else {
                    // For non-time params use full path since many
                    // hdf5 files have same vars per path
                    param->setName(path + "/" + field.name);
                }
                param->setUnit("--");
                params.append(param);
            }
            break;
        }
    }

    return params;
}

int Hdf5Model::_getNumRows(const QString &fileName, const QString &path)
{
    int nRows = 0;

    H5::H5File file = H5::H5File(fileName.toStdString(), H5F_ACC_RDONLY);
    H5::Group root = file.openGroup("/");
    std::vector<DatasetInfo> datasets;
    _getDatasets(root,"",datasets);

    for (const auto& dataset : datasets) {
        if ( dataset.path == path ) {
            nRows = dataset.size;
            break;
        }
    }

    return nRows;
}

#endif
