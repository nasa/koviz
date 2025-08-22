#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QHash>
#include <QProcess>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScreen>
#include <QInputDialog>

#include "plotmainwindow.h"

PlotMainWindow::PlotMainWindow(PlotBookModel* bookModel,
        const QString& trickhost,
        uint trickport,
        double trickoffset,
        const QList<QPair<QString,double> >& videos,
        const QString& excludePattern,
        const QString& filterPattern,
        bool isFilterOutFlatlineZeros,
        const QString& scripts,
        bool isDebug,
        bool isPlotAllVars,
        const QStringList &timeNames,
        const QString &dpDir,
        const QStringList& dpFiles,
        bool isShowTables,
        QStringList unitOverrides,
        QString map,
        QString mapFile,
        Runs* runs,
        QStandardItemModel* varsModel,
        QWidget *parent) :
    QMainWindow(parent),
    _bookModel(bookModel),
    _sieModel(0),
    _tvModel(0),
    _trickhost(trickhost),
    _trickport(trickport),
    _trickoffset(trickoffset),
    _videos(videos),
    _excludePattern(excludePattern),
    _filterPattern(filterPattern),
    _isFilterOutFlatlineZeros(isFilterOutFlatlineZeros),
    _scripts(scripts),
    _isDebug(isDebug),
    _timeNames(timeNames),
    _dpDir(dpDir),
    _dpFiles(dpFiles),
    _isShowTables(isShowTables),
    _unitOverrides(unitOverrides),
    _map(map),
    _mapFile(mapFile),
    _runs(runs),
    _varsModel(varsModel),
    _monteInputsView(0),
    _dpTreeWidget(0),
    _trickView(0),
    vidView(0)
{
    // Set version dependent string split flag
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        _skipEmptyParts = Qt::SkipEmptyParts;
    #else
        _skipEmptyParts = QString::SkipEmptyParts;
    #endif

    // Window title
    QModelIndex titlesIdx = _bookModel->getIndex(QModelIndex(),
                                                 "DefaultPageTitles");
    QString title1 = _bookModel->getDataString(titlesIdx,
                                               "Title1",
                                               "DefaultPageTitles");
    if ( !title1.isEmpty() ) {
        // Set window title to -t1 title
        setWindowTitle(title1);
    } else {
        setWindowTitle(tr("koviz!"));
    }

    createMenu();

    // Central Widget and main layout
    QSplitter* msplit = new QSplitter;
    setCentralWidget(msplit);
    QFrame* lframe = new QFrame(msplit);
    QGridLayout* lgrid = new QGridLayout(lframe);
    QSplitter* lsplit = new QSplitter(lframe);
    lsplit->setOrientation(Qt::Vertical);
    lgrid->addWidget(lsplit,0,0);

    // Create Plot Tabbed Notebook View Widget
    _bookView = new BookView();
    _bookView->setModel(_bookModel);
    connect(_bookView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(_bookViewCurrentChanged(QModelIndex,QModelIndex)));
    connect(_bookModel,
            SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this,
            SLOT(_bookModelRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(_bookModel,
            SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this,
            SLOT(_bookModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));

    msplit->addWidget(_bookView);

    // Monte inputs view (widget added later)
    _monteInputsView = new MonteInputsView(runs,lsplit);
    _monteInputsView->setModel(runs->runsModel());
    connect(_monteInputsView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(_monteInputsViewCurrentChanged(QModelIndex,QModelIndex)));
    _monteInputsHeaderView = _monteInputsView->horizontalHeader();
    connect(_monteInputsHeaderView,SIGNAL(sectionClicked(int)),
            this,SLOT(_monteInputsHeaderViewClicked(int)));
    connect(runs, SIGNAL(runsRefreshed()),
            this, SLOT(_runsRefreshed()));

    if ( _isDebug ) {
        _plotTreeView = new QTreeView(lsplit);
        _plotTreeView->setModel(_bookModel);
    }

    _statusBar = new QStatusBar(this);
    this->setStatusBar(_statusBar);
    _statusBar->showMessage("");

    // Vars/DP Notebook
    _nbDPVars = new QTabWidget(lsplit);
    _nbDPVars->setFocusPolicy(Qt::ClickFocus);
    lsplit->addWidget(_nbDPVars);
    _nbDPVars->setAttribute(Qt::WA_AlwaysShowToolTips, false);

    // Vars Tab
    QFrame* varsFrame = new QFrame(lsplit);
    _varsWidget = new VarsWidget(_timeNames.at(0),
                                 _runs,
                                 _unitOverrides,
                                 _bookModel,
                                 _bookView->selectionModel(),
                                 _monteInputsView,
                                 varsFrame);
    if ( isPlotAllVars ) {
        _varsWidget->selectAllVars();
        _varsWidget->clearSelection();
    }
    _nbDPVars->addTab(varsFrame,"Vars");

    // Sie and TV models!
    if ( _trickport ) {
        _sieModel = new SieListModel(_trickhost, _trickport);
        _tvModel = new TVModel(_trickhost,_trickport);
    }

    // DP Tab
    // Due to what I think is a Qt bug, the DPTreeWidget which is placed
    // in this tab is created when the DP tab is clicked.
    // See PlotMainWindow::_nbCurrentChanged()
    _dpFrame = new QFrame(lsplit);
    _nbDPVars->addTab(_dpFrame,"DP");
    if ( ! _dpFiles.isEmpty() ) {
        // DP files specified on commandline
        _dpTreeWidget = new  DPTreeWidget(_timeNames.at(0), _dpDir,
                                          _dpFiles,
                                          _runs,
                                          _bookModel,
                                          _bookView->selectionModel(),
                                          _monteInputsView,
                                          _sieModel,
                                          _tvModel,
                                          _isShowTables,
                                          _unitOverrides,
                                          _dpFrame);
        _nbDPVars->setCurrentIndex(1);
    }
    connect(_nbDPVars,SIGNAL(currentChanged(int)),
            this,SLOT(_nbCurrentChanged(int)));

    // Runs Tab
    QFrame* runsFrame = new QFrame(lsplit);
    _runsWidget = new RunsWidget(_runs,
                                 _bookModel,
                                 _monteInputsView,
                                 runsFrame);
    _nbDPVars->addTab(runsFrame,"Runs");
    if (  _runs->runPaths().isEmpty() ) {
        // If user specified no commandline runs, show RUNs panel
        _nbDPVars->setCurrentIndex(2);
    }


    // Vars/DP needs monteInputsView, but needs to be added after Vars/DP
    if ( runs->runsModel() ) {
        lsplit->addWidget(_monteInputsView);
    }

    // Trick view tab
    if ( _trickport ) {
        _tvFrame = new QFrame(lsplit);
        _nbDPVars->addTab(_tvFrame,"TV");
        _nbDPVars->setCurrentWidget(_tvFrame);
        _trickView = new TrickView(_sieModel,
                                   _tvModel,
                                   _bookModel,
                                   _bookView->selectionModel(),
                                   _tvFrame);
        connect(_bookView, SIGNAL(signalDropEvent(QDropEvent*,QModelIndex)),
                _trickView,SLOT(slotDropEvent(QDropEvent*,QModelIndex)));
    }

    // Start/Live/Stop times input
    _timeInput = new TimeInput(this);
    _timeInput->hideLiveTime();
    QSizePolicy sp(QSizePolicy::Preferred,QSizePolicy::Preferred);
    _timeInput->setSizePolicy(sp);
    lsplit->addWidget(_timeInput);
    lsplit->setStretchFactor(0,1);
    lsplit->setStretchFactor(1,0);
    connect(_timeInput,SIGNAL(startTimeChanged(double)),
            this, SLOT(_startTimeChanged(double)));
    connect(_timeInput,SIGNAL(liveTimeChanged(double)),
            this, SLOT(_liveTimeChanged(double)));
    connect(_timeInput,SIGNAL(liveTimeNext()),
            this, SLOT(_liveTimeNext()));
    connect(_timeInput,SIGNAL(liveTimePrev()),
            this, SLOT(_liveTimePrev()));
    connect(_timeInput,SIGNAL(stopTimeChanged(double)),
            this, SLOT(_stopTimeChanged(double)));
    connect(_bookModel,
            SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            _timeInput,
            SLOT(_slotDataChanged(QModelIndex,QModelIndex,QVector<int>)));

    /*
     * To run with the ball2 sim, add the following to the input file:
     * trick.var_server_set_port(<_trickport>)
     */
    _vsSocket = new QTcpSocket();
    connect(_vsSocket,SIGNAL(readyRead()),
            this,SLOT(_vsRead()));
    _vsSocket->connectToHost(_trickhost,_trickport);
    if (_vsSocket->waitForConnected(500)) {
        fprintf(stderr,"Connected To The Trick Variable Server!\n");
        _vsSocket->write("trick.var_add(\"trick_sys.sched.time_tics\")\n");
    }

    // creating timecom to send commands to "the visualizer" (VIS)
    _the_visualizer = new TimeCom("127.0.0.1", 64052, this);
    connect(_the_visualizer,SIGNAL(timechangedByBvis(double)),
            this, SLOT(setTimeFromBvis(double)));

    // creating timecom to send commands to the koviz blender plugin
    _blender = new TimeCom("127.0.0.1", 64053, this);
    connect(_blender,SIGNAL(timechangedByBvis(double)),
            this, SLOT(setTimeFromBvis(double)));

    // sending run command if there is only one run
    if ( runs->runsModel()->rowCount() == 1 ) {
        QString rundir = QString("%1/").arg(
                    QDir::current().absoluteFilePath(
                        _runs->runPaths().at(0)));
        _the_visualizer->sendRun2Bvis(rundir);
        _blender->sendRun2Bvis(rundir);
    }

    bool showVideo = _bookModel->getDataBool(QModelIndex(),"ShowVideo","");
    if ( !_videos.isEmpty() && showVideo ) {
#ifndef HAS_MPV
        fprintf(stderr, "koviz [error]: koviz built without mpv video support!"
                        "  Please install mpv and rebuild koviz!\n");
        exit(-1);
#endif
        QPair<QString,double> video; // mp4name,timeoffset
        foreach ( video, _videos ) {
            QFileInfo fi(video.first);
            if ( !fi.isReadable() ) {
                fprintf(stderr, "koviz [error]: Cannot find or read video "
                                "\"%s\"\n",video.first.toLatin1().constData());
                exit(-1);
            }
        }
        _openVideos(_videos);

        selectFirstCurve();
    }

    if ( _isFilterOutFlatlineZeros ) {
        _filterOutFlatLines();
    }

    // Size main window
    QList<int> sizes;
    sizes << 420 << 1180;
    msplit->setSizes(sizes);
    msplit->setStretchFactor(0,0);
    msplit->setStretchFactor(1,1);

    // Read "INI" (may resize window)
    _readSettings();
}

void PlotMainWindow::setTimeFromVideo(double time) {
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    _bookModel->setData(liveIdx,time);
}

void PlotMainWindow::setTimeFromBvis(double time)
{
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    _bookModel->setData(liveIdx,time);
}

PlotMainWindow::~PlotMainWindow()
{
    _vsSocket->close();
    delete _vsSocket;

    delete _the_visualizer;
    delete _blender;
}

void PlotMainWindow::createMenu()
{
    _menuBar = new QMenuBar;
    _fileMenu = new QMenu(tr("&File"), this);
    _optsMenu = new QMenu(tr("&Options"), this);
    _pdfAction  = _fileMenu->addAction(tr("Save &PDF"));
    _jpgAction  = _fileMenu->addAction(tr("Save &JPG"));
    _dpAction  = _fileMenu->addAction(tr("Save &DP"));
    _sessionAction = _fileMenu->addAction(tr("Save &Session"));
#ifdef HAS_MPV
    _openVideoAction = _fileMenu->addAction(tr("Open &Video"));
#endif
    _exitAction = _fileMenu->addAction(tr("E&xit"));
    _showVideoAction = _optsMenu->addAction(tr("ShowVideo"));
        _showVideoAction->setCheckable(true);
        bool showVideo = _bookModel->getDataBool(QModelIndex(),"ShowVideo","");
        _showVideoAction->setChecked(showVideo);
    _videoDirAction = _optsMenu->addAction(tr("VideoDirectory"));
    _showLiveCoordAction = _optsMenu->addAction(tr("ShowLiveCoord"));
        _showLiveCoordAction->setCheckable(true);
        _showLiveCoordAction->setChecked(true);
    _markTimeAction = _optsMenu->addAction(tr("MarkTime"));
    _refreshPlotsAction  = _optsMenu->addAction(tr("RefreshPlots"));
    _clearPlotsAction  = _optsMenu->addAction(tr("ClearPlots"));
    _clearTablesAction = _optsMenu->addAction(tr("ClearTables"));
    _clearRunsAction = _optsMenu->addAction(tr("ClearRuns"));
    _plotAllVarsAction = _optsMenu->addAction(tr("PlotAllVars"));
    _enableDragDropAction = _optsMenu->addAction(tr("EnableDragAndDrop"));
    _enableDragDropAction->setCheckable(true);
    _filterOutFlatLinesAction = _optsMenu->addAction(
                                                  tr("FilterOutFlatlineZeros"));
    _selectRunsHomeAction = _optsMenu->addAction(tr("SelectRunsHome"));
    _menuBar->addMenu(_fileMenu);
    _menuBar->addMenu(_optsMenu);
    if ( !_scripts.isEmpty() ) {
        _scriptsMenu = new QMenu(tr("&Scripts"), this);
        QStringList scripts = _scripts.split(',',_skipEmptyParts);
        foreach ( QString script, scripts ) {
            QAction* action = _scriptsMenu->addAction(script);
            Q_UNUSED(action);
        }
        connect(_scriptsMenu,SIGNAL(triggered(QAction*)),
                this,SLOT(_launchScript(QAction*)));
        _menuBar->addMenu(_scriptsMenu);
    }
    connect(_dpAction, SIGNAL(triggered()),this, SLOT(_saveDP()));
    connect(_pdfAction, SIGNAL(triggered()),this, SLOT(_savePdf()));
    connect(_jpgAction, SIGNAL(triggered()),this, SLOT(_saveJpg()));
    connect(_sessionAction, SIGNAL(triggered()),this, SLOT(_saveSession()));
#ifdef HAS_MPV
    connect(_openVideoAction, SIGNAL(triggered()),
            this, SLOT(_openVideoByMenu()));
#endif
    connect(_exitAction, SIGNAL(triggered()),this, SLOT(close()));
    connect(_showVideoAction, SIGNAL(triggered()),
            this, SLOT(_toggleShowVideo()));
    connect(_videoDirAction, SIGNAL(triggered()),
            this, SLOT(_chooseVideoDirectory()));
    connect(_showLiveCoordAction, SIGNAL(triggered()),
            this, SLOT(_toggleShowLiveCoord()));
    connect(_markTimeAction, SIGNAL(triggered()),
            this, SLOT(_markTime()));
    connect(_refreshPlotsAction, SIGNAL(triggered()),
            this, SLOT(_refreshPlots()));
    connect(_clearPlotsAction, SIGNAL(triggered()),
            this, SLOT(_clearPlots()));
    connect(_clearTablesAction, SIGNAL(triggered()),
            this, SLOT(_clearTables()));
    connect(_clearRunsAction, SIGNAL(triggered()),
            this, SLOT(_clearRuns()));

    connect(_plotAllVarsAction, SIGNAL(triggered()),
            this, SLOT(_plotAllVars()));
    connect(_enableDragDropAction, SIGNAL(toggled(bool)),
            this, SLOT(_toggleEnableDragDrop(bool)));
    connect(_filterOutFlatLinesAction, SIGNAL(triggered()),
            this, SLOT(_filterOutFlatLines()));
    connect(_selectRunsHomeAction, SIGNAL(triggered()),
            this, SLOT(_selectRunsHome()));
    setMenuWidget(_menuBar);
}

void PlotMainWindow::_nbCurrentChanged(int i)
{
    if ( i == 1 && _dpTreeWidget == 0 ) {
        //
        // The reason this is here is to work around what I think
        // is a bug within Qt. If the DPTreeWidget is created
        // earlier, for some reason, this warning message is issued:
        // QSortFilterProxyModel: index from wrong model passed to mapFromSource
        // I believe that, in a rare case, this causes a core dump.
        //
        // So instead of creating the DPTreeWidget alongside the VarsWidget,
        // the DPTreeWidget is created when the DP tab is clicked.
        //
        _dpTreeWidget = new  DPTreeWidget(_timeNames.at(0), _dpDir, _dpFiles,
                                          _runs,
                                          _bookModel,
                                          _bookView->selectionModel(),
                                          _monteInputsView,
                                          _sieModel,
                                          _tvModel,
                                          _isShowTables,
                                          _unitOverrides,
                                          _dpFrame);
    }
}

void PlotMainWindow::_bookViewCurrentChanged(const QModelIndex &currIdx,
                                             const QModelIndex &prevIdx)
{
    Q_UNUSED(prevIdx);

    if ( _monteInputsView ) {
        if ( _bookModel->isIndex(currIdx,"Curve") ) {
            // Make row current in monte inputs view that goes with bview curve
            int runId = _bookModel->getDataInt(currIdx,"CurveRunID","Curve");
            int rc = _monteInputsView->model()->rowCount();
            for (int i = 0; i < rc; ++i ) {
                // note: next line assumes that run is in column 0
                QModelIndex idx = _monteInputsView->model()->index(i,0);
                if ( runId == _monteInputsView->model()->data(idx).toInt() ) {
                    _monteInputsView->setCurrentIndex(idx);
                    break;
                }
            }
        }
    }

    if ( !currIdx.isValid() ) {
        _timeInput->hideLiveTime();
        if ( _monteInputsView ) {
            // Clicked into whitespace in a CurvesView,
            // so clear current run in monte input view and all CurveViews
            QModelIndex invalidIdx;
            _monteInputsView->setCurrentIndex(invalidIdx);
            _bookView->setCurrentCurveRunID(-1);
        }
    } else {
        bool isShowLiveTime = false;
        if ( _bookModel->isIndex(currIdx,"Curve") ) {
            isShowLiveTime = true;

            QStringList list;

            QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
            bool ok;
            double liveTime = _bookModel->data(liveIdx).toDouble(&ok);
            if ( ok ) {
                QString el0 = QString("t=%1").arg(liveTime);
                list.append(el0);
            }

            CurveModel* curveModel = _bookModel->getCurveModel(currIdx);
            curveModel->map();
            ModelIterator* it = curveModel->begin();
            int n = curveModel->rowCount();
            if ( n > 0 ) {
                double begin_time = it->at(0)->t();
                double end_time = it->at(n-1)->t();
                QString el1 = QString("begin_time=%1").arg(begin_time);
                QString el2 = QString("end_time=%1").arg(end_time);
                list.append(el1);
                list.append(el2);
                _blender->sendList2Bvis(list);
            }
            delete it;
            curveModel->unmap();
        } else if ( _bookModel->isIndex(currIdx,"Plot") ) {
            if (_bookModel->getDataString(currIdx,"PlotPresentation")=="error"){
                isShowLiveTime = true;
            }
        }
        if ( isShowLiveTime ) {
            _timeInput->showLiveTime();
        }
    }
}

void PlotMainWindow::_bookModelDataChanged(const QModelIndex &topLeft,
                                           const QModelIndex &bottomRight,
                                           const QVector<int> &roles)
{
    Q_UNUSED(roles);
    if ( topLeft != bottomRight ) return;
    const QAbstractItemModel* model = topLeft.model();
    if ( !model ) return;
    if ( topLeft.column() != 1 ) return;

    QString tag = model->data(topLeft.sibling(topLeft.row(),0)).toString();
    if ( tag == "LiveCoordTime" ) {
        bool ok;
        double liveTime = model->data(topLeft).toDouble(&ok);
        if ( ok ) {
            if ( vidView ) {
                vidView->seek_time(liveTime);
            }
            _the_visualizer->sendTime2Bvis(liveTime);
            _blender->sendTime2Bvis(liveTime);
        }
    } else if ( tag == "StatusBarMessage" ) {
        QString msg = _bookModel->data(topLeft).toString();
        _statusBar->showMessage(msg);
    }
}

void PlotMainWindow::vidViewClosed()
{
    bool showVideo = _bookModel->getDataBool(QModelIndex(),"ShowVideo","");
    if ( showVideo ) {
        QModelIndex showVideoIdx = _bookModel->getDataIndex(QModelIndex(),
                                                            "ShowVideo","");
        _bookModel->setData(showVideoIdx,false);
        _showVideoAction->setChecked(false);
    }
    vidView = 0;
}

void PlotMainWindow::_scriptError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    QMessageBox msgBox;
    QString msg = QString("Error launching user script!");
    msgBox.setText(msg);
    msgBox.exec();
}

void PlotMainWindow::_bookModelRowsAboutToBeRemoved(const QModelIndex &pidx,
                                                    int start, int end)
{
    Q_UNUSED(pidx);
    Q_UNUSED(start);
    Q_UNUSED(end);
    _varsWidget->clearSelection();
    if ( _dpTreeWidget ) {
        _dpTreeWidget->clearSelection();
    }
}

bool PlotMainWindow::_isRUN(const QString &fp)
{
    QFileInfo fi(fp);
    return ( fi.baseName().left(4) == "RUN_" && fi.isDir() ) ;
}

bool PlotMainWindow::_isMONTE(const QString &fp)
{
    QFileInfo fi(fp);
    return ( fi.baseName().left(6) == "MONTE_" && fi.isDir() ) ;
}

void PlotMainWindow::_saveDP()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 QString("Save DP"),
                                                 QString(""),
                                                 tr("files (DP*)"));

    if ( fname.isEmpty() ) {
        return;
    }

    QString i1("  ");
    QString i2("    ");
    QString i3("      ");
    QString i4("        ");

    // Open DP file for writing
    QFile dp(fname);
    if (!dp.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz: [error] could not open %s\n",
                fname.toLatin1().constData());
        return;
    }
    QTextStream out(&dp);

    // Title
    out << "Koviz\n\n";

    // Plots
    out << "PLOTS:\n";

    // Pages
    int pageNum = 1;
    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        QString pageTitle = _bookModel->getDataString(pageIdx,
                                                      "PageTitle","Page");
        if ( pageTitle == "Koviz" ) {
            pageTitle.clear();
        }
        out << i1 << "Page " << pageNum << ":" << " \"" << pageTitle << "\"\n";

        double pageStart = _bookModel->getDataDouble(pageIdx,
                                                     "PageStartTime","Page");
        if ( pageStart != -DBL_MAX ) {
            out << i2 << "start: " << pageStart << "\n";
        }

        double pageStop = _bookModel->getDataDouble(pageIdx,
                                                    "PageStopTime","Page");
        if ( pageStop != DBL_MAX ) {
            out << i2 << "stop: " << pageStop << "\n";
        }

        QString pageBG = _bookModel->getDataString(pageIdx,
                                                   "PageBackgroundColor",
                                                   "Page");
        if ( pageBG != "#FFFFFF") {
            out << i2 << "background_color: \"" << pageBG << "\"\n";
        }

        QString pageFG = _bookModel->getDataString(pageIdx,
                                                   "PageForegroundColor",
                                                   "Page");
        if ( pageFG != "#000000") {
            out << i2  << "foreground_color: \"" << pageFG << "\"\n";
        }

        out << "\n";

        int plotNum = 1;
        QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
        foreach ( QModelIndex plotIdx, plotIdxs ) {
            QString plotTitle = _bookModel->getDataString(plotIdx,
                                                          "PlotTitle","Plot");
            out << i2 << "Plot " << plotNum << ":"
                << " \"" << plotTitle << "\"\n";

            double plotStart = _bookModel->getDataDouble(plotIdx,
                                                         "PlotStartTime",
                                                         "Plot");
            if ( plotStart != -DBL_MAX ) {
                out << i3 << "start: " << plotStart << "\n";
            }

            double plotStop = _bookModel->getDataDouble(plotIdx,
                                                        "PlotStopTime","Plot");
            if ( plotStop != DBL_MAX ) {
                out << i3 << "stop: " << plotStop << "\n";
            }

            double plotXMin = _bookModel->getDataDouble(plotIdx,
                                                        "PlotXMinRange","Plot");
            if ( plotXMin != -DBL_MAX ) {
                out << i3 << "x_min_range: " << plotXMin << "\n";
            }

            double plotXMax = _bookModel->getDataDouble(plotIdx,
                                                        "PlotXMaxRange","Plot");
            if ( plotXMax != DBL_MAX ) {
                out << i3 << "x_max_range: " << plotXMax << "\n";
            }

            double plotYMin = _bookModel->getDataDouble(plotIdx,
                                                        "PlotYMinRange","Plot");
            if ( plotYMin != -DBL_MAX ) {
                out << i3 << "y_min_range: " << plotYMin << "\n";
            }

            double plotYMax = _bookModel->getDataDouble(plotIdx,
                                                        "PlotYMaxRange","Plot");
            if ( plotYMax != DBL_MAX ) {
                out << i3 << "y_max_range: " << plotYMax << "\n";
            }

            QString plotBG = _bookModel->getDataString(plotIdx,
                                                       "PlotBackgroundColor",
                                                       "Plot");
            if ( plotBG != "#FFFFFF") {
                out << i3 << "background_color: \"" << plotBG << "\"\n";
            }

            QString plotFG = _bookModel->getDataString(plotIdx,
                                                       "PlotForegroundColor",
                                                       "Plot");
            if ( plotFG != "#000000" ) {
                out << i3 << "foreground_color: \"" << plotFG << "\"\n";
            }

            QString plotXScale = _bookModel->getDataString(plotIdx,
                                                           "PlotXScale",
                                                           "Plot");
            if ( plotXScale != "linear" ) {
                out << i3 << "plot_x_scale: \"" << plotXScale << "\"\n";
            }

            QString plotYScale = _bookModel->getDataString(plotIdx,
                                                           "PlotYScale",
                                                           "Plot");
            if ( plotYScale != "linear" ) {
                out << i3 << "plot_y_scale: \"" << plotYScale << "\"\n";
            }

            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves","Plot");
            QModelIndexList curveIdxs = _bookModel->curveIdxs(curvesIdx);

            QString xAxisLabel = _bookModel->getDataString(plotIdx,
                                                           "PlotXAxisLabel",
                                                           "Plot");
            bool isXAxisLabel = true;
            foreach ( QModelIndex curveIdx, curveIdxs ) {
                QString xName = _bookModel->getDataString(curveIdx,
                                                          "CurveXName",
                                                          "Curve");
                if ( xName == xAxisLabel ||
                     (xName == "sys.exec.out.time" && xAxisLabel == "Time")) {
                    isXAxisLabel = false;
                    break;
                }
            }
            if ( isXAxisLabel ) {
                out << i3 << "x_axis_label: \"" << xAxisLabel << "\"\n" ;
            }

            QString yAxisLabel = _bookModel->getDataString(plotIdx,
                                                           "PlotYAxisLabel",
                                                           "Plot");
            bool isYAxisLabel = true;
            foreach ( QModelIndex curveIdx, curveIdxs ) {
                QString yName = _bookModel->getDataString(curveIdx,
                                                          "CurveYName",
                                                          "Curve");
                if ( yName == yAxisLabel ) {
                    isYAxisLabel = false;
                    break;
                }
            }
            if ( isYAxisLabel ) {
                out << i3 << "y_axis_label: \"" << yAxisLabel << "\"\n" ;
            }

            QStringList ynames;
            QString lastXName;
            foreach ( QModelIndex curveIdx, curveIdxs ) {
                QString xName = _bookModel->getDataString(curveIdx,
                                                          "CurveXName",
                                                          "Curve");
                QString yName = _bookModel->getDataString(curveIdx,
                                                          "CurveYName",
                                                          "Curve");
                if ( ynames.contains(yName) ) {
                    continue;
                }
                ynames << yName;

                if ( lastXName != xName ) {

                    out << i3 << "x_variable: \"" << xName << "\"\n";

                    QString xUnit = _bookModel->getDataString(curveIdx,
                                                              "CurveXUnit",
                                                              "Curve");
                    if ( !xUnit.isEmpty() ) {
                        out << i4 << "units: \"" << xUnit << "\"\n";
                    }

                    double curveXMin = _bookModel->getDataDouble(curveIdx,
                                                               "CurveXMinRange",
                                                               "Curve");
                    if ( curveXMin != -DBL_MAX ) {
                        out << i4 << "min_range: " << curveXMin << "\n";
                    }

                    double curveXMax = _bookModel->getDataDouble(curveIdx,
                                                               "CurveXMaxRange",
                                                               "Curve");
                    if ( curveXMax != DBL_MAX ) {
                        out << i4 << "max_range: " << curveXMax << "\n";
                    }

                    double curveXScale = _bookModel->getDataDouble(curveIdx,
                                                                  "CurveXScale",
                                                                   "Curve");
                    if ( curveXScale != 1.0 ) {
                        out << i4 << "scale_factor: " << curveXScale << "\n";
                    }

                    double curveXBias = _bookModel->getDataDouble(curveIdx,
                                                                  "CurveXBias",
                                                                  "Curve");
                    if ( curveXBias != 0.0 ) {
                        out << i4 << "bias: " << curveXBias << "\n";
                    }

                }

                out << i3 << "y_variable: \"" << yName << "\"\n";

                QString yUnit = _bookModel->getDataString(curveIdx,
                                                          "CurveYUnit",
                                                          "Curve");
                if ( !yUnit.isEmpty() ) {
                    out << i4 << "units: \"" << yUnit << "\"\n";
                }

                double curveYMin = _bookModel->getDataDouble(curveIdx,
                                                             "CurveYMinRange",
                                                             "Curve");
                if ( curveYMin != -DBL_MAX ) {
                    out << i4 << "min_range: " << curveYMin << "\n";
                }

                double curveYMax = _bookModel->getDataDouble(curveIdx,
                                                             "CurveYMaxRange",
                                                             "Curve");
                if ( curveYMax != DBL_MAX ) {
                    out << i4 << "max_range: " << curveYMax << "\n";
                }

                double curveYScale = _bookModel->getDataDouble(curveIdx,
                                                               "CurveYScale",
                                                               "Curve");
                if ( curveYScale != 1.0 ) {
                    out << i4 << "scale_factor: " << curveYScale << "\n";
                }

                double curveYBias = _bookModel->getDataDouble(curveIdx,
                                                              "CurveYBias",
                                                              "Curve");
                if ( curveYBias != 0.0 ) {
                    out << i4 << "bias: " << curveYBias << "\n";
                }

                QString symbolStyle = _bookModel->getDataString(curveIdx,
                                                             "CurveSymbolStyle",
                                                             "Curve");
                if ( !symbolStyle.isEmpty() ) {
                    out << i4 << "symbol_style: \"" << symbolStyle << "\"\n";
                }

                QString symbolEnd = _bookModel->getDataString(curveIdx,
                                                             "CurveSymbolEnd",
                                                             "Curve");
                if ( !symbolEnd.isEmpty() ) {
                    out << i4 << "symbol_end: \"" << symbolEnd << "\"\n";
                }

                QString symbolSize = _bookModel->getDataString(curveIdx,
                                                              "CurveSymbolSize",
                                                              "Curve");
                if ( !symbolSize.isEmpty() ) {
                    out << i4 << "symbol_size: \"" << symbolSize << "\"\n";
                }

                QString lineStyle = _bookModel->getDataString(curveIdx,
                                                              "CurveLineStyle",
                                                              "Curve");
                if ( !lineStyle.isEmpty() && lineStyle != "plain" ) {
                    out << i4 << "line_style: \"" << lineStyle << "\"\n";
                }

                // Only do color if it is a named color (not #NNNNNN)
                QString lineColor = _bookModel->getDataString(curveIdx,
                                                              "CurveColor",
                                                              "Curve");
                if ( !lineColor.isEmpty() && !lineColor.startsWith("#")) {
                    out << i4 << "line_color: \"" << lineColor << "\"\n";
                }

                QString yLabel = _bookModel->getDataString(curveIdx,
                                                           "CurveYLabel",
                                                           "Curve");
                if ( !yLabel.isEmpty() ) {
                    out << i4 << "label: \"" << yLabel << "\"\n";
                }

                lastXName = xName;
            }

            out << "\n";
            ++plotNum;
        }
        out << "\n";

        ++pageNum;
    }

    // Tables
