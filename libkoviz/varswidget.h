#ifndef VARSWIDGET_H
#define VARSWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListView>
#include <QFileInfo>
#include <QDir>
#include <QProgressDialog>
#include <QApplication>
#include <QElapsedTimer>
#include <float.h>
#include <stdlib.h>
#include "dp.h"
#include "bookmodel.h"
#include "monteinputsview.h"
#include "trickvarsortfilterproxymodel.h"
#include "runs.h"

class VarsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VarsWidget(const QString& timeName,
                        Runs* runs,
                        const QStringList& unitOverrides,
                        PlotBookModel* plotModel,
                        QItemSelectionModel*  plotSelectModel,
                        MonteInputsView* monteInputsView,
                        QWidget *parent = 0);
    ~VarsWidget();

    void clearSelection();
    void selectAllVars();
    void setDragEnabled(bool isEnabled);


signals:
    
public slots:

private:
    QString _timeName;
    Runs* _runs;
    QStandardItemModel* _varsModel;
    QStringList _unitOverrides;
    PlotBookModel* _plotModel;
    QItemSelectionModel*  _plotSelectModel;
    MonteInputsView* _monteInputsView;
    QGridLayout* _gridLayout ;
    QLineEdit* _searchBox;
    QListView* _listView ;

    TrickVarSortFilterProxyModel* _varsFilterModel;
    QItemSelectionModel* _varsSelectModel;

    int _qpId;

    QModelIndex _findSinglePlotPageWithCurve(const QString& curveYName);
    QStandardItem* _addChild(QStandardItem* parentItem,
                   const QString& childTitle,
                   const QVariant &childValue=QVariant());
    void _addPlotToPage(QStandardItem* pageItem,
                                 const QModelIndex &varIdx);
    void _selectCurrentRunOnPageItem(QStandardItem* pageItem);

    QStandardItemModel* _createVarsModel(Runs* runs);


private slots:
     void _varsSearchBoxTextChanged(const QString& rx);
     void _varsSelectModelSelectionChanged(
                              const QItemSelection& currVarSelection,
                              const QItemSelection& prevVarSelection);
     void _runsRefreshed();
};

#endif // VARSWIDGET_H
