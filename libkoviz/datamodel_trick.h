#ifndef TRICK_MODEL_H
#define TRICK_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <QHash>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QAbstractTableModel>
#include <QString>
#include <QStringList>
#include <vector>

#include "datamodel.h"
#include "snaptable.h"
#include "trick_types.h"
#include "parameter.h"
using namespace std;

class TrickModel;
class TrickModelIterator;

class TrickParameter : public Parameter
{
public:
    TrickParameter() :
        _name(""),
        _unit("--"),
        _type(0),
        _size(0)
    {}

    void setName(const QString& name ) override { _name = name; }
    void setUnit(const QString& unit ) override { _unit = unit; }
    QString name() const override { return _name; }
    QString unit() const override { return _unit; }

    void setType(int type) { _type = type; }
    void setSize(int size) { _size = size; }
    int type() const { return _type; }
    int size() const { return _size; }

protected:
    QString _name;
    QString _unit;

private:
    int     _type;
    int     _size;
};

class TrickModel : public DataModel
{
  Q_OBJECT

  friend class TrickModelIterator;

  public:

    enum TrickVersion
    {
        TrickVersion07,
        TrickVersion10
    };

    explicit TrickModel(const QStringList &timeNames,
                        const QString &runPath,
                        const QString &trkfile,
                       QObject *parent = 0);
    ~TrickModel();

    const Parameter* param(int col) const override ;

    void map() override;
    void unmap() override;
    int paramColumn(const QString& param) const override
    {
        return _param2column.value(param,-1);
    }
    ModelIterator* begin(int tcol, int xcol, int ycol) const override ;
    int indexAtTime(double time) override;

    static void writeTrkHeader(QDataStream &out, const QList<TrickParameter> &params);

    int rowCount(const QModelIndex & pidx = QModelIndex() ) const override;
    int columnCount(const QModelIndex & pidx = QModelIndex() ) const override;
    QVariant data (const QModelIndex & index,
                   int role = Qt::DisplayRole ) const override;

  private:

    QStringList _timeNames;
    QString _trkfile;

    QHash<int,TrickParameter*> _col2param;   // ordered by column

    TrickVersion _trick_version;
    vector<int> _paramtypes;
    QHash<QString,int> _param2column;

    qint64 _nrows;
    qint64 _row_size;
    qint32 _ncols;
    qint32 _timeCol;
    QHash<int,qint64> _col2offset;

    qint64 _pos_beg_data;
    ptrdiff_t _mem;
    ptrdiff_t _data;
    int _fd;
    struct stat _fstat;
    QFile _file;

    TrickModelIterator* _iteratorTimeIndex;

    static QString _err_string;
    static QTextStream _err_stream;

    bool _load_trick_header();
    qint32 _load_binary_param(QDataStream& in, int col);
    int _idxAtTimeBinarySearch (TrickModelIterator *it,
                               int low, int high, double time);

    static void _write_binary_param(QDataStream& out, const TrickParameter &p);
    static void _write_binary_qstring(QDataStream& out, const QString& str);

    void _toDoubleErrorMessage(int col, int paramtype) const
    {
        static bool isPrintedErrorMsg = false;
        if ( !isPrintedErrorMsg ) {
            const Parameter *p = param(col);
            fprintf(stderr,
                    "koviz [error]: can't handle trick 10 type \"%d\"\n"
                    "for parameter=%s\n"
                    "Look in trick_types.h for type.\n"
                    "Only printing first error encountered.\n",
                    paramtype, p->name().toLatin1().constData());
            isPrintedErrorMsg = true;
        }
    }

  private:

