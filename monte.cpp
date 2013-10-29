#include "monte.h"
#include <QDebug>
#include "timeit_linux.h"

Monte::Monte(const QString& dirname) :
    _montedir(dirname),
    _err_stream(&_err_string)
{
    if ( ! dirname.isEmpty() ) {
        setDir(dirname);
    }
}

Monte::~Monte()
{
    foreach (QString ftrk, _ftrk2models.keys()) {
        QList<TrickModel*>* models = _ftrk2models.value(ftrk);
        foreach(TrickModel* model, *models) {
            delete model;
        }
        delete models;
    }
}

bool Monte::setDir(const QString &montedir)
{
    bool ok = true;
    _montedir = montedir;

    QDir mdir(montedir);
    if ( ! mdir.exists() ) {
        _err_stream << "snap [error]: couldn't find run directory: "
                    << montedir << "\n";
        throw std::invalid_argument(_err_string.toAscii().constData());
    }

    QStringList filters;
    filters << "RUN_*";
    mdir.setNameFilters(filters);

    //
    // Sanity check: Are there any RUNs in the monte carlo dir?
    //
    _runs = mdir.entryList(filters, QDir::Dirs);
    if ( _runs.empty() ) {
        _err_stream << "snap [error]: no RUN dirs in "
                    << _montedir << "\n";
        throw std::invalid_argument(_err_string.toAscii().constData());
    }

    // Get RUN_00000
    QString run0 = _runs.at(0);
    QDir rundir0(_montedir + "/" + run0);

    //
    // Make a list of trk log files from RUN_00000
    //
    QSet<QString> trks;
    QStringList filter_trk;
    filter_trk << "*.trk";
    QStringList skip;
    skip <<  "log_frame.trk" << "log_jobs.trk" << "log_mem_stats.trk";
    QStringList trklist = rundir0.entryList(filter_trk, QDir::Files);
    foreach ( QString trk, trklist ) {
        if ( skip.contains(trk) ) continue;
        trks << trk;
    }
    if ( trks.empty() ) {
        _err_stream << "snap [error]: no trk logfiles found in "
                    << _montedir << "\n";
        throw std::invalid_argument(_err_string.toAscii().constData());
    }

    //
    // Make list of params from RUN_00000
    //
    foreach (QString ftrk, trks.values()) {
        QString trk(_montedir + "/" + run0 + "/" + ftrk);
        TrickModel* m = new TrickModel(trk,trk);
        int ncols = m->columnCount();
        for ( int col = 0; col < ncols; ++col) {
            QString param = m->headerData(col,
                                          Qt::Horizontal,Param::Name).toString();
            if ( ! _params.contains(param) ) {
                _params.append(param);
                _param2ftrk.insert(param,ftrk);
            }
        }
        delete m;
    }

    //
    // Make sure all runs have same trks and params
    // Also, map ftrk to list of models
    //
    foreach (QString ftrk, trks.values()) {
        QStringList params;
        _ftrk2models.insert(ftrk,new QList<TrickModel*>);
        foreach ( QString run, _runs ) {
            QDir rundir(_montedir + "/" + run);
            if ( ! rundir.exists(ftrk) ) {
                _err_stream << "snap [error]: monte carlo runs are "
                            << "inconsistent. " << _runs.at(0)
                            << " contains file " << ftrk
                            << " but run "
                            << run << " does not.";
                throw std::invalid_argument(_err_string.toAscii().constData());
            }
            QString trk(_montedir + "/" + run + "/" + ftrk);
            TrickModel* m = new TrickModel(trk,trk);
            int ncols = m->columnCount();
            if ( run == _runs.at(0) ) {
                params.clear();
                for ( int col = 0; col < ncols; ++col) {
                    QString param = m->headerData(col,
                                        Qt::Horizontal,Param::Name).toString();
                    params.append(param);
                }
            } else {
                if ( ncols != params.size() ) {
                    _err_stream << "snap [error]: monte carlo runs are "
                                << "inconsistent. " << _runs.at(0) << "/"
                                << ftrk << " has a different number of "
                                << "parameters(" << params.size()
                                << ") than run "
                                << run << "/" << ftrk << "("
                                   << ncols << ")";
                    throw std::invalid_argument(_err_string.toAscii().
                                                constData());
                }
                for ( int col = 0; col < ncols; ++col) {
                    QString param = m->headerData(col,
                                         Qt::Horizontal,Param::Name).toString();
                    if ( param != params.at(col) ) {
                        _err_stream << "snap [error]: monte carlo runs are "
                                    << "inconsistent. " << _runs.at(0) << "/"
                                    << ftrk << " and " << run << "/" << ftrk
                                    << " have two different parameters \n"
                                    << "      " << params.at(col) << "\n"
                                    << "      " << param << "\n"
                                    << "in column " << col << ".";
                        throw std::invalid_argument(_err_string.toAscii().
                                                    constData());
                    }
                }
            }
            _ftrk2models.value(ftrk)->append(m);
            m->unmap();
        }
    }

    return ok;
}

QList<TrickModel *> *Monte::models(const QString &param)
{
    return _ftrk2models.value(_param2ftrk.value(param));
}