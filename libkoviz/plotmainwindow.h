# PLOTMAINWINDOW_H
# PLOTMAINWINDOW_H

# <QMainWindow>
# <QMenu>
# <QMenuBar>
# <QFrame>
# <QVBoxLayout>
# <QHBoxLayout>
# <QGridLayout>
# <QSplitter>
# <QList>
# <QHash>
# <QStandardItemModel>
# <QFileInfo>
# <QStringList>
# <QFileDialog>
# <QMessageBox>
# <QLineEdit>
# <QSizePolicy>
# <QTreeView>
# <QFile>
# <QTextStream>
# <QDate>
# <QSettings>
# <QProcess>
# <QTcpSocket>
# <QStatusBar>

# "monte.h"
# "dp.h"
# "bookmodel.h"
# "dptreewidget.h"
# "varswidget.h"
# "monteinputsview.h"
# "timeinput.h"
# "bookview.h"
# "runs.h"
# "timecom.h"
# "videowindow.h"

 PlotMainWindow :  QMainWindow
{
    Q_OBJECT

public:
     PlotMainWindow( PlotBookModel* bookModel,
                              QString& trickhost,
                              trickport,
                             trickoffset,
                              QString& videoFileName,
                              videoOffset,
                              QString& excludePattern,
                              QString& filterPattern,
                              QString& scripts,
                              isDebug,
                              isPlotAllVars,
                              QStringList& timeNames,
                              QString& dpDir,
                              QStringList& dpFiles,
                              isShowTables,
                             QStringList unitOverrides,
                             QString map,
                             QString mapFile,
                             Run runs,
                             QStandardItemModel varsModel,
                             QStandardItemModel monteInputsModel=0,
                             QWidget *parent 0);

      savePdf( QStrin fname);

    ~PlotMainWindow();

protected:
     virtual closeEvent(QCloseEvent *);


private:
    PlotBookModel* _bookModel;

     QString& _trickhost;
   uint_  _trickport;
     _trickoffset;
    QString _videoFileName;
     _videoOffset;
    QString _excludePattern;
    QString _filterPattern;
    QString _scripts;
     _isDebug;
    QStringList _timeNames;
    QString _dpDir;
    QStringList _dpFiles;
     _isShowTables;
    QStringList _unitOverrides;
    QString _map;
    QString _mapFile;
    Run _runs;
    QStandardItemModel* _varsModel;
    QStandardItemModel* _monteInputsModel;
    MonteInputsView* _monteInputsView;
    QHeaderView* _monteInputsHeaderView;
    TimeInput* _timeInput;

    QGridLayout _layout;
    QGridLayout _left_lay ;

     createMenu();
    QMenuBar menuBar;
    QMenu fileMenu;
    QMenu optsMenu;
    QMenu scriptsMenu;
    QAction dpAction;
    QAction pdfAction;
    QAction jpgAction;
    QAction sessionAction;
    QAction openVideoAction;
    QAction exitAction;
    QAction showLiveCoordAction;
    QAction clearPlotsAction;
    QAction clearTablesAction;
    QAction plotAllVarsAction;

    QTabWidget nbDPVars;
    VarsWidget varsWidget;
    QFrame _dpFrame ;
    DPTreeWidget _dpTreeWidget;

    QTreeView _plotTreeView ;
    BookView _bookView;

    QStatusBar _statusBar;

     RUN(QString& fp);
    MONTE(QString& fp);

     _readSettings();
     _writeSettings();
     _readMainWindowSettings();
     _readVideoWindowSettings();

    QModelIndex _currCurveIdx();

    TimeCom _the_visualizer;
    TimeCom _blender;

    VideoWindow vidView;
    QTcpSocket _vsSocket ;

     _openVideoFile( QString);


 slots:
      _nbCurrentChanged(int i);
      _bookModelRowsAboutToBeRemoved( QModelIndex& pidx,
                                          start, end);
      _saveDP();
      _savePdf();
      _saveJpg();
      _saveSession();
      _openVideo();
      _toggleShowLiveCoord();
      _clearPlots();
      _clearTables();
      _launchScript(QAction *action);
      _plotAllVars();

      _startTimeChanged(double startTime);
      _liveTimeChanged(double liveTime);
      _stopTimeChanged(double stopTime);
      _liveTimeNext();
      _liveTimePrev();
      _monteInputsHeaderViewClicked(section);
      _monteInputsViewCurrentChanged(QModelIndex& currIdx,
                                         const QModelIndex& prevIdx);
      _bookViewCurrentChanged( QModelIndex& currIdx,
                                   QModelIndex& prevIdx);
      _bookModelDataChanged( QModelIndex &topLeft,
                                 QModelIndex &bottomRight,
                                 QVectorint &roles  QVectorint());

      setTimeFromVideo(double time);
      setTimeFromBvis(double time);
      _scriptError(QProcess::ProcessError ();
      _vsRead();
};