    inline double _toDouble(ptrdiff_t addr, int paramtype, int col) const
    {
        if ( _trick_version == TrickVersion07 ) {
            switch (paramtype) {
            case TRICK_07_DOUBLE:
            {
                return *((double*)(addr));
            }
            case TRICK_07_UNSIGNED_LONG_LONG:
            {
                return (double) *((unsigned long long*)(addr));
            }
            case TRICK_07_LONG_LONG:
            {
                return (double) *((long long*)(addr));
            }
            case TRICK_07_FLOAT:
            {
                return (double) *((float*)(addr));
            }
            case TRICK_07_INTEGER:
            case TRICK_07_ENUMERATED:
            case TRICK_07_UNSIGNED_BITFIELD:
            {
                return (double) *((int*)(addr));
            }
            case TRICK_07_UNSIGNED_CHARACTER:
            {
                return (double) *((unsigned char*)(addr));
            }
            case TRICK_07_SHORT:
            {
                return (double) *((short int*)(addr));
            }
            case TRICK_07_UNSIGNED_SHORT:
            {
                return (double) *((unsigned short int*)(addr));
            }
            case TRICK_07_UNSIGNED_INTEGER:
            {
                return (double) *((unsigned int*)(addr));
            }
            case TRICK_07_LONG:
            {
                return (double) *((long int*)(addr));
            }
            case TRICK_07_BOOLEAN:
            {
                return (double) *((bool *)(addr));
            }
            case TRICK_07_BITFIELD:
            {
                return (double) *((int *)(addr));
            }
            default:
            {
                _toDoubleErrorMessage(col,paramtype);
                return qQNaN();
            }
            }
        } else {
            switch (paramtype) {
            case TRICK_10_DOUBLE:
            {
                return *((double*)(addr));
            }
            case TRICK_10_UNSIGNED_LONG_LONG:
            {
                return (double) *((unsigned long long*)(addr));
            }
            case TRICK_10_LONG_LONG:
            {
                return (double) *((long long*)(addr));
            }
            case TRICK_10_FLOAT:
            {
                return (double) *((float*)(addr));
            }
            case TRICK_10_INTEGER:
            case TRICK_10_ENUMERATED:
            case TRICK_10_UNSIGNED_BITFIELD:
            {
                return (double) *((int*)(addr));
            }
            case TRICK_10_UNSIGNED_CHARACTER:
            {
                return (double) *((unsigned char*)(addr));
            }
            case TRICK_10_SHORT:
            {
                return (double) *((short int*)(addr));
            }
            case TRICK_10_UNSIGNED_SHORT:
            {
                return (double) *((unsigned short int*)(addr));
            }
            case TRICK_10_UNSIGNED_INTEGER:
            {
                return (double) *((unsigned int*)(addr));
            }
            case TRICK_10_LONG:
            {
                return (double) *((long int*)(addr));
            }
            case TRICK_10_BOOLEAN:
            {
                return (double) *((bool *)(addr));
            }
            case TRICK_10_BITFIELD:
            {
                return (double) *((int *)(addr));
            }
            case TRICK_10_CHARACTER:
            {
                return (double) *((char *)(addr));
            }
            case TRICK_10_UNSIGNED_LONG:
            {
                return (double) *((unsigned long*)(addr));
            }
            default:
            {
                _toDoubleErrorMessage(col,paramtype);
                return qQNaN();
            }
            }
        }
    }

signals:
    
public slots:
    
};

//
// Iterates over rows for curve data triple - (time,x,y)
//
class TrickModelIterator : public ModelIterator
{
  public:

    inline TrickModelIterator(): i(0) {}

    inline TrickModelIterator(int row, // iterator pos
                              const TrickModel* model,
                              int tcol, int xcol, int ycol):
        i(row),
        _model(model),
        _tcol(tcol),_xcol(xcol),_ycol(ycol),
        _row_count(model->rowCount()),
        _row_size(model->_row_size),_data(model->_data),
        _tco(_model->_col2offset.value(tcol)),
        _xco(_model->_col2offset.value(xcol)),
        _yco(_model->_col2offset.value(ycol)),
        _ttype(_model->_paramtypes.at(tcol)),
        _xtype(_model->_paramtypes.at(xcol)),
        _ytype(_model->_paramtypes.at(ycol))
    {
    }

    virtual ~TrickModelIterator() {}

    void start() override
    {
        i = 0;
        _row_count = _model->rowCount();
    }

    void next() override
    {
        ++i;
    }

    bool isDone() const override
    {
        return ( i >= _row_count ) ;
    }

    TrickModelIterator* at(int n) override
    {
        i = n;
        return this;
    }

    inline double t() const override
    {
        return _model->_toDouble(_data+i*_row_size+_tco,_ttype,_tcol);
    }

    inline double x() const override
    {
        return _model->_toDouble(_data+i*_row_size+_xco,_xtype,_xcol);
    }

    inline double y() const override
    {
        return _model->_toDouble(_data+i*_row_size+_yco,_ytype,_ycol);
    }

  private:

    qint64 i;
    const TrickModel* _model;
    int _tcol;
    int _xcol;
    int _ycol;
    int _row_count;
    int _row_size;
    ptrdiff_t _data;
    qint64 _tco ;
    qint64 _xco ;
    qint64 _yco ;
    int _ttype ;
    int _xtype ;
    int _ytype ;
};


#endif // TRICKMODEL_H