#if 0
    int tableNum = 1;
    QModelIndex tablesIdx = _bookModel->getIndex(QModelIndex(),"Tables");
    QModelIndexList tableIdxs = _bookModel->getIndexList(tablesIdx,"Table");
    // TODO Save DP Tables
    foreach ( QModelIndex tableIdx, tableIdxs ) {
        if ( tableNum == 1 ) {
            out << "TABLES:\n";
        }
    }
#endif

    dp.close();
}

void PlotMainWindow::savePdf(const QString& fname)
{
    if ( ! fname.isEmpty() ) {
        _bookView->savePdf(fname);
    }
}

void PlotMainWindow::selectFirstCurve()
{
    if ( _bookModel->isChildIndex(QModelIndex(),"","Pages") ) {
        QModelIndex pagesIdx = _bookModel->getIndex(QModelIndex(),
                                                    "Pages","");
        if ( _bookModel->isChildIndex(pagesIdx, "Pages", "Page") ) {
            QModelIndexList pageIdxs = _bookModel->getIndexList(pagesIdx,
                                                                "Page","Pages");
            QModelIndex pageIdx0 = pageIdxs.at(0);
            if ( _bookModel->isChildIndex(pageIdx0,"Page", "Plots") ) {
                QModelIndex plotsIdx = _bookModel->getIndex(pageIdx0,
                                                            "Plots","Page");
                if ( _bookModel->isChildIndex(plotsIdx,"Plots","Plot")) {
                    QModelIndexList plotIdxs = _bookModel->getIndexList(
                                plotsIdx,
                                "Plot","Plots");
                    QModelIndex plotIdx0 = plotIdxs.at(0);
                    if ( _bookModel->isChildIndex(plotIdx0,
                                                  "Plot","Curves")) {
                        QModelIndex curvesIdx = _bookModel->getIndex(
                                    plotIdx0,
                                    "Curves","Plot");
                        if ( _bookModel->isChildIndex(curvesIdx,
                                                      "Curves","Curve")) {
                            QModelIndexList curveIdxs = _bookModel->
                                                         getIndexList(curvesIdx,
                                                              "Curve","Curves");
                            QModelIndex curveIdx0 = curveIdxs.at(0);
                            QModelIndex runIdx = _bookModel->getIndex(
                                                         curveIdx0,
                                                         "CurveRunID", "Curve");
                            int id = _bookModel->data(runIdx).toInt();
                            _bookView->setCurrentCurveRunID(id);
                        }
                    }
                }
            }
        }
    }
}

