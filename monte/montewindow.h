#ifndef MONTE_WINDOW_H
#define MONTE_WINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QLineEdit>
#include <QGridLayout>
#include <QList>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QRegExp>
#include <QTreeView>
#include <QTableView>
#include <QListView>
#include <QFileInfo>
#include <QTabWidget>
#include <QStringList>
#include <QFileDialog>

#include "libsnapdata/monte.h"
#include "dp.h"
#include "plotbookview.h"
#include "dpfilterproxymodel.h"

#include "libsnapdata/timeit_linux.h"

class MonteWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonteWindow(const QString& montedir,
                         int beginRun, int endRun,
                         QWidget *parent = 0);
    ~MonteWindow();

private:
    QString _montedir;
    int _beginRun;
    int _endRun;
    Monte* _monte;
    QGridLayout* _layout;
    QGridLayout* _left_lay ;

    void createMenu();
    QMenuBar* _menuBar;
    QMenu *_fileMenu;
    QAction *_pdfAction;
    QAction *_exitAction;
    TimeItLinux _timer;

    QTabWidget* _nbDPVars;

    QStandardItemModel* _createVarsModel(MonteModel* mm);
    QStandardItemModel* _varsModel;
    QSortFilterProxyModel* _varsFilterModel;
    QItemSelectionModel* _varsSelectModel;
    QLineEdit* _varsSearchBox;
    QListView* _varsListView ;
    void _updateVarSelection(const QModelIndex& pageIdx);

    QFileSystemModel* _dpModel ;
    DPFilterProxyModel* _dpFilterModel;
    QTreeView* _dpTreeView ;
    QLineEdit* _dpSearchBox;
    void _updateDPSelection(const QModelIndex& pageIdx);

    QStandardItemModel* _plotModel;
    QTreeView* _plotTreeView ;
    PlotBookView* _plotBookView;
    QItemSelectionModel* _plotSelectModel;
    int _currQPIdx;
    bool _isSkip; // Hack City :(

    MonteModel* _monteModel;
    void _createDPPages(const QString& dpfile);
    bool _isDP(const QString& fp);
    bool _isRUN(const QString& fp);
    bool _isMONTE(const QString& fp);

    QStandardItemModel* _monteInputsModel; // from monte_runs file
    QTableView* _monteInputsView ;
    QItemSelectionModel* _monteInputsSelectModel;

    QString _descrPlotTitle(DPPlot* plot);

    QStandardItem* _createQPItem();
    void _addPlotOfVarToPageItem(QStandardItem* pageItem,
                          const QModelIndex &varIdx);
    QModelIndex _findSinglePlotPageWithCurve(const QString& curveName);

    void _selectCurrentRunOnPageItem(QStandardItem* pageItem);
    int currSelectedRun();

    bool _isCurveIdx(const QModelIndex& idx) const;

private slots:
     void _dpTreeViewClicked(const QModelIndex& idx);
     void _dpTreeViewCurrentChanged(const QModelIndex &currIdx,
                                           const QModelIndex &prevIdx);
     void _dpSearchBoxTextChanged(const QString& rx);
     void _varsSelectModelSelectionChanged(
                              const QItemSelection& currVarSelection,
                              const QItemSelection& prevVarSelection);
     void _varsSearchBoxTextChanged(const QString& rx);
     void _monteInputsSelectModelCurrentChanged(const QModelIndex& curr,
                                     const QModelIndex& prev);
     void _monteInputsViewHeaderSectionClicked(int section);
     void _plotSelectModelSelectionChanged(const QItemSelection& currSel,
                                          const QItemSelection& prevSel);
     void _plotSelectModelCurrentChanged(const QModelIndex& currIdx,
                                        const QModelIndex& prevIdx);
     void _plotModelRowsAboutToBeRemoved(const QModelIndex& pidx,
                                         int start, int end);
     void _savePdf();
};

#endif // MAINWINDOW_H
