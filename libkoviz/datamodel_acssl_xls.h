#ifndef ACSSL_XLS_MODEL_H
#define ACSSL_XLS_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QApplication>
#include <stdexcept>

#include "datamodel.h"
#include "parameter.h"
#include "unit.h"

class AcsslXlsModel;
class AcsslXlsModelIterator;

class AcsslXlsModel : public DataModel
{
  Q_OBJECT

  friend class AcsslXlsModelIterator;

  public:

    explicit AcsslXlsModel(const QStringList &timeNames,
                      const QString &runPath,
                      const QString &xlsfile,
                      QObject *parent = 0);
    ~AcsslXlsModel();

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

    static bool isValid( const QString& xlsFile,
                         const QStringList& timeNames);

  private:

    QStringList _timeNames;
    QString _xlsfile;

    int _nrows;
    int _ncols;
    int _timeCol;

    QHash<int,Parameter*> _col2param;
    QHash<QString,int> _paramName2col;
    AcsslXlsModelIterator* _iteratorTimeIndex;

    double* _data;

    static QString _err_string;
    static QTextStream _err_stream;

    void _init();
    int _idxAtTimeBinarySearch (AcsslXlsModelIterator *it,
                               int low, int high, double time);

    inline double _strtod(const char* ptr, const char *eof, char **endptr);
    QHash<QString,int> _str2id;
};

class AcsslXlsModelIterator : public ModelIterator
{
  public:

    inline AcsslXlsModelIterator(): i(0) {}

    inline AcsslXlsModelIterator(int row, // iterator pos
                            const AcsslXlsModel* model,
                            int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol), _xcol(xcol), _ycol(ycol)
    {
    }

    virtual ~AcsslXlsModelIterator() {}

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

    AcsslXlsModelIterator* at(int n) override
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
    const AcsslXlsModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
};


#endif // ACSSL_XLSMODEL_H