void PlotMainWindow::_savePdf()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 QString("Save PDF"),
                                                 QString(""),
                                                 tr("files (*.pdf)"));

    if ( ! fname.isEmpty() ) {

        int ret = QMessageBox::Save;
        if ( !fname.endsWith(".pdf") ) {
            fname += ".pdf";
            QFileInfo fi(fname);
            if ( fi.exists() ) {
                QMessageBox msgBox;
                msgBox.setStandardButtons(QMessageBox::Save|
                                          QMessageBox::Cancel);
                QString msg;
                msg = "Overwrite: " + fi.fileName() + "?";
                msgBox.setText(msg);
                msgBox.setDefaultButton(QMessageBox::Save);
                ret = msgBox.exec();
            }
        }

        if ( ret == QMessageBox::Save ) {
            //QString fname = "/users/kvetter/dev/dog.pdf";
            _bookView->savePdf(fname);
            /*
            QString program = "evince";
            QStringList arguments;
            arguments << fname;
            QProcess *myProcess = new QProcess(this);
            myProcess->start(program, arguments);

            QFileInfo fi(fname);
            double l = log10(fi.size());
            if ( l >= 3 && l < 6 ) {
                fprintf(stderr,"dog.pdf.size=%lf K\n", fi.size()/1000);
            } else if ( l >= 6 && l < 9 ) {
                fprintf(stderr,"dog.pdf.size=%lf MB\n", fi.size()/1000000);
            } else {
                fprintf(stderr,"dog.pdf.size=%lf bytes\n", fi.size());
            }
            */
        }
    }
}

