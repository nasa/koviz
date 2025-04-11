#include "dpfilterproxymodel.h"

// static to get around const
QHash<QString,bool> DPFilterProxyModel::_acceptedDPFileCache;

DPFilterProxyModel::DPFilterProxyModel(const QString& timeName,
                                       Runs* runs,
                                       SieListModel* sieModel,
                                       QObject *parent) :
    QSortFilterProxyModel(parent),
    _timeName(timeName),
    _runs(runs),
    _sieModel(sieModel)
{
    _runsRefreshed();

    connect(_runs, SIGNAL(runsRefreshed()),
            this, SLOT(_runsRefreshed()));

    if ( _sieModel ) {
        connect(_sieModel, SIGNAL(modelLoaded()),
                this, SLOT(_sieModelLoaded()));
    }
}

bool DPFilterProxyModel::filterAcceptsRow(int row,
                                          const QModelIndex &pidx) const
{
    if ( ! pidx.isValid() ) {
        return false;
    }

    bool isAccept = false;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QRegularExpression rx(filterRegularExpression());
#else
    QRegExp rx(filterRegExp());
#endif
    QFileSystemModel* m = (QFileSystemModel*) sourceModel();
    int ncols = m->columnCount(pidx);
    for ( int col = 0; col < ncols ; ++col) {
        QModelIndex idx = m->index(row,col,pidx);
        if ( _isAccept(idx,m,rx) ) {
            isAccept = true;
            break;
        }
    }

    return isAccept;
}

bool DPFilterProxyModel::filterAcceptsColumn(int col,
                                             const QModelIndex &pidx) const
{
    if ( !pidx.isValid() ) {
        return false;
    }

    if ( col > 0 ) {
        return false;
    }

    bool isAccept = false;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QRegularExpression rx(filterRegularExpression());
#else
    QRegExp rx(filterRegExp());
#endif
    QFileSystemModel* m = (QFileSystemModel*) sourceModel();
    int nrows = m->rowCount(pidx);
    for ( int row = 0; row < nrows ; ++row) {
        QModelIndex idx = m->index(row,col,pidx);
        if ( _isAccept(idx,m,rx) ) {
            isAccept = true;
            break;
        }
    }

    return isAccept;
}

void DPFilterProxyModel::_runsRefreshed()
{
    _modelParams.clear();
    _modelParams.insert(_timeName,0);
    foreach (QString param, _runs->params()) {
        _modelParams.insert(param,0);
    }
    _acceptedDPFileCache.clear();
    invalidateFilter();
}

void DPFilterProxyModel::_sieModelLoaded()
{
    // Refresh filter after model loaded
    _acceptedDPFileCache.clear();
    invalidateFilter();
}

bool DPFilterProxyModel::_isAccept(const QModelIndex &idx,
                                   QFileSystemModel *m,
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
                                   const QRegularExpression &rx) const
#else
                                   const QRegExp &rx) const
#endif
{
    bool isAccept = false ;

    QString dpFilePath = m->fileInfo(idx).absoluteFilePath();
    bool isCached = _acceptedDPFileCache.contains(dpFilePath);
    if ( isCached ) {
        return _acceptedDPFileCache.value(dpFilePath);
    }

    if ( m->isDir(idx) ) {
        isAccept = true;
    } else {
        QString path = m->filePath(idx);
        if ( path.contains(rx) && (m->fileInfo(idx).suffix() == "xml" ||
             ((m->fileInfo(idx).fileName().startsWith("DP_") &&
               m->fileInfo(idx).suffix().isEmpty() &&
              m->fileInfo(idx).isFile()))) ) {
            isAccept = true;
        }

        // Filter for DPs that have params in recorded data or live sim
        if ( isAccept ) {
            foreach ( QString param,DPProduct::paramList(dpFilePath) ) {
                if ( !_modelParams.contains(param) ) {
                    if ( _sieModel ) {
                        if ( !_sieModel->isParamExists(param) ) {
                            isAccept = false;
                            break;
                        }
                    } else {
                        isAccept = false;
                        break;
                    }
                }
            }
        }
    }

    _acceptedDPFileCache.insert(dpFilePath,isAccept);

    return isAccept;
}
