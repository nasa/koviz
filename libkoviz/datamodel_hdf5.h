#ifndef HDF5_MODEL_H
#define HDF5_MODEL_H

#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QApplication>
#include <vector>

#ifdef HAS_HDF5
#include <H5Cpp.h>
#endif

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

class Hdf5Model;
class Hdf5ModelIterator;

class Hdf5Model : public DataModel
{
  Q_OBJECT

#ifdef HAS_HDF5
  struct FieldInfo {
      QString name;
      H5T_class_t type;
      size_t offset;
  };

  struct DatasetInfo {
    QString path;
    H5T_class_t type;
    hsize_t size;
    int rank;
    std::vector<FieldInfo> fields;
  };
#endif

  friend class Hdf5ModelIterator;

  public:

    explicit Hdf5Model(const QStringList &timeNames,
                       const QString &runPath,
                       const QString &hdf5file,
                       const QString &signalPath,
                       QObject *parent = 0);
    ~Hdf5Model();

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

    static bool isValid( const QString& hdf5File,
                         const QStringList& timeNames);

    static QList<DataModel*> dataModels(const QStringList &timeNames,
                                        const QString &runPath,
                                        const QString &hdf5file);

  private:

    QStringList _timeNames;
    QString _hdf5file;
    QString _signalPath;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    Hdf5ModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (Hdf5ModelIterator *it,
                               int low, int high, double time);

    QHash<QString,int> _str2id;

#ifdef HAS_HDF5
    static void _getDatasets(const H5::Group& group,
                             const QString& base,
                             std::vector<DatasetInfo>& out);
    static void _getFields(H5::DataSet& dataset, DatasetInfo& info);

    static QStringList _getTimeSignalPaths(const QStringList& timeNames,
                                           const QString& fileName);

    bool _isTrickHdf5(const QString& fileName);

    QList<Parameter*> _getParameters(const QString& fileName,
                                     const QString &path,
                                     const QStringList &timeNames);
    QList<Parameter*> __getTrickParameters(const QString& fileName);
    QList<Parameter*> __getTimeSignalParameters(const QString& fileName,
                                                const QString& path,
                                                const QStringList& timeNames);
    int _getNumRows(const QString& fileName, const QString& path);
#endif
};

class Hdf5ModelIterator : public ModelIterator
{
  public:

    inline Hdf5ModelIterator(): i(0) {}

    inline Hdf5ModelIterator(int row, // iterator pos
                            const Hdf5Model* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~Hdf5ModelIterator() {}

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

    Hdf5ModelIterator* at(int n) override
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
    const Hdf5Model* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // HDF5MODEL_H