void PlotMainWindow::_saveJpg()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 QString("Save JPG"),
                                                 QString(""),
                                                 tr("files (*.jpg)"));

    if ( ! fname.isEmpty() ) {

        int ret = QMessageBox::Save;
        if ( !fname.endsWith(".jpg") ) {
            fname += ".jpg";
            QFileInfo fi(fname);
            if ( fi.exists() ) {
                QMessageBox msgBox;
                msgBox.setStandardButtons(QMessageBox::Save|
                                          QMessageBox::Cancel);
                QString msg;
                msg = "Overwrite: " + fi.fileName() + "?";
                msgBox.setText(msg);
                msgBox.setDefaultButton(QMessageBox::Save);
                ret = msgBox.exec();
            }
        }

        if ( ret == QMessageBox::Save ) {
            _bookView->saveJpg(fname);
        }
    }
}

void PlotMainWindow::_saveSession()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 QString("Save Session"),
                                                 QString("session_"),
                                                 tr("files (session*)"));

    if ( ! fname.isEmpty() ) {

        // Open trk file for writing
        QFile f(fname);
        if (!f.open(QIODevice::WriteOnly)) {
            fprintf(stderr,"koviz: [error] could not open %s\n",
                    fname.toLatin1().constData());
            exit(-1);
        }
        QTextStream out(&f);

        // Session Title
        QFileInfo fi(".");
        QString userName = fi.owner();
        QDate date = QDate::currentDate();
        QString fmt("MMMM d, yyyy");
        QString dateStr = date.toString(fmt);
        QString sessionTitle = "Session - " + userName + " " + dateStr ;
        out << sessionTitle;
        out << "\n\n";

        // RUNs
        foreach ( QString run, _runs->runPaths() ) {
            out << "RUN: " << run << "\n";
        }

        // DPs
        foreach ( QString dp, _dpFiles ) {
            out << "PRODUCT: " << dp << "\n";
        }

        // Title1
        QModelIndex titlesIdx = _bookModel->getIndex(QModelIndex(),
                                                     "DefaultPageTitles");
        QString title1 = _bookModel->getDataString(titlesIdx,
                                                   "Title1",
                                                   "DefaultPageTitles");
        if ( !title1.startsWith("koviz") ) {
            out << "t1: " << title1 << "\n";
        } else {
            out << "t1: \"\"\n";
        }

        // Title2
        QString title2 = _bookModel->getDataString(titlesIdx,
                                                   "Title2",
                                                   "DefaultPageTitles");
        if ( !title2.contains('\n') ) {
            out << "t2: " << title2 << "\n";
        } else {
            out << "t2: \"\"\n";
        }

        // Title3
        QString title3 = _bookModel->getDataString(titlesIdx,
                                                   "Title3",
                                                   "DefaultPageTitles");
        if ( !title3.startsWith("User:") ) {
             out << "t3: " << title3 << "\n";
        } else {
            out << "t3: \"\"\n";
        }

        // Title4
        QString title4 = _bookModel->getDataString(titlesIdx,
                                                   "Title4",
                                                   "DefaultPageTitles");
        if ( !title4.startsWith("Date:") ) {
             out << "t4: " << title4 << "\n";
        } else {
            out << "t4: \"\"\n";
        }

        // Foreground
        QString fg = _bookModel->getDataString(QModelIndex(),
                                               "ForegroundColor","");
        if ( !fg.isEmpty() ) {
             out << "fg: " << fg << "\n";
        }

        // Background
        QString bg = _bookModel->getDataString(QModelIndex(),
                                               "BackgroundColor","");
        if ( !bg.isEmpty() ) {
             out << "bg: " << bg << "\n";
        }

        // RUN/log Exclude and Filter patterns
        if ( !_excludePattern.isEmpty() ) {
            out << "exclude:" << _excludePattern << "\n";
        }
        if ( !_filterPattern.isEmpty() ) {
            out << "filter:" << _filterPattern << "\n";
        }

        // Time match tolerance
        double tmt = _bookModel->getDataDouble(QModelIndex(),
                                               "TimeMatchTolerance","");
        out << "time_match_tolerance:" << tmt << "\n";

        // Legend/Curve Colors
        QModelIndex clrIdx = _bookModel->getIndex(QModelIndex(),
                                                  "LegendColors","");
        QString c1 = _bookModel->getDataString(clrIdx,"Color1","LegendColors");
        QString c2 = _bookModel->getDataString(clrIdx,"Color2","LegendColors");
        QString c3 = _bookModel->getDataString(clrIdx,"Color3","LegendColors");
        QString c4 = _bookModel->getDataString(clrIdx,"Color4","LegendColors");
        QString c5 = _bookModel->getDataString(clrIdx,"Color5","LegendColors");
        QString c6 = _bookModel->getDataString(clrIdx,"Color6","LegendColors");
        QString c7 = _bookModel->getDataString(clrIdx,"Color7","LegendColors");
        out << "c1: \"" << c1 << "\"\n" ;
        out << "c2: \"" << c2 << "\"\n" ;
        out << "c3: \"" << c3 << "\"\n" ;
        out << "c4: \"" << c4 << "\"\n" ;
        out << "c5: \"" << c5 << "\"\n" ;
        out << "c6: \"" << c6 << "\"\n" ;
        out << "c7: \"" << c7 << "\"\n" ;

        // Legend labels
        QModelIndex legIdx = _bookModel->getIndex(QModelIndex(),"LegendLabels");
        QString l1 = _bookModel->getDataString(legIdx,"Label1","LegendLabels");
        QString l2 = _bookModel->getDataString(legIdx,"Label2","LegendLabels");
        QString l3 = _bookModel->getDataString(legIdx,"Label3","LegendLabels");
        QString l4 = _bookModel->getDataString(legIdx,"Label4","LegendLabels");
        QString l5 = _bookModel->getDataString(legIdx,"Label5","LegendLabels");
        QString l6 = _bookModel->getDataString(legIdx,"Label6","LegendLabels");
        QString l7 = _bookModel->getDataString(legIdx,"Label7","LegendLabels");
        out << "l1: \"" << l1 << "\"\n" ;
        out << "l2: \"" << l2 << "\"\n" ;
        out << "l3: \"" << l3 << "\"\n" ;
        out << "l4: \"" << l4 << "\"\n" ;
        out << "l5: \"" << l5 << "\"\n" ;
        out << "l6: \"" << l6 << "\"\n" ;
        out << "l7: \"" << l7 << "\"\n" ;

        // Linestyles
        QModelIndex lsIdx = _bookModel->getIndex(QModelIndex(),"Linestyles");
        QString ls1 =_bookModel->getDataString(lsIdx,"Linestyle1","Linestyles");
        QString ls2 =_bookModel->getDataString(lsIdx,"Linestyle2","Linestyles");
        QString ls3 =_bookModel->getDataString(lsIdx,"Linestyle3","Linestyles");
        QString ls4 =_bookModel->getDataString(lsIdx,"Linestyle4","Linestyles");
        QString ls5 =_bookModel->getDataString(lsIdx,"Linestyle5","Linestyles");
        QString ls6 =_bookModel->getDataString(lsIdx,"Linestyle6","Linestyles");
        QString ls7 =_bookModel->getDataString(lsIdx,"Linestyle7","Linestyles");
        out << "ls1: \"" << ls1 << "\"\n" ;
        out << "ls2: \"" << ls2 << "\"\n" ;
        out << "ls3: \"" << ls3 << "\"\n" ;
        out << "ls4: \"" << ls4 << "\"\n" ;
        out << "ls5: \"" << ls5 << "\"\n" ;
        out << "ls6: \"" << ls6 << "\"\n" ;
        out << "ls7: \"" << ls7 << "\"\n" ;

        // Symbolstyles
        QModelIndex ssIdx = _bookModel->getIndex(QModelIndex(),"Symbolstyles");
        QString s1 =_bookModel->getDataString(ssIdx,"Symbolstyle1",
                                                    "Symbolstyles");
        QString s2 =_bookModel->getDataString(ssIdx,"Symbolstyle2",
                                                    "Symbolstyles");
        QString s3 =_bookModel->getDataString(ssIdx,"Symbolstyle3",
                                                    "Symbolstyles");
        QString s4 =_bookModel->getDataString(ssIdx,"Symbolstyle4",
                                                    "Symbolstyles");
        QString s5 =_bookModel->getDataString(ssIdx,"Symbolstyle5",
                                                    "Symbolstyles");
        QString s6 =_bookModel->getDataString(ssIdx,"Symbolstyle6",
                                                    "Symbolstyles");
        QString s7 =_bookModel->getDataString(ssIdx,"Symbolstyle7",
                                                    "Symbolstyles");
        out << "s1: \"" << s1 << "\"\n" ;
        out << "s2: \"" << s2 << "\"\n" ;
        out << "s3: \"" << s3 << "\"\n" ;
        out << "s4: \"" << s4 << "\"\n" ;
        out << "s5: \"" << s5 << "\"\n" ;
        out << "s6: \"" << s6 << "\"\n" ;
        out << "s7: \"" << s7 << "\"\n" ;

        // Groups
        QModelIndex groupsIdx = _bookModel->getIndex(QModelIndex(),"Groups");
        QString g1 =_bookModel->getDataString(groupsIdx,"Group1","Groups");
        QString g2 =_bookModel->getDataString(groupsIdx,"Group2","Groups");
        QString g3 =_bookModel->getDataString(groupsIdx,"Group3","Groups");
        QString g4 =_bookModel->getDataString(groupsIdx,"Group4","Groups");
        QString g5 =_bookModel->getDataString(groupsIdx,"Group5","Groups");
        QString g6 =_bookModel->getDataString(groupsIdx,"Group6","Groups");
        QString g7 =_bookModel->getDataString(groupsIdx,"Group7","Groups");
        out << "g1: \"" << g1 << "\"\n" ;
        out << "g2: \"" << g2 << "\"\n" ;
        out << "g3: \"" << g3 << "\"\n" ;
        out << "g4: \"" << g4 << "\"\n" ;
        out << "g5: \"" << g5 << "\"\n" ;
        out << "g6: \"" << g6 << "\"\n" ;
        out << "g7: \"" << g7 << "\"\n" ;

        // Show Legend
        QString isLegend = _bookModel->getDataString(QModelIndex(),"IsLegend");
        out << "legend: " << isLegend << "\n";

        // Orientation
        QString orient = _bookModel->getDataString(QModelIndex(),"Orientation");
        out << "orient: " << orient << "\n";

        // Presentation
        QString pres = _bookModel->getDataString(QModelIndex(),"Presentation");
        if ( pres.isEmpty() ) {
            out << "presentation: compare\n";
        } else {
            out << "presentation: " << pres << "\n";
        }

        // Shift
        QHash<QString,QVariant> shifts = _bookModel->getDataHash(QModelIndex(),
                                                             "RunToShiftHash");
        QString shift;
        foreach ( QString run, shifts.keys() ) {
            QString shiftVal = shifts.value(run).toString();
            shift += run + ":" + shiftVal + ",";
        }
        if ( !shift.isEmpty() ) {
            shift.chop(1); // chop off comma
            out << "shift: " << shift << "\n";
        }

        // Show Tables
        out << "showTables: ";
        if ( _isShowTables ) {
            out << "on\n";
        } else {
            out << "off\n";
        }

        // Start/Stop Times
        double start = _bookModel->getDataDouble(QModelIndex(),"StartTime");
        double stop  = _bookModel->getDataDouble(QModelIndex(),"StopTime");
        if ( start != -DBL_MAX ) {
            out << "start: " << start << "\n";
        }
        if ( stop != DBL_MAX ) {
            out << "stop: "  << stop << "\n";
        }

        // Timename(s)
        QString timeNames;
        foreach ( QString timeName, _timeNames ) {
            timeNames += timeName + "=";
        }
        if ( !timeNames.isEmpty() ) {
            timeNames.chop(1);
            out << "timeName: " << timeNames << "\n";
        }

        // Map or mapfile
        if ( !_map.isEmpty() && !_mapFile.isEmpty() ) {
            fprintf(stderr, "koviz [bad scoobs]: PlotMainWindow::_saveSession()"
                            " commandline options map=%s and mapFile=%s cannot "
                            "be used together.\n",
                            _map.toLatin1().constData(),
                            _mapFile.toLatin1().constData());
            exit(-1);
        }
        if ( !_map.isEmpty() ) {
            out << "map: " << _map << "\n";
        }
        if ( !_mapFile.isEmpty() ) {
            out << "mapFile: " << _mapFile << "\n";
        }

        if ( !_videos.isEmpty() ) {
            QString videoOption;
            QPair<QString,double> video;
            foreach ( video, _videos ) {
                videoOption += video.first + ":" +
                                            QString::number(video.second) + ",";
            }
            videoOption.chop(1); // take off trailing ','

            out << "videoList: " << videoOption;
        }

        QString isShowPageTitle = _bookModel->getDataString(
                                    QModelIndex(),"IsShowPageTitle");
        out << "showPageTitle: " << isShowPageTitle << "\n";

        QString isShowPlotLegend = _bookModel->getDataString(
                                    QModelIndex(),"IsShowPlotLegend");
        out << "showPlotLegend: " << isShowPlotLegend << "\n";

        QString plotLegendPosition = _bookModel->getDataString(
                                         QModelIndex(),"PlotLegendPosition");
        out << "PlotLegendPosition: " << plotLegendPosition << "\n";

        f.close();
    }
}

