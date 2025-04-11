#ifndef RUNSWIDGET_H
#define RUNSWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListView>
#include <QFileInfo>
#include <QDir>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QStandardPaths>

#include "bookmodel.h"
#include "monteinputsview.h"
#include "runs.h"

class RunsWidgetFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit RunsWidgetFilterProxyModel(Runs* runs, QObject *parent = nullptr) :
        QSortFilterProxyModel(parent),
        _runs(runs)
       {}

protected:
    bool filterAcceptsRow(int row,const QModelIndex &srcIdx) const;

private:
    Runs* _runs;

    bool _isDirAccept(const QString &path,
                        const QRegularExpression &rx,
                        int depth) const;
};


class RunsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RunsWidget(Runs* runs,
                        PlotBookModel* bookModel,
                        MonteInputsView* monteInputsView,
                        QWidget *parent = 0);
    ~RunsWidget();

    void setRunsHome(const QString& runsHomeDir);
    QString runsHome();

signals:
    
public slots:

private:
    QStandardItemModel* _runsModel;
    Runs* _runs;
    PlotBookModel* _bookModel;
    MonteInputsView* _monteInputsView;
    QGridLayout* _gridLayout ;
    QLineEdit* _searchBox;
    QListView* _listView ;
    QTreeView* _fileTreeView;
    QFileSystemModel* _fileModel;
    QSortFilterProxyModel* _filterModel;
    QString _runsHome;

    QStandardItemModel* _createVarsModel(Runs* runs);


private slots:
     void _runsSearchBoxReturnPressed();
};


#endif // RUNSWIDGET_H
