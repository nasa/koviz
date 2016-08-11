#ifndef MONTE_H
#define MONTE_H

#include <QString>
#include <QDir>
#include <QFile>
#include <QPair>
#include <QTextStream>
#include <QSet>
#include <QHash>
#include <QList>
#include <QStandardItemModel>
#include <stdexcept>
#include "trickmodel.h"
#include "numsortitem.h"
#include "runs.h"

class Monte : public Runs
{
  public:
    Monte(const QString &dirname, int beginRun=0, int endRun=1.0e6);
    ~Monte();
    QStringList params() const { return _params; }
    QStringList runs() const { return _runsSubset; }
    QList<TrickModel*>* models(const QString& param);
    QStandardItemModel* inputModel();   // rows:monteinputvars cols:values

  private:
    QString _montedir;
    int _beginRun;
    int _endRun;
    QStringList _runs;
    QStringList _runsSubset; // subset bounds are [_beginRun,_endRun]
    QMap<QString,QString> _param2ftrk;
    QStringList _params;
    QHash<QString,QList<TrickModel*>* > _ftrk2models;
    static QString _err_string;
    static QTextStream _err_stream;

    bool _setDir(const QString& montedir);
};


#endif // MONTE_H