void PlotMainWindow::_openVideoByRun()
{
    int i = _monteInputsView->currentRun();
    if ( i >= 0 && _videos.isEmpty() ) { // No cmdline or menu opened videos
        // Look in RUN dir for videos
        QString runpath = _runs->runPaths().at(i);
        QString videoDir = _bookModel->getDataString(QModelIndex(),
                                                     "VideoDir","");
        QString videoDirName = runpath + "/" + videoDir;
        QFileInfo fi(videoDirName);
        QList<QPair<QString, double> > videos;
        if ( fi.exists() && fi.isDir() ) {
            QStringList filter;
            filter << "*.MP4" << "*.mp4" << "*.avi";
            QDir videoDir(videoDirName);
            QStringList files = videoDir.entryList(filter, QDir::Files);
            if ( files.size() > 0 ) {
                QString f1 = videoDir.absoluteFilePath("video-offsets.txt");
                QString f2 = videoDir.absoluteFilePath("video-offset.txt");
                if ( QFileInfo(f1).exists() ) {
                    if ( !QFileInfo(f1).isReadable() ) {
                        fprintf(stderr, "koviz [error]: Cannot read "
                                        "video file=\"%s\"\n",
                                f1.toLatin1().constData());
                        exit(-1);
                    }
                    QFile file(f1);
                    if (!file.open(QIODevice::ReadOnly)) {
                        fprintf(stderr,"koviz: [error] could not open %s\n",
                                file.fileName().toLatin1().constData());
                        exit(-1);
                    }
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString line = in.readLine();
                        if ( ! line.contains(',') ) {
                            fprintf(stderr, "koviz [error]: Video offsets "
                                          "file=%s has line=%s without a comma "
                                          "delimited file,offset\n",
                                    f1.toLatin1().constData(),
                                    line.toLatin1().constData());
                            exit(-1);
                        }
                        QString f = line.split(',', _skipEmptyParts)[0];

                        if ( !videoDir.exists(f) ) {
                            fprintf(stderr, "koviz [error]: Video file=%s "
                                            "referenced in \"%s\" DNE.\n",
                                    f.toLatin1().constData(),
                                    f1.toLatin1().constData());
                            exit(-1);
                        }
                        f = videoDir.filePath(f);
                        QString o = line.split(',', _skipEmptyParts)[1];
                        bool ok;
                        double offset = o.toDouble(&ok);
                        if ( ok ) {
                            videos.append(qMakePair(f,offset));
                        } else {
                            fprintf(stderr, "koviz [error]: Bad video "
                                            "offset in file=%s\n",
                                    f2.toLatin1().constData());
                            exit(-1);
                        }
                    }
                } else if (QFileInfo(f2).exists() ) {

                    QPair<QString,double> video;

                    // If there are multiple videos, really should use
                    // video-offets.txt rather than video-offset.txt
                    if ( files.size() > 1 ) {
                        fprintf(stderr, "koviz [warning]: Multiple videos found"
                               " in videoDir=%s, using video=%s\n"
                               "Use -video-offsets instead of -video-offset.\n",
                               videoDir.dirName().toLatin1().constData(),
                               files.at(0).toLatin1().constData());
                    }

                    video.first = videoDir.absoluteFilePath(files.at(0));

                    video.second = 0.0;
                    QFile file(f2);
                    if (file.open(QFile::ReadOnly)) {
                        QByteArray buf;
                        buf = file.readLine();
                        if ( !buf.isEmpty() ) {
                            // the line is available in buf
                            QString str(buf);
                            bool ok;
                            double offset = str.toDouble(&ok);
                            if ( ok ) {
                                video.second = offset;
                            } else {
                                fprintf(stderr, "koviz [error]: Bad video "
                                                "offset in file=%s\n",
                                        f2.toLatin1().constData());
                            }
                        }
                    } else {
                        fprintf(stderr, "koviz [error]: Cannot read "
                                        "video file=\"%s\"\n",
                                f2.toLatin1().constData());
                        exit(-1);
                    }
                    videos.append(video);
                }
            }
        }
        _openVideos(videos);
    }
}

void PlotMainWindow::_openVideoByMenu()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    if ( ! filename.isEmpty() ) {
        _videos.clear();
        _videos.append(qMakePair(filename,0.0));
        _openVideos(_videos);
    }
}

void PlotMainWindow::_openVideos(const QList<QPair<QString, double> > &videos)
{
#ifndef HAS_MPV
        fprintf(stderr, "koviz [error]: koviz built without mpv video support!"
                        "  Please install mpv and rebuild koviz!\n");
        exit(-1);
#endif
    QRect lastVideoRect;
    if ( vidView ) {
        lastVideoRect = vidView->geometry();
    }

    if ( !vidView && videos.isEmpty() ) {
        // No video view and no videos, nothing to do
        return;
    }

    bool isLiveTime = false;
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    double liveTime;
    if ( !_bookModel->data(liveIdx).toString().isEmpty() ) {
        isLiveTime = true;
        liveTime = _bookModel->data(liveIdx).toDouble();
    }

    if ( !vidView ) {
        vidView = new VideoWindow(videos,this);
        vidView->setAttribute(Qt::WA_DeleteOnClose);
        vidView->show();
        this->setFocusPolicy(Qt::StrongFocus);
        connect(vidView,SIGNAL(timechangedByMpv(double)),
                this, SLOT(setTimeFromVideo(double)));
        connect(vidView,SIGNAL(closeVidView()), this, SLOT(vidViewClosed()));
    } else {
        if ( vidView->isHidden() ) {
            vidView->show();
        }
    }

    if ( isLiveTime ) {
        vidView->set_start(liveTime);
        vidView->pause();
    }
    vidView->set_videos(videos);
    vidView->raise();

    if ( isLiveTime ) {
        // Koviz needs to drive time if -liveTime used
        this->activateWindow();
        QCoreApplication::processEvents();
    }

    if ( vidView ) {
        if ( lastVideoRect.isNull() ) {
            _readVideoWindowSettings();
        } else {
            vidView->setGeometry(lastVideoRect);
        }
    }
}

void PlotMainWindow::_toggleShowVideo()
{
    QModelIndex showVideoIdx = _bookModel->getDataIndex(QModelIndex(),
                                                        "ShowVideo","");
    bool showVideo = _bookModel->getDataBool(QModelIndex(),"ShowVideo","");
    QSettings settings("JSC", "koviz");
    if ( showVideo ) {
        _bookModel->setData(showVideoIdx,false);
        _showVideoAction->setChecked(false);
        settings.setValue("VideoWindow/showVideo", false);
        if ( vidView ) {
            vidView->close();
            vidView = 0;
        }
    } else {
#ifndef HAS_MPV
            QMessageBox msgBox;
            msgBox.setText("Koviz built without mpv video support!  "
                           "Please install mpv and rebuild koviz!");
            msgBox.exec();
            _showVideoAction->setChecked(false);  // reject user check
            return;
#endif
        if ( !_videos.isEmpty() ) {
            QPair<QString,double> video; // mp4name,timeoffset
            foreach ( video, _videos ) {
                QFileInfo fi(video.first);
                if ( !fi.isReadable() ) {
                    QMessageBox msgBox;
                    QString msg = QString("Cannot find or read video=%1\n").
                                          arg(video.first);
                    msgBox.setText(msg);
                    return;
                }
            }
            _openVideos(_videos);
            selectFirstCurve();
        } else {
            _openVideoByRun();
        }
        _bookModel->setData(showVideoIdx,true);
        settings.setValue("VideoWindow/showVideo", true);
        _showVideoAction->setChecked(true);
    }
}

void PlotMainWindow::_chooseVideoDirectory()
{
    QString currVideoDir = _bookModel->getDataString(QModelIndex(),
                                                     "VideoDir","");
    QString videoDir = QInputDialog::getText(this, "VideoDir",
                                             "Video directory path:",
                                             QLineEdit::Normal,currVideoDir);

    if (!videoDir.isEmpty()) {
        QSettings settings("JSC", "koviz");
        QModelIndex videoDirIdx = _bookModel->getDataIndex(QModelIndex(),
                                                           "VideoDir","");
        _bookModel->setData(videoDirIdx,videoDir);
        settings.setValue("VideoWindow/videoDir", videoDir);
    }
}

