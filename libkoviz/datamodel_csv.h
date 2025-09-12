#ifndef CSV_MODEL_H
#define CSV_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QElapsedTimer>
#include <stdexcept>

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

class CsvModel;
class CsvModelIterator;

class CsvModel : public DataModel
{
  Q_OBJECT

  friend class CsvModelIterator;

  public:

    explicit CsvModel(const QStringList &timeNames,
                      const QString &runPath,
                      const QString &csvfile,
                      QObject *parent = 0);
    ~CsvModel();

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
                         const QStringList& timeNames);

  private:

    QStringList _timeNames;
    QString _csvfile;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    CsvModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (CsvModelIterator *it,
                               int low, int high, double time);

    inline double _strtod(const char* ptr, const char *eof, char **endptr);
    QHash<QString,int> _str2id;
};

class CsvModelIterator : public ModelIterator
{
  public:

    inline CsvModelIterator(): i(0) {}

    inline CsvModelIterator(int row, // iterator pos
                            const CsvModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~CsvModelIterator() {}

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

    CsvModelIterator* at(int n) override
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
    const CsvModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // CSVMODEL_H
