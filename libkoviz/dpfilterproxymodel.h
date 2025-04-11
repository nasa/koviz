#ifndef DPFILTERPROXYMODEL_H
#define DPFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QFileSystemModel>
#include <QString>
#include <QHash>

#include "dp.h"
#include "sie_listmodel.h"
#include "runs.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    #include <QRegularExpression>
#else
    #include <QRegExp>
#endif

// The dp filter models uses a list of params that are common between runs.
// i.e. Runs::params(). Only DP_files which have params which are in
// all runs will show in the DP tree
//
// The dp filter also takes the sie model.  The sie model is used to
// filter for variables that the live sim publishes.
class DPFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DPFilterProxyModel(const QString &timeName,
                                Runs* runs,
                                SieListModel* sieModel,
                                QObject *parent = 0);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &pidx) const;
    bool filterAcceptsColumn(int col, const QModelIndex &pidx) const;
signals:
    
private slots:
    void _runsRefreshed();
    void _sieModelLoaded();

private:
    QString _timeName;
    Runs* _runs;
    QHash<QString,int> _modelParams;
    SieListModel* _sieModel;
    static QHash<QString,bool> _acceptedDPFileCache; // static to get around const

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    bool _isAccept(const QModelIndex& idx,
                   QFileSystemModel* m,
                   const QRegularExpression& rx) const;
#else
    bool _isAccept(const QModelIndex& idx,
                   QFileSystemModel* m,
                   const QRegExp& rx) const;
#endif
};

#endif // DPFILTERPROXYMODEL_H