void PlotMainWindow::_toggleShowLiveCoord()
{
    QModelIndex isShowIdx = _bookModel->getDataIndex(QModelIndex(),
                                                     "IsShowLiveCoord");
    bool isShowLiveCoord = _bookModel->data(isShowIdx).toBool();
    if ( isShowLiveCoord ) {
        _bookModel->setData(isShowIdx,false);  // hide
        _showLiveCoordAction->setChecked(false);
    } else {
        _bookModel->setData(isShowIdx,true);   // show
        _showLiveCoordAction->setChecked(true);
    }
}

void PlotMainWindow::_markTime()
{
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    double liveTime = _bookModel->data(liveIdx).toDouble();
    MarkerDialog dialog(liveTime,this);
    if (dialog.exec() == QDialog::Accepted) {
        QString label = dialog.label();
        double time = dialog.time();
        QModelIndex liveTimeIdxIdx = _bookModel->getDataIndex(QModelIndex(),
                                                          "LiveCoordTimeIndex");
        int timeIdx = _bookModel->data(liveTimeIdxIdx).toInt();

        _bookView->markTime(label,time,timeIdx);
    }

}

void PlotMainWindow::_refreshPlots()
{
    _bookModel->refreshRuns();

    foreach (QModelIndex pageIdx, _bookModel->pageIdxs()) {
        // For now just reset curve x/y bias to undo any curve shifting
        foreach (QModelIndex plotIdx, _bookModel->plotIdxs(pageIdx)) {
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves","Plot");
            foreach (QModelIndex curveIdx, _bookModel->curveIdxs(curvesIdx)) {
                QModelIndex xBiasIdx = _bookModel->getDataIndex(curveIdx,
                                                          "CurveXBias","Curve");
                _bookModel->setData(xBiasIdx,0);
                QModelIndex yBiasIdx = _bookModel->getDataIndex(curveIdx,
                                                          "CurveYBias","Curve");
                _bookModel->setData(yBiasIdx,0);
            }
            QRectF bbox = _bookModel->calcCurvesBBox(curvesIdx);
            _bookModel->setPlotMathRect(bbox,plotIdx);
        }
    }
}

void PlotMainWindow::_clearPlots()
{
    QModelIndex pagesIdx = _bookModel->getIndex(QModelIndex(),"Pages");
    int nPages = _bookModel->rowCount(pagesIdx);
    for (int i = nPages-1; i >= 0; --i) {
        _bookModel->removeRow(i,pagesIdx);
    }
}

void PlotMainWindow::_clearTables()
{
    QModelIndex tablesIdx = _bookModel->getIndex(QModelIndex(),"Tables");
    int nTables = _bookModel->rowCount(tablesIdx);
    for (int i = nTables-1; i >= 0; --i) {
        _bookModel->removeRow(i,tablesIdx);
    }
}

void PlotMainWindow::_clearRuns()
{
    _runs->clear();
}

void PlotMainWindow::_launchScript(QAction* action)
{
    int i = _monteInputsView->currentRun();
    QString runpath;
    if ( i >= 0 ) {
        runpath = _runs->runPaths().at(i);
    } else {
        if ( _runs->runPaths().size() == 1 ) {
            runpath = _runs->runPaths().at(0);
        } else {
            QMessageBox msgBox;
            msgBox.setText("Please select a run before launching your script.");
            msgBox.exec();
        }
    }
    if ( !runpath.isEmpty() ) {
        // Regular expression to capture words and make multiple words
        // surrounded in single quotes a single word
        // Example call using single quoted multiple words:
        // koviz RUN_test -script "&koviz -t1 'Hello World'"
        QRegularExpression regex(R"((?:'([^']*)')|(\S+))");
        QStringList fields;
        QRegularExpressionMatchIterator it = regex.globalMatch(action->text());
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            if (match.captured(1).isEmpty()) {
                fields.append(match.captured(2)); // Unquoted words
            } else {
                fields.append(match.captured(1)); // Single quoted words
            }
        }

        QString program = fields.takeAt(0);
        program = program.remove('&');
        QStringList arguments;
        arguments << runpath;
        foreach ( QString field, fields ) {
            arguments << field;
        }
        QProcess *proc = new QProcess(this);
        connect(proc,SIGNAL(errorOccurred(QProcess::ProcessError)),
                this,SLOT(_scriptError(QProcess::ProcessError)));
        proc->start(program, arguments);
    }
}

void PlotMainWindow::_plotAllVars()
{
    _varsWidget->selectAllVars();
    _varsWidget->clearSelection();
}

void PlotMainWindow::_runsRefreshed()
{
    // Setup progress dialog
    int nPlots = 0;
    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
        nPlots += plotIdxs.size();
    }
    QProgressDialog* progress =  new QProgressDialog("Updating plots...",
                                                     QString(), 0, nPlots, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);

    int ii = 0;
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
        foreach ( QModelIndex plotIdx, plotIdxs ) {
            if ( progress ) {
                progress->setValue(++ii);
            }
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves", "Plot");
            QModelIndexList curveIdxs = _bookModel->curveIdxs(curvesIdx);
            QStringList curveRunPaths;
            QHash<QPair<QString,QString>,QModelIndex> varHash;
            if ( curveIdxs.isEmpty() ) {
                // Use saved curve xy if no curves
                // Saved curve is last curve deleted
                if ( _bookModel->isChildIndex(plotIdx,
                                              "Plot","PlotSavedCurve") ) {
                    QModelIndex pscIdx = _bookModel->getIndex(plotIdx,
                                                              "PlotSavedCurve",
                                                              "Plot");
                    if ( _bookModel->isChildIndex(pscIdx,
                                                  "PlotSavedCurve","Curve") ) {
                        QModelIndex curveIdx = _bookModel->getIndex(pscIdx,
                                                              "Curve",
                                                              "PlotSavedCurve");
                        QString xName = _bookModel->getDataString(curveIdx,
                                                          "CurveXName","Curve");
                        QString yName = _bookModel->getDataString(curveIdx,
                                                          "CurveYName","Curve");
                        QPair<QString,QString> xy = qMakePair(xName,yName);
                        varHash.insert(xy,curveIdx);
                    }
                }
            } else {
                foreach ( QModelIndex curveIdx, curveIdxs ) {
                    QString curveRunPath = _bookModel->getDataString(curveIdx,
                                                        "CurveRunPath","Curve");
                    curveRunPaths.append(curveRunPath);
                    QString xName = _bookModel->getDataString(curveIdx,
                                                          "CurveXName","Curve");
                    QString yName = _bookModel->getDataString(curveIdx,
                                                          "CurveYName","Curve");
                    QPair<QString,QString> xy = qMakePair(xName,yName);
                    varHash.insert(xy,curveIdx);
                }
            }
            QStringList runPathsToAppend;
            foreach ( QString runPath, _runs->runPaths() ) {
                if ( !curveRunPaths.contains(runPath) ) {
                    runPathsToAppend.append(runPath);
                }
            }
            QStringList runPathsToRemove;
            foreach ( QString curveRunPath, curveRunPaths ) {
                if ( !_runs->runPaths().contains(curveRunPath) ) {
                    runPathsToRemove.append(curveRunPath);
                }
            }

            bool isAppend = false;
            bool isRemove = false;
            if ( varHash.keys().size() == 1 ) { // Same xy forall curr runs
                QPair<QString,QString> xy = varHash.keys().at(0);
                QString xName = xy.first;
                QString yName = xy.second;
                if ( _runs->params().contains(xName) &&
                     _runs->params().contains(yName) ) {
                    // Run to append/remove contains current plot curve xy
                    if ( runPathsToAppend.size() > 0 ) {
                        isAppend = true;
                    }
                    if ( runPathsToRemove.size() > 0 ) {
                        isRemove = true;
                    }
                } else if ( _runs->runPaths().isEmpty() ) {
                    if ( runPathsToRemove.size() > 0 ) {
                        isRemove = true;
                    }
                }
            }

            if ( isAppend ) {
                QString tName = _timeNames.first();
                QString xName = varHash.keys().at(0).first;
                QString yName = varHash.keys().at(0).second;
                foreach ( QString runPathToAppend, runPathsToAppend ) {
                    // Try to append new run using values from last curve
                    int rc = _runs->runsModel()->rowCount();
                    QList<QColor> colors = _bookModel->createCurveColors(rc);
                    for ( int r = 0; r < rc; ++r ) {
                        QString path = _runs->runsModel()->
                                          headerData(r,Qt::Vertical).toString();
                        QFileInfo fi(path);
                        if ( runPathToAppend == fi.absoluteFilePath() ) {
                            // Make presentation "compare"
                            QModelIndex presIdx = _bookModel->getDataIndex(
                                                             plotIdx,
                                                             "PlotPresentation",
                                                             "Plot");
                            _bookModel->setData(presIdx, "compare");

                            // Last curve idx in plot
                            QModelIndex cidx = varHash.values().last();

                            CurveModel* curveModel = _runs->curveModel(r,
                                                             tName,
                                                             xName,
                                                             yName);

                            QModelIndex idx = _runs->runsModel()->index(r,0);
                            int runId = _runs->runsModel()->data(idx).toInt();

                            // Get values from last curve
                            idx = _bookModel->getDataIndex(cidx,
                                                          "CurveXUnit","Curve");
                            QString xUnit = _bookModel->data(idx).toString();
                            idx = _bookModel->getDataIndex(cidx,
                                                         "CurveXScale","Curve");
                            double xScale = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                          "CurveXBias","Curve");
                            double xBias = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                      "CurveXMinRange","Curve");
                            double xMinRange = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                      "CurveXMaxRange","Curve");
                            double xMaxRange = _bookModel->data(idx).toDouble();


                            idx = _bookModel->getDataIndex(cidx,
                                                          "CurveYUnit","Curve");
                            QString yUnit = _bookModel->data(idx).toString();
                            idx = _bookModel->getDataIndex(cidx,
                                                         "CurveYScale","Curve");
                            double yScale = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                         "CurveYBias","Curve");
                            double yBias = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                      "CurveYMinRange","Curve");
                            double yMinRange = _bookModel->data(idx).toDouble();
                            idx = _bookModel->getDataIndex(cidx,
                                                      "CurveYMaxRange","Curve");
                            double yMaxRange = _bookModel->data(idx).toDouble();

                            idx = _bookModel->getDataIndex(cidx,
                                                     "CurveLineStyle","Curve");
                            QString lineStyle =_bookModel->data(idx).toString();
                            idx = _bookModel->getDataIndex(cidx,
                                                    "CurveSymbolStyle","Curve");
                            QString symbolStyle =
                                               _bookModel->data(idx).toString();
                            idx = _bookModel->getDataIndex(cidx,
                                                     "CurveSymbolSize","Curve");
                            QString symbolSize=_bookModel->data(idx).toString();
                            idx = _bookModel->getDataIndex(cidx,
                                                     "CurveSymbolEnd","Curve");
                            QString symbolEnd =_bookModel->data(idx).toString();

                            // Unit check
                            bool canConvertXUnit = Unit::canConvert(xUnit,
                                                       curveModel->x()->unit());
                            bool canConvertYUnit = Unit::canConvert(yUnit,
                                                       curveModel->y()->unit());
                            if( !canConvertXUnit || !canConvertYUnit ) {
                                continue; // Don't add curve if units mismatch
                            }

                            // Add Curve
                            QStandardItem* curvesItem = _bookModel->
                                                       itemFromIndex(curvesIdx);
                            QStandardItem *curveItem = _bookModel->
                                                   addChild(curvesItem,"Curve");
                            _bookModel->addChild(curveItem, "CurveRunID",runId);
                            _bookModel->addChild(curveItem, "CurveRunPath",
                                                            runPathToAppend);
                            _bookModel->addChild(curveItem, "CurveTimeName",
                                                            tName);
                            _bookModel->addChild(curveItem, "CurveTimeUnit",
                                                       curveModel->t()->unit());

                            _bookModel->addChild(curveItem, "CurveXName",xName);
                            _bookModel->addChild(curveItem, "CurveXUnit",xUnit);
                            _bookModel->addChild(curveItem, "CurveXScale",
                                                                        xScale);
                            _bookModel->addChild(curveItem, "CurveXBias",xBias);

                            _bookModel->addChild(curveItem, "CurveYName",yName);
                            _bookModel->addChild(curveItem, "CurveYUnit",yUnit);
                            _bookModel->addChild(curveItem, "CurveYScale",
                                                                        yScale);
                            _bookModel->addChild(curveItem, "CurveYBias",yBias);

                            _bookModel->addChild(curveItem, "CurveXMinRange",
                                                            xMinRange);
                            _bookModel->addChild(curveItem, "CurveXMaxRange",
                                                            xMaxRange);
                            _bookModel->addChild(curveItem, "CurveYMinRange",
                                                            yMinRange);
                            _bookModel->addChild(curveItem, "CurveYMaxRange",
                                                            yMaxRange);
                            _bookModel->addChild(curveItem, "CurveYLabel",
                                                            yName);
                            _bookModel->addChild(curveItem, "CurveColor",
                                                            colors.at(r));
                            _bookModel->addChild(curveItem, "CurveLineStyle",
                                                            lineStyle);
                            _bookModel->addChild(curveItem, "CurveSymbolStyle",
                                                            symbolStyle);
                            _bookModel->addChild(curveItem, "CurveSymbolSize",
                                                            symbolSize);
                            _bookModel->addChild(curveItem, "CurveSymbolEnd",
                                                            symbolEnd);
                            QVariant v = PtrToQVariant<CurveModel>::convert(
                                                                    curveModel);
                            _bookModel->addChild(curveItem, "CurveData", v);

                            QRectF bbox = _bookModel->calcCurvesBBox(curvesIdx);
                            _bookModel->setPlotMathRect(bbox,plotIdx);
                        }
                    }
                }
            }

            if ( isRemove ) {
                // First, set plot presentation to compare
                QModelIndex presIdx = _bookModel->getDataIndex(
                                                             plotIdx,
                                                             "PlotPresentation",
                                                             "Plot");
                _bookModel->setData(presIdx, "compare");

                QModelIndexList curveIdxsToRemove;
                foreach ( QModelIndex curveIdx, curveIdxs ) {
                    QString curveRunPath = _bookModel->getDataString(curveIdx,
                                                        "CurveRunPath","Curve");
                    if ( runPathsToRemove.contains(curveRunPath) ) {
                        curveIdxsToRemove.append(curveIdx);
                    }
                }
                if ( !_bookModel->isChildIndex(plotIdx,
                                              "Plot","PlotSavedCurve") ) {
                    // Create and append PlotSavedCurve item to Plot
                    QStandardItem* plotItem= _bookModel->itemFromIndex(plotIdx);
                    QStandardItem* plotSavedCurveItem = new QStandardItem(
                                                              "PlotSavedCurve");
                    plotItem->appendRow(plotSavedCurveItem);
                }
                QModelIndex saveIdx = _bookModel->getIndex(plotIdx,
                                                            "PlotSavedCurve",
                                                            "Plot");
                int rc = _bookModel->rowCount(saveIdx);
                _bookModel->removeRows(0,rc,saveIdx);
                _copyCurve(curveIdxsToRemove.last(),saveIdx);
                for ( int i = curveIdxsToRemove.size()-1; i >= 0; --i ) {
                    QModelIndex curveIdx = curveIdxsToRemove.at(i);
                    _bookModel->removeRow(curveIdx.row(),curvesIdx);
                }

                QModelIndexList curveIdxs = _bookModel->curveIdxs(curvesIdx);
                int nc = curveIdxs.size();
                if ( nc > 0 ) {
                    QList<QColor> colors = _bookModel->createCurveColors(nc);
                    int i = 0;
                    // Reset colors on all curves
                    foreach ( QModelIndex curveIdx, curveIdxs ) {
                        QModelIndex curveColorIdx = _bookModel->getDataIndex(
                                    curveIdx,"CurveColor","Curve");
                        QColor color = colors.at(i);
                        _bookModel->setData(curveColorIdx,color);
                        ++i;
                    }

                    QRectF currPlotRect = _bookModel->getDataRectF(plotIdx,
                                                         "PlotMathRect","Plot");
                    QRectF newBBox = _bookModel->calcCurvesBBox(curvesIdx);

                    if ( newBBox != currPlotRect ) {
                        if ( currPlotRect.contains(newBBox) ) {
                            // If current plot zoom contains newBBox with
                            // curve removed, zoom in to newBBox
                            _bookModel->setPlotMathRect(newBBox,plotIdx);
                        } else {
                            // Keep current bbox so curr zoom is not lost
                            _bookModel->setPlotMathRect(currPlotRect,plotIdx);
                        }
                    }
                } else {
                    // All curves have been removed
                    QRectF currPlotRect = _bookModel->getDataRectF(plotIdx,
                                                         "PlotMathRect","Plot");
                    if ( !currPlotRect.normalized().isEmpty() ) {
                        // Reset to force redraw
                        QRectF E; // Empty
                        _bookModel->setPlotMathRect(E,plotIdx);
                        _bookModel->setPlotMathRect(currPlotRect,plotIdx);
                    }
                }
            }
        }
    }

    // End Progress Dialog
    if ( progress ) {
        progress->setValue(nPlots);
        delete progress;
    }
}

