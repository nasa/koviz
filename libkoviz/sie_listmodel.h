#ifndef SIELISTMODEL_H
#define SIELISTMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMimeData>

class SieListModel : public QAbstractListModel
{
public:
    explicit SieListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;

public slots:
    void setParams(const QStringList *params);
    int setRegexp(const QString &rx);

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const QStringList* _params;
    QString _rx;
    QStringList _filteredParams;
    int _fetchCount;
    int _fetchChunkSize;
};

#endif // SIELISTMODEL_H
