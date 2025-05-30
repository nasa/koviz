#ifndef RUNDIR_H
#define RUNDIR_H

#include "run.h"
#include "mapvalue.h"
#include "datamodel_csv.h"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QHash>

class RunDir : public Run
{
public:
    RunDir(const QString& run,
           const QStringList& timeNames,
           const QHash<QString,QStringList> &varMap,
           const QString& filterPattern,
           const QString& excludePattern);
    ~RunDir() {}

    QStringList params() override;
    DataModel *dataModel(const QString& param) override;

    static bool isValid(const QString& run,
                        const QStringList &timeNames,
                        const QString& filterPattern,
                        const QString& excludePattern);

private:
    QStringList _timeNames;
    const QHash<QString, QStringList> _varMap;
    QHash<QString,DataModel*> _param2model;

    static QStringList _fileList(const QString &run,
                                 const QStringList &timeNames,
                                 const QString& filterPattern,
                                 const QString& excludePattern);
};

#endif // RUNDIR_H