void PlotMainWindow::_copyCurve(const QModelIndex &srcCurveIdx,
                                const QModelIndex &tgtParentIdx)
{
    QStandardItem* tgtParentItem = _bookModel->itemFromIndex(tgtParentIdx);
    QStandardItem* curveItem = _bookModel->addChild(tgtParentItem, "Curve", "");
    for (int r = 0; r < _bookModel->rowCount(srcCurveIdx); ++r ) {
        QModelIndex srcIdx0 = _bookModel->index(r,0,srcCurveIdx);
        QString tag = _bookModel->data(srcIdx0).toString();
        QModelIndex srcIdx1 = _bookModel->index(r,1,srcCurveIdx);
        QVariant value = _bookModel->data(srcIdx1);
        _bookModel->addChild(curveItem, tag, value);
    }
}

void PlotMainWindow::_toggleEnableDragDrop(bool isChecked )
{
    _varsWidget->setDragEnabled(isChecked);
    if ( _trickView ) {
        _trickView->setDragEnabled(isChecked);
    }
}

void PlotMainWindow::_filterOutFlatLines()
{
    QModelIndex pagesIdx = _bookModel->getIndex(QModelIndex(),"Pages","");
    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    int nPages = pageIdxs.size();
    for (int i = nPages-1; i >= 0; --i) {
        QModelIndex pageIdx = pageIdxs.at(i);
        QModelIndex plotsIdx = _bookModel->getIndex(pageIdx,"Plots","Page");
        QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
        int nplots = plotIdxs.size();
        for (int j = nplots-1; j >= 0; --j) {
            QModelIndex plotIdx = plotIdxs.at(j);
            QString plotPres = _bookModel->getDataString(plotIdx,
                                                    "PlotPresentation", "Plot");
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                               "Curves","Plot");
            if ( plotPres == "error" ) {
                QPainterPath* errorPath = _bookModel->getCurvesErrorPath(
                                                                     curvesIdx);
                QRectF ebox = errorPath->boundingRect();
                if ( ebox.height() == 0.0 && ebox.y() == 0.0 ) {
                    _bookModel->removeRow(j,plotsIdx);
                    if ( _bookModel->rowCount(plotsIdx) == 0 ) {
                        // If all plots removed, remove page
                        _bookModel->removeRow(i,pagesIdx);
                    }
                }
                delete errorPath;
            } else if ( plotPres == "compare" ) {
                bool isFlat = true;
                QString plotYScale = _bookModel->getDataString(plotIdx,
                                                           "PlotYScale","Plot");

                QModelIndexList curveIdxs = _bookModel->getIndexList(curvesIdx,
                                                              "Curve","Curves");
                foreach ( QModelIndex curveIdx, curveIdxs ) {
                    QPainterPath* path = _bookModel->getPainterPath(curveIdx);
                    QRectF cbox = path->boundingRect();
                    if ( cbox.height() == 0.0 ) {

                        // If logscale, scale/bias done in _createPainterPath
                        double ys = 1.0;
                        double yb = 0.0;
                        if ( plotYScale == "linear" ) {
                            ys = _bookModel->getDataDouble(curveIdx,
                                                         "CurveYScale","Curve");
                            yb = _bookModel->getDataDouble(curveIdx,
                                                          "CurveYBias","Curve");
                        }

                        // Empty paths are considered "flat"
                        double y = cbox.y()*ys+yb;
                        if ( y != 0 ) {
                            isFlat = false;
                            break;
                        }
                    } else {
                        isFlat = false;
                        break;
                    }
                }

                if ( isFlat ) {
                    _bookModel->removeRow(j,plotsIdx);
                    if ( _bookModel->rowCount(plotsIdx) == 0 ) {
                        // If all plots removed, remove page
                        _bookModel->removeRow(i,pagesIdx);
                    }
                }
            } else {
                // Do not filter when error+compare (TODO if ever needed)
            }
        }
    }

    nPages = _bookModel->rowCount(pagesIdx);
    if ( nPages == 0 && QGuiApplication::platformName() != "offscreen" ) {
        QMessageBox msgBox;
        QString msg = QString("All pages filtered out "
                              "since all plots flatline zero or empty!");
        msgBox.setText(msg);
        msgBox.exec();
    }
}

void PlotMainWindow::_selectRunsHome()
{
    QString currRunsHome = _runsWidget->runsHome();
    QString runsHome = QFileDialog::getExistingDirectory(this,
                                                   "Select Runs Home Directory",
                                                   currRunsHome,
                                                   QFileDialog::ShowDirsOnly);
    if ( !runsHome.isEmpty() ) {
        QFileInfo fi(runsHome);
        if ( fi.isDir() ) {
            _runsWidget->setRunsHome(runsHome);

            // Save runs home in .config
            QSettings settings("JSC", "koviz");
            settings.beginGroup("PlotMainWindow");
            settings.setValue("runsHome", runsHome);
            settings.endGroup();
        }
    }
}

void PlotMainWindow::_startTimeChanged(double startTime)
{
    QModelIndex startTimeIdx = _bookModel->getDataIndex(QModelIndex(),
                                                        "StartTime");
    _bookModel->setData(startTimeIdx,startTime);

    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        foreach ( QModelIndex plotIdx, _bookModel->plotIdxs(pageIdx) ) {
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves","Plot");
            QRectF bbox = _bookModel->calcCurvesBBox(curvesIdx);
            _bookModel->setPlotMathRect(bbox,plotIdx);
        }
    }

    // If live time is less than start time, change live time to start time
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    double liveTime = _bookModel->data(liveIdx).toDouble();
    if ( liveTime < startTime ) {
        _bookModel->setData(liveIdx,startTime);
    }
}

