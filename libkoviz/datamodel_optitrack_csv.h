#ifndef OPTITRACK_CSV__MODEL_H
#define OPTITRACK_CSV__MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <stdexcept>
#include <cmath> // For NAN macro

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

class OptiTrackCsvModel;
class OptiTrackCsvModelIterator;

class OptiTrackCsvModel : public DataModel
{
  Q_OBJECT

  friend class OptiTrackCsvModelIterator;

  public:

    explicit OptiTrackCsvModel(const QStringList &timeNames,
                      const QString &runPath,
                      const QString &csvFile,
                      QObject *parent = 0);
    ~OptiTrackCsvModel();

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

     static bool isValid(const QString &fileName);

  private:

    QStringList _timeNames;
    QString _csvFile;
    double _frameRate;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    OptiTrackCsvModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (OptiTrackCsvModelIterator *it,
                               int low, int high, double time);

    inline double _convert(const QString& s);
};

class OptiTrackCsvModelIterator : public ModelIterator
{
  public:

    inline OptiTrackCsvModelIterator(): i(0) {}

    inline OptiTrackCsvModelIterator(int row, // iterator pos
                            const OptiTrackCsvModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _xcol(xcol), _ycol(ycol)
    {
        Q_UNUSED(tcol);
    }

    virtual ~OptiTrackCsvModelIterator() {}

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

    OptiTrackCsvModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    // Time is calculated from the data but not in _data
    inline double t() const override
    {
        // 2 is the column of the device frame
        double deviceFrame = _model->_data[i*(_model->_ncols-1)+2];
        return deviceFrame/(_model->_frameRate);
    }

    inline double x() const override
    {
        if ( _xcol == _model->_timeCol ) {
            return t();
        } else {
            return _model->_data[i*(_model->_ncols-1)+(_xcol-1)];
        }
    }

    inline double y() const override
    {
        if ( _ycol == _model->_timeCol ) {
            return t();
        } else {
            return _model->_data[i*(_model->_ncols-1)+(_ycol-1)];
        }
    }

  private:

    int i;
    const OptiTrackCsvModel* _model;
    int _xcol;
    int _ycol;
};


#endif // OPTITRACK_CSV_MODEL_H