void PlotMainWindow::_liveTimeChanged(double liveTime)
{
    QModelIndex idx = _currentIdx();
    if ( !idx.isValid() ) {
        return;
    }

    CurveModel* curveModel = _bookModel->getCurveModel(idx);

    if ( curveModel ) {

        curveModel->map();

        QString tag = _bookModel->data(idx.sibling(idx.row(),0)).toString();

        bool isXTime = (curveModel->x()->name() == curveModel->t()->name());

        // Get curve index i of liveTime
        // Curve x bias and x scale
        double xs = 1.0;
        double xb = 0.0;
        if ( isXTime && tag == "Curve" ) {
            xs = _bookModel->getDataDouble(idx,"CurveXScale","Curve");
            xb = _bookModel->getDataDouble(idx,"CurveXBias","Curve");
        }

        int i = 0;
        if ( isXTime && tag == "Curve" ) {
            // Convert liveTime to logged/model time (undo units and scale/bias)
            QString xunit = _bookModel->getDataString(idx,"CurveXUnit","Curve");
            QString tunit = curveModel->t()->unit();
            double tus = 1.0;
            double tub = 0.0;
            if ( !xunit.isEmpty() ) {
                tus = Unit::scale(xunit,tunit);
                tub = Unit::bias(xunit,tunit);
            }
            double logTime = (liveTime-xb)/xs;
            logTime = tus*logTime + tub;
            i = curveModel->indexAtTime(logTime);
        } else {
            // e.g. ball xy curve where x is position[0]
            i = curveModel->indexAtTime(liveTime);
        }

        ModelIterator* it = curveModel->begin();
        double t = it->at(i)->t();
        if ( isXTime && tag == "Curve" ) {
            QString xunit = _bookModel->getDataString(idx,"CurveXUnit","Curve");
            QString tunit = curveModel->t()->unit();
            double tus = 1.0;
            double tub = 0.0;
            if ( !xunit.isEmpty() ) {
                tus = Unit::scale(tunit,xunit);
                tub = Unit::bias(tunit,xunit);
            }
            t = tus*t + tub;
            t = t*xs+xb;
        }
        delete it;
        curveModel->unmap();

        QModelIndex rootIdx;
        double start = _bookModel->getDataDouble(rootIdx,"StartTime");
        double stop  = _bookModel->getDataDouble(rootIdx,"StopTime");
        t = (t < start) ? start : t;
        t = (t > stop)  ? stop  : t;
        QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                       "LiveCoordTime");
        _bookModel->setData(liveIdx,t);
        _timeInput->setLiveTime(t);

        // TODO: See getCurveModel() comment
        if ( tag == "Plot" ) {
            delete curveModel;
        }
    }
}

void PlotMainWindow::_stopTimeChanged(double stopTime)
{
    QModelIndex stopTimeIdx = _bookModel->getDataIndex(QModelIndex(),
                                                       "StopTime");
    _bookModel->setData(stopTimeIdx,stopTime);

    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        foreach ( QModelIndex plotIdx, _bookModel->plotIdxs(pageIdx) ) {
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves","Plot");
            QRectF bbox = _bookModel->calcCurvesBBox(curvesIdx);
            _bookModel->setPlotMathRect(bbox,plotIdx);
        }
    }

    // If live time is greater than stop time, change live time to stop time
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    double liveTime = _bookModel->data(liveIdx).toDouble();
    if ( liveTime > stopTime ) {
        _bookModel->setData(liveIdx,stopTime);
    }
}

void PlotMainWindow::_liveTimeNext()
{
    QModelIndex idx = _currentIdx();
    _bookModel->liveTimeNext(idx);
}

void PlotMainWindow::_liveTimePrev()
{
    QModelIndex idx = _currentIdx();
    _bookModel->liveTimePrev(idx);
}

void PlotMainWindow::_monteInputsHeaderViewClicked(int section)
{
    Q_UNUSED(section);

    if ( !_bookModel ) return;

    int rc = _runs->runsModel()->rowCount();
    QList<QColor> colors = _bookModel->createCurveColors(rc);

    QHash<int,QString> run2color;
    for ( int r = 0; r < rc; ++r ) {
        QModelIndex runIdx = _runs->runsModel()->index(r,0);
        int runId = _runs->runsModel()->data(runIdx).toInt();
        run2color.insert(runId, colors.at(r).name());
    }

    QModelIndexList pageIdxs = _bookModel->pageIdxs();
    foreach ( QModelIndex pageIdx, pageIdxs ) {
        QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
        foreach ( QModelIndex plotIdx, plotIdxs ) {
            // Turn off signals so pixmaps are not created as each color changes
            bool block = _bookModel->blockSignals(true);
            QModelIndex changeColorIdx;
            QString     changeColorFrom;
            QString     changeColorTo;
            QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                         "Curves", "Plot");
            QModelIndexList curveIdxs = _bookModel->curveIdxs(curvesIdx);
            foreach ( QModelIndex curveIdx, curveIdxs ) {
                int runId = _bookModel->getDataInt(curveIdx,
                                                   "CurveRunID",
                                                   "Curve");
                QString nextColor = run2color.value(runId);
                QModelIndex colorIdx = _bookModel->getIndex(curveIdx,
                                                            "CurveColor",
                                                            "Curve");
                colorIdx = _bookModel->sibling(colorIdx.row(),1,colorIdx);
                QString currColor = _bookModel->data(colorIdx).toString();
                if ( nextColor != currColor ) {
                    changeColorIdx  = colorIdx;
                    changeColorFrom = currColor;
                    changeColorTo   = nextColor;
                }
                _bookModel->setData(colorIdx,nextColor);
            }

            // Set one curve color back to its original color
            // before turning the signals back on
            _bookModel->setData(changeColorIdx,changeColorFrom);

            // Turn signals back on
            _bookModel->blockSignals(block);

            // Set one curve color while signals on so that the dataChanged
            // signal will fire and a new pixmap will be created
            _bookModel->setData(changeColorIdx,changeColorTo);
        }
    }
}

void PlotMainWindow::_monteInputsViewCurrentChanged(const QModelIndex &currIdx,
                                                    const QModelIndex &prevIdx)
{
    if ( currIdx.isValid() ) {
        // set all curves in bookview with runID to current
        // Note: column 0 is runID by convention
        QModelIndex runIDIdx = _monteInputsView->model()->sibling(currIdx.row(),
                                                                  0,currIdx);
        int runID = _monteInputsView->model()->data(runIDIdx).toInt();
        _bookView->setCurrentCurveRunID(runID);
        if (currIdx.row() != prevIdx.row())
        {
            QString runpath = QString("%1/").arg(
                                    QDir::current().absoluteFilePath(
                                     _runs->runPaths().at(runID)));
            _the_visualizer->sendRun2Bvis(runpath);
            _blender->sendRun2Bvis(runpath);
        }
#if HAS_MPV
        bool showVideo = _bookModel->getDataBool(QModelIndex(),"ShowVideo","");
        if ( showVideo ) {
            _openVideoByRun();
        }
#endif
    }
}

void PlotMainWindow::_writeSettings()
{
    QSettings settings("JSC", "koviz");

    settings.beginGroup("PlotMainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
    if ( vidView ) {
        settings.beginGroup("VideoWindow");
        settings.setValue("size", vidView->size());
        settings.setValue("pos", vidView->pos());
        settings.endGroup();
    }
}

// On linux the settings file is in ~/.config/JSC/koviz.conf
void PlotMainWindow::_readSettings()
{
    _readMainWindowSettings();
    _readVideoWindowSettings();
}

void PlotMainWindow::_readMainWindowSettings()
{
    QSettings settings("JSC", "koviz");
    settings.beginGroup("PlotMainWindow");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(1300, 720)).toSize();
    settings.endGroup();

    __placeWindow(this,pos,size);
}

void PlotMainWindow::_readVideoWindowSettings()
{
    if ( vidView ) {
        QSettings settings("JSC", "koviz");
        settings.beginGroup("VideoWindow");
        QPoint pos = settings.value("pos", QPoint(50, 50)).toPoint();
        QSize size = settings.value("size", QSize(1300, 720)).toSize();
        settings.endGroup();

        __placeWindow(vidView, pos, size);
    }
}

void PlotMainWindow::__placeWindow(QWidget *window,
                                   const QPoint &pos, const QSize &size)
{
    // Apply size
    window->resize(size);

    // Move window to saved position if it is viewable at saved position
    bool isPositionValid = false;
    foreach ( QScreen* screen, QGuiApplication::screens() ) {
        QRect screenGeom = screen->availableGeometry();
        if (screenGeom.contains(pos)) {
            isPositionValid = true;
            break;
        }
    }
    if (isPositionValid) {
        window->move(pos);
    }
}


void PlotMainWindow::closeEvent(QCloseEvent *event)
{
    _writeSettings();
    event->accept();
}

QModelIndex PlotMainWindow::_currentIdx()
{
    QModelIndex currentIdx;

    QModelIndex currentBookViewIdx = _bookView->currentIndex();
    if ( currentBookViewIdx.isValid() ) {
        QString tag = _bookModel->data(currentBookViewIdx).toString();
        if ( tag == "Curve" ) {
            currentIdx = currentBookViewIdx;
        } else if ( tag == "Plot" ) {
            // Error plots have a Plot index as the "curve idx"
            currentIdx = currentBookViewIdx;
        } else {
            QModelIndex pageIdx;
            if ( tag ==  "Page" ) {
                pageIdx = currentBookViewIdx;
            } else {
                QModelIndexList pageIdxs = _bookModel->pageIdxs();
                if ( !pageIdxs.isEmpty() ) {
                    pageIdx = pageIdxs.at(0);
                }
            }
            if ( pageIdx.isValid() ) {
                QModelIndex monteIdx;
                if ( _monteInputsView ) {
                    monteIdx = _monteInputsView->currentIndex();
                }
                if ( monteIdx.isValid() ) {
                    // Find first curve in first plot with matching runID
                    QModelIndex runIdx = monteIdx.sibling(monteIdx.row(),0);
                    int runID = _monteInputsView->model()->data(runIdx).toInt();
                    QModelIndex plotsIdx = _bookModel->getIndex(pageIdx,
                                                                "Plots","Page");
                    QModelIndexList plotIdxs = _bookModel->getIndexList(
                                plotsIdx,
                                "Plot",
                                "Plots");
                    foreach ( QModelIndex plotIdx, plotIdxs ) {
                        bool isFound = false;
                        QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,
                                                                     "Curves",
                                                                     "Plot");
                        QModelIndexList curveIdxs = _bookModel->getIndexList(
                                    curvesIdx,
                                    "Curve",
                                    "Curves");
                        foreach ( QModelIndex cIdx, curveIdxs ) {
                            int curveRunID = _bookModel->getDataInt(cIdx,
                                                                   "CurveRunID",
                                                                   "Curve");
                            if ( curveRunID == runID ) {
                                currentIdx = cIdx;
                                isFound = true;
                                break;
                            }
                        }
                        if ( isFound ) {
                            break;
                        }
                    }
                } else {
                    // Find first curve on page regardless of runID match
                    QModelIndexList plotIdxs = _bookModel->plotIdxs(pageIdx);
                    if ( ! plotIdxs.isEmpty() ) {
                        QModelIndex plotIdx0 = plotIdxs.at(0);
                        QModelIndex curvesIdx = _bookModel->getIndex(plotIdx0,
                                                                     "Curves",
                                                                     "Plot");
                        QModelIndexList curveIdxs = _bookModel->curveIdxs(
                                                                     curvesIdx);
                        if ( !curveIdxs.isEmpty() ) {
                            currentIdx = curveIdxs.at(0);
                        }
                    }
                }
            }
        }
    }

    return currentIdx;
}

void PlotMainWindow::_vsRead()
{
    QByteArray bytes = _vsSocket->readLine();
    QString msg(bytes);
    QStringList fields = msg.split('\t');
    double time = fields.at(1).toDouble()/1.0e6;

    static double lasttime = -1.0;
    if ( time != lasttime ) {
        if ( vidView ) {
            vidView->seek_time(time+_trickoffset);
        }
        lasttime = time;
    }
}
