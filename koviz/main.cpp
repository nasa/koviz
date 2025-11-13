#include <QApplication>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QString>
#include <QDate>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QSize>

#include <string>
using namespace std;

#include <QFileInfo>
#include <QTextStream>
#include <stdio.h>
#include <float.h>
#include <cfloat>
#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG 17
#endif

#include "libkoviz/options.h"
#include "libkoviz/runs.h"
#include "libkoviz/plotmainwindow.h"
#include "libkoviz/dp.h"
#include "libkoviz/snap.h"
#include "libkoviz/datamodel_trick.h"
#include "libkoviz/datamodel_csv.h"
#include "libkoviz/trick_types.h"
#include "libkoviz/session.h"
#include "libkoviz/versionnumber.h"
#include "libkoviz/mapvalue.h"

QStandardItemModel* createVarsModel(Runs* runs);
bool writeTrk(const QString& ftrk,
              const QStringList& timeNames,
              const QList<DPVar>& vars,
              const QString& runPath,
              double start, double stop,
              double timeShift, double tmt);
bool writeData(QDataStream* outTrk, QTextStream* outCsv,
                  const QList<DPVar>& vars, const QString& runPath,
                  const QStringList& timeNames,
                  double start, double stop, double timeShift, double tmt);
QList<DataModel*> runDataModels(const QString& runPath,
                                const QStringList& timeNames);
bool writeCsv(const QString& fcsv,
              const QStringList& timeNames,
              const QList<DPVar> &vars,
              const QString& runPath,
              double start, double stop,
              double timeShift, double tmt);
bool printVarValuesAtTime(double time, double tmt, const QStringList& timeNames,
                          const QString& varsOptString,
                          const QString& runPath, Runs* runs);
QList<DPVar> makeVarsList(const QString& varsOptString,Runs* runs);
bool convert2csv(const QStringList& timeNames,
                 const QString& ftrk, const QString& fcsv,
                 double start, double stop, double shift, double tmt);
bool convert2trk(const QString& csvFileName, const QString &trkFileName,
                 const QStringList &timeNames,
                 double start, double stop, double shift, double tmt);
QHash<QString,QVariant> getShiftHash(const QString& shiftString,
                                const QStringList &runPaths);
QHash<QString,QStringList> getVarMap(const QString& mapString);
QHash<QString,QStringList> getVarMapFromFile(const QString& mapFileName);
QStringList getTimeNames(const QString& timeName);
QSize str2size(const QString& str);

Option::FPresetQString presetExistsFile;
Option::FPresetDouble preset_start;
Option::FPresetDouble preset_stop;
Option::FPresetQStringList presetRunsDPs;
Option::FPostsetQStringList postsetRunsDPs;
Option::FPresetQString presetPresentation;
Option::FPresetUInt presetBeginRun;
Option::FPresetUInt presetEndRun;
Option::FPresetQString presetOutputFile;
Option::FPresetQString presetOrientation;
Option::FPresetQString presetIsShowPlotLegend;
Option::FPresetQString presetplotLegendPosition;

class SnapOptions : public Options
{
  public:
    bool isHelp;
    QStringList rundps;
    double start;
    double stop;
    bool isReportRT;
    QString presentation;
    unsigned int beginRun;
    unsigned int endRun;
    QString pdfOutFile;
    QString jpgOutFile;
    QString jpgSize;
    QString dp2trkOutFile;
    QString dp2csvOutFile;
    QString vars2csvFile;
    double vars2valsAtTime;
    QString title1;
    QString title2;
    QString title3;
    QString title4;
    QString timeName;
    QString trk2csvFile;
    QString csv2trkFile;
    QString outputFileName;
    QString shiftString;
    QString map;
    QString mapFile;
    bool isDebug;
    bool isLegend;
    QString legend1;
    QString legend2;
    QString legend3;
    QString legend4;
    QString legend5;
    QString legend6;
    QString legend7;
    QString orient;
    QString sessionFile;
    QString color1;
    QString color2;
    QString color3;
    QString color4;
    QString color5;
    QString color6;
    QString color7;
    QString foreground;
    QString background;
    QString showTables;  // need a string since this can be unset
    QString linestyle1;
    QString linestyle2;
    QString linestyle3;
    QString linestyle4;
    QString linestyle5;
    QString linestyle6;
    QString linestyle7;
    QString symbolstyle1;
    QString symbolstyle2;
    QString symbolstyle3;
    QString symbolstyle4;
    QString symbolstyle5;
    QString symbolstyle6;
    QString symbolstyle7;
    QString symbolend1;
    QString symbolend2;
    QString symbolend3;
    QString symbolend4;
    QString symbolend5;
    QString symbolend6;
    QString symbolend7;
    bool isPlotAllVars;
    QString scripts;
    QString excludePattern;
    QString filterPattern;
    double  timeMatchTolerance;
    QString trickhost;
    uint trickport;
    double trickoffset;
    QString videoFileName;
    double videoOffset;
    QString videoList;
    QString videoDir;
    QString showVideo;
    QString unitOverrides;
    QString group1;
    QString group2;
    QString group3;
    QString group4;
    QString group5;
    QString group6;
    QString group7;
    bool isShowUnits;
    bool isShowPageTitle;
    QString isShowPlotLegend;
    QString plotLegendPosition;
    QString buttonSelectAndPan;
    QString buttonZoom;
    QString buttonReset;
    QString platform;
    QString xaxislabel;
    QString yaxislabel;
    QString vars;
    QString liveTime;
    bool isFilterOutFlatlineZeros;
};

SnapOptions opts;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    Qt::SplitBehaviorFlags skipEmptyParts = Qt::SkipEmptyParts;
#else
    QString::SplitBehavior skipEmptyParts = QString::SkipEmptyParts;
#endif

int main(int argc, char *argv[])
{
    bool ok;
    int ret = -1;

    opts.add("-h:{0,1}",&opts.isHelp,false, "print usage");
    opts.add("[RUNs and DPs:{0,5000}]",
             &opts.rundps, QStringList(),
             "List of RUN dirs/files and DP files",
             presetRunsDPs, postsetRunsDPs);
    opts.add("-rt:{0,1}",&opts.isReportRT,false, "print realtime text report");
    opts.add("-start", &opts.start, -DBL_MAX, "start time", preset_start);
    opts.add("-stop", &opts.stop, DBL_MAX, "stop time", preset_stop);
    opts.add("-pres",&opts.presentation,"",
             "present plot with two curves as compare,error or error+compare",
             presetPresentation);
    opts.add("-beginRun",&opts.beginRun,0,
             "begin run (inclusive) in set of Monte carlo RUNs",
             presetBeginRun);
    opts.add("-endRun",&opts.endRun,100000,
             "end run (inclusive) in set of Monte carlo RUNs",
             presetEndRun);
    opts.add("-t1",&opts.title1,"", "Main title");
    opts.add("-t2",&opts.title2,"", "Subtitle");
    opts.add("-t3",&opts.title3,"", "User title");
    opts.add("-t4",&opts.title4,"", "Date title");
    opts.add("-timeName", &opts.timeName, "",
             "Time variable (e.g. -timeName sys.exec.out.time=mySimTime)");
    opts.add("-pdf", &opts.pdfOutFile, QString(""),
             "Name of pdf output file");
    opts.add("-jpg", &opts.jpgOutFile, QString(""),
             "Name of jpg output file");
    opts.add("-jpgSize", &opts.jpgSize, QString("2560x1440"),
             "Size of jpg e.g. -jpgSize 2560x1440");
    opts.add("-dp2trk", &opts.dp2trkOutFile, QString(""),
             "Create trk from DP_ vars, "
             "e.g. koviz DP_foo RUN_a -dp2trk foo.trk");
    opts.add("-dp2csv", &opts.dp2csvOutFile, QString(""),
             "Create csv from DP_ vars, "
             "e.g. koviz DP_foo RUN_a -dp2csv foo.csv");
    opts.add("-vars2csv", &opts.vars2csvFile, QString(""),
             "Create csv from -vars list");
    opts.add("-vars2valsAtTime", &opts.vars2valsAtTime, qQNaN(),
             "Print val(s) of var(s) at given time.");
    opts.add("-trk2csv", &opts.trk2csvFile, QString(""),
             "Name of trk file to convert to csv (fname subs trk with csv)",
             presetExistsFile);
    opts.add("-csv2trk", &opts.csv2trkFile, QString(""),
             "Name of csv file to convert to trk (fname subs csv with trk)",
             presetExistsFile);
    opts.add("-o", &opts.outputFileName, QString(""),
             "Name of file to output with trk2csv and csv2trk options",
             presetOutputFile);
    opts.add("-shift", &opts.shiftString, QString(""),
             "time shift run curves by value "
             "(e.g. -shift \"RUN_a:1.075,RUN_b:2.0\")");
    opts.add("-map", &opts.map, QString(""),
             "variable mapping (e.g. -map trick.x=spots.x)");
    opts.add("-mapFile", &opts.mapFile, QString(""),
             "variable mapping file (e.g. -mapFile myMapFile.txt)");
    opts.add("-debug:{0,1}",&opts.isDebug,false, "Show book model tree etc.");
    opts.add("-legend:{0,1}",&opts.isLegend,true, "Show legend");
    opts.add("-l1",&opts.legend1,"", "Curve label 1");
    opts.add("-l2",&opts.legend2,"", "Curve label 2");
    opts.add("-l3",&opts.legend3,"", "Curve label 3");
    opts.add("-l4",&opts.legend4,"", "Curve label 4");
    opts.add("-l5",&opts.legend5,"", "Curve label 5");
    opts.add("-l6",&opts.legend6,"", "Curve label 6");
    opts.add("-l7",&opts.legend7,"", "Curve label 7");
    opts.add("-c1",&opts.color1,"","Curve color 1 <#rrggbb|colorName>");
    opts.add("-c2",&opts.color2,"","Curve color 2 <#rrggbb|colorName>");
    opts.add("-c3",&opts.color3,"","Curve color 3 <#rrggbb|colorName>");
    opts.add("-c4",&opts.color4,"","Curve color 4 <#rrggbb|colorName>");
    opts.add("-c5",&opts.color5,"","Curve color 5 <#rrggbb|colorName>");
    opts.add("-c6",&opts.color6,"","Curve color 6 <#rrggbb|colorName>");
    opts.add("-c7",&opts.color7,"","Curve color 7 <#rrggbb|colorName>");
    opts.add("-ls1",&opts.linestyle1,"","Curve Linestyle 1");
    opts.add("-ls2",&opts.linestyle2,"","Curve Linestyle 2");
    opts.add("-ls3",&opts.linestyle3,"","Curve Linestyle 3");
    opts.add("-ls4",&opts.linestyle4,"","Curve Linestyle 4");
    opts.add("-ls5",&opts.linestyle5,"","Curve Linestyle 5");
    opts.add("-ls6",&opts.linestyle6,"","Curve Linestyle 6");
    opts.add("-ls7",&opts.linestyle7,"","Curve Linestyle 7");
    opts.add("-s1",&opts.symbolstyle1,"","Curve Symbolstyle 1");
    opts.add("-s2",&opts.symbolstyle2,"","Curve Symbolstyle 2");
    opts.add("-s3",&opts.symbolstyle3,"","Curve Symbolstyle 3");
    opts.add("-s4",&opts.symbolstyle4,"","Curve Symbolstyle 4");
    opts.add("-s5",&opts.symbolstyle5,"","Curve Symbolstyle 5");
    opts.add("-s6",&opts.symbolstyle6,"","Curve Symbolstyle 6");
    opts.add("-s7",&opts.symbolstyle7,"","Curve Symbolstyle 7");
    opts.add("-se1",&opts.symbolend1,"","Curve Symbolend 1");
    opts.add("-se2",&opts.symbolend2,"","Curve Symbolend 2");
    opts.add("-se3",&opts.symbolend3,"","Curve Symbolend 3");
    opts.add("-se4",&opts.symbolend4,"","Curve Symbolend 4");
    opts.add("-se5",&opts.symbolend5,"","Curve Symbolend 5");
    opts.add("-se6",&opts.symbolend6,"","Curve Symbolend 6");
    opts.add("-se7",&opts.symbolend7,"","Curve Symbolend 7");
    opts.add("-g1",&opts.group1,"","Group rgx/range 1 e.g. -g1 \"_unstable_\"");
    opts.add("-g2",&opts.group2,"","Group rgx/range 2 e.g. -g2 \"0,50\"");
    opts.add("-g3",&opts.group3,"","Group rgx/range 3 e.g. -g3 \"a|b\"");
    opts.add("-g4",&opts.group4,"","Group rgx/range 4");
    opts.add("-g5",&opts.group5,"","Group rgx/range 5");
    opts.add("-g6",&opts.group6,"","Group rgx/range 6");
    opts.add("-g7",&opts.group7,"","Group rgx/range 7");
    opts.add("-orient",&opts.orient,"",
             "PDF page orientation - landscape or portrait",
             presetOrientation);
    opts.add("-session",&opts.sessionFile,"","session file name",
             presetExistsFile);
    opts.add("-fg",&opts.foreground,"","Page foreground <#rrggbb|colorName>");
    opts.add("-bg",&opts.background,"","Page background <#rrggbb|colorName>");
    opts.add("-showTables",&opts.showTables,"","Show DP tables");
    opts.add("-a:{0,1}",&opts.isPlotAllVars,false,"Plot all variables");
    opts.add("-script",&opts.scripts,"",
             "List of user scripts e.g. -script \"&myscript1 arg1 arg2, "
             "&helloworld\"");
    opts.add("-exclude",&opts.excludePattern,"",
             "exclude pattern to filter out RUNs and/or log files");
    opts.add("-filter",&opts.filterPattern,"",
             "filter pattern to filter for RUNs and/or log files");
    opts.add("-tmt", &opts.timeMatchTolerance, DBL_MAX,
             "time match tolerance for error plots");
    opts.add("-trickhost", &opts.trickhost, "127.0.0.1",
             "trick var server host");
    opts.add("-trickport", &opts.trickport, 0,
             "trick var server port");
    opts.add("-trickoffset", &opts.trickoffset, 0.0,
             "trick var server time sync offset");
    opts.add("-video", &opts.videoFileName, "",
             "mp4 video filename");
    opts.add("-videoOffset", &opts.videoOffset, 0.0,
             "video time sync offset");
    opts.add("-videoList", &opts.videoList, "",
             "list of videos and colon delimited offsets "
             "e.g. \"myvideo.mp4:123.4,myothervideo.mp4:567.8\"");
    opts.add("-videoDir", &opts.videoDir, "",
             "relative path to video directory from RUN e.g. ../Video");
    opts.add("-showVideo", &opts.showVideo,"",
             "Show video if possible - valid values yes,no,1,0 etc.");
    opts.add("-units", &opts.unitOverrides, "",
             "comma delimited list of override units e.g. -units \"in,d\"");
    opts.add("-showUnits:{0,1}",&opts.isShowUnits,false,
             "Print available units");
    opts.add("-showPageTitle:{0,1}",
             &opts.isShowPageTitle,true, "Show page title and page legend?");
    opts.add("-showPlotLegend",
             &opts.isShowPlotLegend,"", "Show plot legend if possible",
             presetIsShowPlotLegend);
    opts.add("-plotLegendPosition",
             &opts.plotLegendPosition,"","Valid positions are ne,n,nw,w etc.",
             presetplotLegendPosition);
    opts.add("-buttonSelectAndPan",
             &opts.buttonSelectAndPan,"left","left or right mouse button");
    opts.add("-buttonZoom",
             &opts.buttonZoom,"middle","left, middle or right mouse button");
    opts.add("-buttonReset",
             &opts.buttonReset,"right","left, middle or right mouse button");
    opts.add("-platform",
             &opts.platform,"","Set to \"offscreen\" for pdf without X");
    opts.add("-xaxislabel",
             &opts.xaxislabel,"","X axis label override");
    opts.add("-yaxislabel",
             &opts.yaxislabel,"","Y axis label override");
    opts.add("-vars",
             &opts.vars,"","List variables to plot. "
                        "Use @var to place var on same plot as prev variable.");
    opts.add("-liveTime",
             &opts.liveTime,"", "Select first curve and set live time arrow.  "
                                "Videos will start paused at given time.");
    opts.add("-z:{0,1}",
             &opts.isFilterOutFlatlineZeros,false,
             "Filter out flat line zero and empty plots");

    opts.parse(argc,argv, QString("koviz"), &ok);

    if ( opts.isHelp ) {
        fprintf(stdout,"%s\n",opts.usage().toLatin1().constData());
        return 0;
    }

    if ( opts.isShowUnits ) {
        QString msg = Unit::showUnits();
        fprintf(stdout, "%s", msg.toLatin1().constData());
        return 0;
    }

    if ( !ok ) {
        return -1;
    }

    QStringList dps;
    QStringList runPaths;
    foreach ( QString f, opts.rundps ) {
        QFileInfo fi(f);
        if ( fi.fileName().startsWith("DP_") ) {
            dps << f;
        } else if ( fi.isDir() ) {
            runPaths << f;
        } else if ( fi.isFile() ) {
            runPaths << f;
        }
    }

    Session* session = 0;
    if ( !opts.sessionFile.isEmpty() ) {
        session = new Session(opts.sessionFile);
    }

    if ( session ) {
        runPaths << session->runs();

        if ( runPaths.isEmpty() ) {
            fprintf(stderr,"koviz [error]: no RUNs "
                           "specified in session_file=\"%s\"\n",
                    opts.sessionFile.toLatin1().constData());
            exit(-1);
        }
        dps << session->dps();
    }

    // Error check
    if ( runPaths.isEmpty() && opts.trickport == 0 && !dps.isEmpty() ) {
        fprintf(stderr, "koviz [error]: DP given but no RUNs specified\n");
        exit(-1);
    }

    // Var Map
    QString mapString = opts.map;
    if ( session ) {
        if ( mapString.isEmpty() && !session->map().isEmpty() ) {
            mapString = session->map();
        }
    }
    QString mapFile = opts.mapFile;
    if ( session ) {
        if ( mapFile.isEmpty() && !session->mapFile().isEmpty() ) {
            mapFile = session->mapFile();
        }
    }
    if ( !mapString.isEmpty() && !mapFile.isEmpty() ) {
        fprintf(stderr,"koviz [error] : the -map and -mapFile cannot be "
                       "used together.  Please use one or the other.\n");
        exit(-1);
    }
    QHash<QString,QStringList> varMap;
    if ( !mapString.isEmpty() ) {
        varMap = getVarMap(mapString);
    } else if ( !mapFile.isEmpty() ) {
        varMap = getVarMapFromFile(mapFile);
    }
    foreach ( QString key, varMap.keys() ) {
        if ( key.contains(':') ) {
            fprintf(stderr, "koviz [error]: bad mapkey=\"%s\". "
                            "Keys cannot contain colons.\n",
                    key.toLatin1().constData());
            exit(-1);
        }
        if ( key.contains('{') || key.contains('(')) {
            fprintf(stderr, "koviz [error]: bad mapkey=\"%s\". "
                            "Keys cannot contain units, scales or bias.\n",
                    key.toLatin1().constData());
            exit(-1);
        }
    }

    // Time Name
    QString timeName = opts.timeName;
    if ( timeName.isEmpty() && session ) {
        timeName = session->timeName();
    }
    if ( timeName.isEmpty() ) {
        timeName = "sys.exec.out.time";
    }
    QStringList timeNames = getTimeNames(timeName);
    timeName = timeNames.at(0);

    // Start time
    double startTime = opts.start;
    if ( startTime == -DBL_MAX && session ) {
        startTime = session->start();
    }

    // Stop time
    double stopTime = opts.stop;
    if ( stopTime == DBL_MAX && session ) {
        stopTime = session->stop();
    }

    // Time match tolerance
    double tmt = opts.timeMatchTolerance;
    if ( tmt == DBL_MAX ) {
        if ( session ) {
            tmt = session->timeMatchTolerance();
        } else {
            tmt = 0.0000001;  // 10th of a microsecond
        }
    }

    // Check to ensure time isn't scaled or biased via the map
    foreach (QString key, varMap.keys() ) {
        foreach (QString val, varMap.value(key)) {
            MapValue mapval(val);
            foreach ( timeName, timeNames ) {
                if ( mapval.name() == timeName ) {
                    if ( mapval.bias() != 0.0 ) {
                        fprintf(stderr, "koviz [error]: var map is attempting "
                                "to bias time.  Koviz doesn't not "
                                "support this.\n");
                        exit(-1);
                    }
                    if ( mapval.scale() != 1.0 ) {
                        fprintf(stderr, "koviz [error]: var map is attempting "
                                "to scale time.  Koviz doesn't not "
                                "support this.\n");
                        exit(-1);
                    }
                }
            }
        }
    }

    // Exclude and Filter patterns
    QString excludePattern = opts.excludePattern;
    if ( excludePattern.isEmpty() && session ) {
        excludePattern = session->excludePattern();
    }
    QString filterPattern = opts.filterPattern;
    if ( filterPattern.isEmpty() && session ) {
        filterPattern = session->filterPattern();
    }

    // Video(s) and VideoOffset(s)
    QList<QPair<QString,double>> videos;  // list mp4name/video_offset pairs
    QString videoFileName = opts.videoFileName;
    if ( videoFileName.isEmpty() && session ) {
        videoFileName = session->videoFileName();
    }
    double videoOffset = opts.videoOffset;
    if ( videoOffset == 0.0 && session ) {
        videoOffset = session->videoOffset();
    }
    if ( !videoFileName.isEmpty() ) {
        videos.append(qMakePair(videoFileName,videoOffset));
    }
    if ( !opts.videoList.isEmpty() ) {
        if ( !videoFileName.isEmpty() ) {
            fprintf(stderr, "koviz [error]: Cannot use -video and "
                             "-videoList options together.");
            exit(-1);
        }

        QStringList items = opts.videoList.split(',',skipEmptyParts);
        foreach ( QString item, items ) {
            if ( item.contains(':') ) {
                QString f = item.split(':',skipEmptyParts).at(0).trimmed();
                QString s = item.split(':',skipEmptyParts).at(1);
                bool ok;
                double o = s.toDouble(&ok);
                if ( !ok ) {
                    fprintf(stderr, "koviz [error]: Bad offset=%s "
                                     "in -videoList option=%s\n",
                            s.toLatin1().constData(),
                            opts.videoList.toLatin1().constData());
                    exit(-1);
                }
                videos.append(qMakePair(f,o));
            } else {
                videos.append(qMakePair(item.trimmed(),0.0));
            }
        }
    }

    // -videoDir: relative path from RUN to video directory
    QString videoDir;
    if ( !opts.videoDir.isEmpty() ) {
        videoDir = opts.videoDir;
    } else {
        // -videoDir option not used
        QSettings settings("JSC", "koviz");
        if (!settings.contains("VideoWindow/videoDir")) {
            // Make koviz.conf's video directory default to "video"
            settings.setValue("VideoWindow/videoDir", "video");
        }
        videoDir = settings.value("VideoWindow/videoDir").toString();
    }

    // Enable/disable showing video
    bool showVideo;
    if ( !opts.showVideo.isEmpty() ) {
        bool ok;
        bool isShow = Options::stringToBool(opts.showVideo,&ok);
        if ( ok ) {
            showVideo = isShow;
        } else {
            fprintf(stderr, "koviz [error]: Bad option \"%s\" for -showVideo.  "
                            "Should be a boolean like true,yes,no,0,1 etc.\n",
                            opts.showVideo.toLatin1().constData());
            exit(-1);
        }
    } else {
        // -showVideo not set
        if ( !opts.videoFileName.isEmpty() || !opts.videoList.isEmpty() ) {
            // If -video or -videoList option set, show video
            showVideo = true;
        } else {
            // Else use what's in the koviz.conf settings file
            QSettings settings("JSC", "koviz");
            if (!settings.contains("VideoWindow/showVideo")) {
                // Make koviz.conf's showVideo default true
                settings.setValue("VideoWindow/showVideo", true);
            }
            showVideo = settings.value("VideoWindow/showVideo").toBool();
        }
    }

    if ( !opts.trk2csvFile.isEmpty() ) {
        QString csvOutFile = opts.outputFileName;
        if ( csvOutFile.isEmpty() ) {
            QFileInfo fi(opts.trk2csvFile);
            csvOutFile = fi.absolutePath() + "/" +
                         QString("%1.csv").arg(fi.baseName());
        }
        bool ok = false;
        double shift = opts.shiftString.toDouble(&ok);
        if ( !ok ) {
            shift = 0.0;
        }
        bool ret;
        try {
            ret = convert2csv(timeNames,opts.trk2csvFile, csvOutFile,
                              startTime,stopTime,shift,tmt);
        } catch (std::exception &e) {
            fprintf(stderr,"\n%s\n",e.what());
            exit(-1);
        }
        if ( !ret )  {
            fprintf(stderr, "koviz [error]: Aborting trk to csv conversion!\n");
            return -1;
        }
    }

    if ( !opts.csv2trkFile.isEmpty() ) {
        QString trkOutFile = opts.outputFileName;
        if ( trkOutFile.isEmpty() ) {
            QFileInfo fi(opts.csv2trkFile);
            trkOutFile = fi.absolutePath() + "/" +
                         QString("%1.trk").arg(fi.baseName());
        }
        bool ok = false;
        double shift = opts.shiftString.toDouble(&ok);
        if ( !ok ) {
            shift = 0.0;
        }
        bool ret = convert2trk(opts.csv2trkFile,trkOutFile,timeNames,
                               startTime,stopTime,shift,tmt);
        if ( !ret )  {
            fprintf(stderr, "koviz [error]: Aborting csv to trk conversion!\n");
            return -1;
        }
    }

    try {
        if ( opts.isReportRT ) {
            foreach ( QString run, runPaths ) {
                if ( opts.start != -DBL_MAX || opts.stop != DBL_MAX ) {
                    fprintf(stderr, "snap [warning]: when using the -rt option "
                                    "the -start/stop options are ignored\n");
                }
                VersionNumber version = TrickVersion(run).versionNumber();
                if ( version < VersionNumber("17.0.0-0") ) {
                    fprintf(stderr, "koviz [error]: The -rt realtime option "
                            "requires Trick version 17.0.0+.\n");
                    exit(-1);
                }
                Snap snap(run,timeNames);
                SnapReport rpt(snap);
                fprintf(stderr,"%s",rpt.report().toLatin1().constData());
            }
        }
    } catch (std::exception &e) {
        fprintf(stderr,"\n%s\n",e.what());
        exit(-1);
    }


    if ( opts.isReportRT || !opts.csv2trkFile.isEmpty()
         || !opts.trk2csvFile.isEmpty() ) {
        return 0;
    }

    try {

#if QT_VERSION < 0x040800
        QApplication::setGraphicsSystem("raster");
#endif
        QApplication a(argc, argv);

        Runs* runs = 0;
        QStandardItemModel* varsModel = 0;
        QStandardItemModel* monteInputsModel = 0;

        bool isDP2Trk = false;
        if ( !opts.dp2trkOutFile.isEmpty() ) {
            isDP2Trk = true;
        }

        QString pdfOutFile;
        bool isPdf = false;
        if ( !opts.pdfOutFile.isEmpty() ) {
            pdfOutFile = opts.pdfOutFile;
            isPdf = true;
        } else if ( session ) {
            QString device = session->device();
            if ( device != "terminal" ) {
                pdfOutFile = device;
                isPdf = true;
            }
        }

        QString jpgOutFile;
        bool isJpg = false;
        if ( !opts.jpgOutFile.isEmpty() ) {
            jpgOutFile = opts.jpgOutFile;
            isJpg = true;
        }

        bool isDP2Csv = false;
        if ( !opts.dp2csvOutFile.isEmpty() ) {
            isDP2Csv = true;
        }

        bool isVars2Csv = false;
        if ( !opts.vars2csvFile.isEmpty() ) {
            isVars2Csv = true;
        }

        bool isVars2Vals = false;
        if ( !qIsNaN(opts.vars2valsAtTime)) {
            isVars2Vals = true;
        }

        if ( ((isPdf || isJpg) && isDP2Trk) ||
             ((isPdf || isJpg) && isDP2Csv) ||
             ((isPdf || isJpg) && isVars2Csv) ||
             ((isPdf || isJpg) && isVars2Vals) ||
             (isPdf && isJpg) ||
             (isDP2Trk && isDP2Csv) ) {
            fprintf(stderr,
                    "koviz [error] : you may not use the -pdf, -jpg, -trk, "
                    "-csv and -vars options together.");
            exit(-1);
        }

        // If outputting to pdf, you must have a DP file and RUN dir
        if ( isPdf &&
            ((!opts.isPlotAllVars && dps.size() == 0) || runPaths.size() == 0)&&
            ((opts.vars.isEmpty() && dps.size() == 0) || runPaths.size() == 0)){
            fprintf(stderr,
                    "koviz [error] : when using the -pdf option you must "
                    "specify a RUN directory and DP product file "
                    "(or -a or -vars option) \n");
            exit(-1);
        }

        // If outputting to jpg, you must have a DP file and RUN dir
        if ( isJpg && (dps.size() == 0 || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -jpg option you must "
                    "specify a RUN directory and DP product file \n");
            exit(-1);
        }

        // If outputting to trk, you must have a DP file and RUN
        if ( isDP2Trk && (dps.size() == 0 || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -trk option you must "
                    "specify a DP product file and RUN directory\n");
            exit(-1);
        }

        // If outputting to csv, you must have a DP file and RUN
        if ( isDP2Csv && (dps.size() == 0 || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -csv option you must "
                    "specify a DP product file and RUN directory\n");
            exit(-1);
        }

        // If using the -vars2csv or -vars2valsAtTime options,
        // the -vars and RUN options must be set
        if ( (isVars2Vals || isVars2Csv) &&
             (opts.vars.isEmpty() || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -vars2csv or "
                    "-vars2valsAtTime option you must "
                    "specify a non-empty -vars list as well as a RUN\n");
            exit(-1);

        }

        bool isMonte = false;
        if ( runPaths.size() == 1 ) {
            QFileInfo fileInfo(runPaths.at(0));
            if ( fileInfo.fileName().startsWith("MONTE_") ) {
                isMonte = true;
            }
        } else if ( runPaths.size() == 0 &&
                    opts.trickport > 0 ) {
            // Add var server "run path"
            QString tvpath = QString("trick://%1:%2").
                             arg(opts.trickhost).arg(opts.trickport);
            runPaths.append(tvpath);
        }

        bool isShowProgress = true;
        if ( isPdf || isJpg ) {
            isShowProgress = false;
        }

        runs = new Runs(timeNames,tmt,runPaths,varMap,
                        filterPattern,
                        excludePattern,
                        opts.beginRun,opts.endRun,
                        isShowProgress);
        monteInputsModel = runs->runsModel();
        varsModel = createVarsModel(runs);

        // Make a list of titles
        QStringList titles;
        QString title = opts.title1;
        if ( title.isEmpty() ) {
            if ( session ) {
                title = session->title1();
            }
            if ( title.isEmpty() ) {
                title = "koviz ";
                if ( isMonte ) {
                    title += runPaths.at(0);
                } else {
                    if ( runPaths.size() == 1 ) {
                        title += runPaths.at(0);
                    } else if ( runPaths.size() == 2 ) {
                        title += runPaths.at(0) + " " + runPaths.at(1);
                    } else if ( runPaths.size() > 2 ) {
                        title += runPaths.at(0) + " " + runPaths.at(1) + "...";
                    }
                }
            }
        }
        titles << title;

        // Default title2 to RUN names
        title = opts.title2;
        if ( title.isEmpty() ) {
            if ( session ) {
                title = session->title2();
            }
            if ( title.isEmpty() ) {
                if ( !runPaths.isEmpty() ) {
                    title = "(";
                    foreach ( QString runPath, runPaths ) {
                        title += runPath + ",\n";
                    }
                    title.chop(2);
                    title += ")";
                }
            }
        }
        titles << title;

        // Default title3 to username
        title = opts.title3;
        if ( title.isEmpty() ) {
            if ( session ) {
                title = session->title3();
            }
            if ( title.isEmpty() ) {
                QString homeDir = QStandardPaths::writableLocation(
                                                  QStandardPaths::HomeLocation);
                QDir dir(homeDir);
                QString userName = dir.dirName();
                title = "User: " + userName;
            }
        }
        titles << title;

        // Default title4 to date
        title = opts.title4;
        if ( title.isEmpty() ) {
            if ( session ) {
                title = session->title4();
            }
            if ( title.isEmpty() ) {
                QDate date = QDate::currentDate();
                QString fmt("Date: MMMM d, yyyy");
                QString dateStr = date.toString(fmt);
                title = dateStr;
            }
        }
        titles << title;

        // Presentation
        QString presentation = opts.presentation;
        if ( presentation.isEmpty() && session ) {
            presentation = session->presentation();
        }

        // Make a list of legend labels
        QStringList legends;
        if ( session ) {
            legends << session->legend1() << session->legend2()
                    << session->legend3() << session->legend4()
                    << session->legend5() << session->legend6()
                    << session->legend7();
            if ( !opts.legend1.isEmpty() ) { legends.replace(0,opts.legend1); }
            if ( !opts.legend2.isEmpty() ) { legends.replace(1,opts.legend2); }
            if ( !opts.legend3.isEmpty() ) { legends.replace(2,opts.legend3); }
            if ( !opts.legend4.isEmpty() ) { legends.replace(3,opts.legend4); }
            if ( !opts.legend5.isEmpty() ) { legends.replace(4,opts.legend5); }
            if ( !opts.legend6.isEmpty() ) { legends.replace(5,opts.legend6); }
            if ( !opts.legend7.isEmpty() ) { legends.replace(6,opts.legend7); }
        } else {
            legends << opts.legend1 << opts.legend2 << opts.legend3
                    << opts.legend4 << opts.legend5 << opts.legend6
                    << opts.legend7;
        }

        // Make a list of user given curve colors
        QStringList colors;
        if ( session ) {
            colors << session->color1() << session->color2()
                   << session->color3() << session->color4()
                   << session->color5() << session->color6()
                   << session->color7();
            if ( !opts.color1.isEmpty() ) { colors.replace(0,opts.color1); }
            if ( !opts.color2.isEmpty() ) { colors.replace(1,opts.color2); }
            if ( !opts.color3.isEmpty() ) { colors.replace(2,opts.color3); }
            if ( !opts.color4.isEmpty() ) { colors.replace(3,opts.color4); }
            if ( !opts.color5.isEmpty() ) { colors.replace(4,opts.color5); }
            if ( !opts.color6.isEmpty() ) { colors.replace(5,opts.color6); }
            if ( !opts.color7.isEmpty() ) { colors.replace(6,opts.color7); }
        } else {
            colors << opts.color1 << opts.color2 << opts.color3
                   << opts.color4 << opts.color5 << opts.color6 << opts.color7;
        }

        // Make a list of user given linestyles
        QStringList linestyles;
        if ( session ) {
            linestyles << session->linestyle1() << session->linestyle2()
                       << session->linestyle3() << session->linestyle4()
                       << session->linestyle5() << session->linestyle6()
                       << session->linestyle7();
            if ( !opts.linestyle1.isEmpty() ) {
                linestyles.replace(0,opts.linestyle1);
            }
            if ( !opts.linestyle2.isEmpty() ) {
                linestyles.replace(1,opts.linestyle2);
            }
            if ( !opts.linestyle3.isEmpty() ) {
                linestyles.replace(2,opts.linestyle3);
            }
            if ( !opts.linestyle4.isEmpty() ) {
                linestyles.replace(3,opts.linestyle4);
            }
            if ( !opts.linestyle5.isEmpty() ) {
                linestyles.replace(4,opts.linestyle5);
            }
            if ( !opts.linestyle6.isEmpty() ) {
                linestyles.replace(5,opts.linestyle6);
            }
            if ( !opts.linestyle7.isEmpty() ) {
                linestyles.replace(6,opts.linestyle7);
            }
        } else {
            linestyles << opts.linestyle1 << opts.linestyle2 << opts.linestyle3
                       << opts.linestyle4 << opts.linestyle5 << opts.linestyle6
                       << opts.linestyle7;
        }

        // Make a list of user given symbolstyles
        QStringList symbolstyles;
        if ( session ) {
            symbolstyles << session->symbolstyle1() << session->symbolstyle2()
                         << session->symbolstyle3() << session->symbolstyle4()
                         << session->symbolstyle5() << session->symbolstyle6()
                         << session->symbolstyle7();
            if ( !opts.symbolstyle1.isEmpty() ) {
                symbolstyles.replace(0,opts.symbolstyle1);
            }
            if ( !opts.symbolstyle2.isEmpty() ) {
                symbolstyles.replace(1,opts.symbolstyle2);
            }
            if ( !opts.symbolstyle3.isEmpty() ) {
                symbolstyles.replace(2,opts.symbolstyle3);
            }
            if ( !opts.symbolstyle4.isEmpty() ) {
                symbolstyles.replace(3,opts.symbolstyle4);
            }
            if ( !opts.symbolstyle5.isEmpty() ) {
                symbolstyles.replace(4,opts.symbolstyle5);
            }
            if ( !opts.symbolstyle6.isEmpty() ) {
                symbolstyles.replace(5,opts.symbolstyle6);
            }
            if ( !opts.symbolstyle7.isEmpty() ) {
                symbolstyles.replace(6,opts.symbolstyle7);
            }
        } else {
            symbolstyles << opts.symbolstyle1 << opts.symbolstyle2
                         << opts.symbolstyle3 << opts.symbolstyle4
                         << opts.symbolstyle5 << opts.symbolstyle6
                         << opts.symbolstyle7;
        }

        // Make a list of user given end symbols
        QStringList symbolends;
        if ( session ) {
            symbolends << session->symbolend1() << session->symbolend2()
                       << session->symbolend3() << session->symbolend4()
                       << session->symbolend5() << session->symbolend6()
                       << session->symbolend7();
            if ( !opts.symbolend1.isEmpty() ) {
                symbolends.replace(0,opts.symbolend1);
            }
            if ( !opts.symbolend2.isEmpty() ) {
                symbolends.replace(1,opts.symbolend2);
            }
            if ( !opts.symbolend3.isEmpty() ) {
                symbolends.replace(2,opts.symbolend3);
            }
            if ( !opts.symbolend4.isEmpty() ) {
                symbolends.replace(3,opts.symbolend4);
            }
            if ( !opts.symbolend5.isEmpty() ) {
                symbolends.replace(4,opts.symbolend5);
            }
            if ( !opts.symbolend6.isEmpty() ) {
                symbolends.replace(5,opts.symbolend6);
            }
            if ( !opts.symbolend7.isEmpty() ) {
                symbolends.replace(6,opts.symbolend7);
            }
        } else {
            symbolends << opts.symbolend1 << opts.symbolend2
                       << opts.symbolend3 << opts.symbolend4
                       << opts.symbolend5 << opts.symbolend6
                       << opts.symbolend7;
        }

        // Make a list of groups
        QStringList groups;
        if ( session ) {
            groups << session->group1() << session->group2()
                   << session->group3() << session->group4()
                   << session->group5() << session->group6()
                   << session->group7();
            if ( !opts.group1.isEmpty() ) {
                groups.replace(0,opts.group1);
            }
            if ( !opts.group2.isEmpty() ) {
                groups.replace(1,opts.group2);
            }
            if ( !opts.group3.isEmpty() ) {
                groups.replace(2,opts.group3);
            }
            if ( !opts.group4.isEmpty() ) {
                groups.replace(3,opts.group4);
            }
            if ( !opts.group5.isEmpty() ) {
                groups.replace(4,opts.group5);
            }
            if ( !opts.group6.isEmpty() ) {
                groups.replace(5,opts.group6);
            }
            if ( !opts.group7.isEmpty() ) {
                groups.replace(6,opts.group7);
            }
        } else {
            groups << opts.group1 << opts.group2
                         << opts.group3 << opts.group4
                         << opts.group5 << opts.group6
                         << opts.group7;
        }

#if QT_VERSION < 0x050000
        bool isGroups = false;
        foreach (QString group, groups ) {
            if ( !group.isEmpty() ) {
                isGroups = true;
                break;
            }
        }
        if ( isGroups ) {
            fprintf(stderr, "koviz [error]: Groups require Qt5!\n");
            fprintf(stderr, "               You may not use -g# options!\n");
            exit(-1);
        }
#endif

        // Frequency
        double frequency = 0.0;
        if ( session ) {
            frequency = session->frequency();
        }

        // Foreground
        QString fg = opts.foreground;
        if ( fg.isEmpty() && session ) {
            fg = session->foreground();
        }

        // Background
        QString bg = opts.background;
        if ( bg.isEmpty() && session ) {
            bg = session->background();
        }

        // Orientation
        QString orient = opts.orient;
        if ( orient.isEmpty() ) {
            if ( session ) {
                orient = session->orient();
            } else {
                orient = "landscape";
            }
        }

        // Shift
        QString shiftString = opts.shiftString;
        if ( opts.shiftString.isEmpty() && session ) {
            shiftString = session->shift();
        }
        QHash<QString,QVariant> shifts = getShiftHash(shiftString,runPaths);

        // Get time shift for single run
        double timeShift = 0.0;
        if ( shifts.size() == 1 ) {
            bool ok;
            timeShift = shifts.values().at(0).toDouble(&ok);
            if ( !ok ) {
                fprintf(stderr, "koviz [bad scoobs]: -shift <value> "
                                "cannot be converted to a double.\n");
                exit(-1);
            }
        }

        bool isShowPageTitle = opts.isShowPageTitle;
        if ( isShowPageTitle == true  && session ) {
            isShowPageTitle = session->isShowPageTitle();
        }

        // Is page legend
        bool isLegend = opts.isLegend;
        if ( isLegend == true && session ) {
            // Since bool is not tri-state, I can't detect if
            // the commandline was set explicitly to true, so will just take
            // the setting in the session file if there is a session
            isLegend = session->isLegend();
        }

        // Show plot legend?
        QString isShowPlotLegend;
        if ( !opts.isShowPlotLegend.isEmpty() ) {
            isShowPlotLegend = opts.isShowPlotLegend;
        } else if ( session ) {
            isShowPlotLegend = session->isShowPlotLegend();
        }
        if ( ! isShowPlotLegend.isEmpty() ) {
            bool ok;
            bool yesNo = Options::stringToBool(isShowPlotLegend,&ok);
            if ( ok ) {
                if ( yesNo == true ) {
                    isShowPlotLegend = "yes";
                } else {
                    isShowPlotLegend = "no";
                }
            }
        } else {
            // If -legend is off and -showPlotLegend is unset,
            // turn -showPlotLegend off
            if ( !isLegend ) {
                isShowPlotLegend = "no";
            }
        }

        // Plot legend position
        QString plotLegendPosition = "ne";
        if ( !opts.plotLegendPosition.isEmpty() ) {
            plotLegendPosition = opts.plotLegendPosition;
        } else if ( session ) {
            plotLegendPosition = session->plotLegendPosition();
        }

        // Show Tables (don't show if too many runs since it is *slow*)
        bool isShowTables = (isMonte || runPaths.size() > 7) ? false : true;
        if ( !opts.showTables.isEmpty() ) {  // use cmd line opt if set
            bool ok;
            isShowTables = Options::stringToBool(opts.showTables,&ok);
            if ( !ok ) {
                fprintf(stderr, "koviz [error]: cmd line option -showTables "
                                "has a value of \"%s\". Expected a boolean.\n",
                                 opts.showTables.toLatin1().constData());
                exit(-1);
            }
        } else {
            if ( session && !session->showTables().isEmpty() ) {
                bool ok;
                isShowTables = Options::stringToBool(session->showTables(),&ok);
                if ( !ok ) {
                    fprintf(stderr, "koviz [error]: session file \"%s\" "
                                    "has a showTables spec with value \"%s\".  "
                                    "Expected a boolean.\n",
                            opts.sessionFile.toLatin1().constData(),
                            opts.showTables.toLatin1().constData());
                    exit(-1);
                }
            }
        }

        // Unit overrides list
        QStringList unitOverridesList;
        foreach ( QString unitOverride, opts.unitOverrides.
                                        split(',',skipEmptyParts) ) {
            unitOverridesList << unitOverride.trimmed();
        }

        // Mouse buttons
        if ( opts.buttonSelectAndPan != "left" &&
             opts.buttonSelectAndPan != "right" ) {
            fprintf(stderr, "koviz [error]: -buttonSelectAndPan is set to "
                            "\"%s\", should be left or right.\n",
                    opts.buttonSelectAndPan.toLatin1().constData());
            exit(-1);
        }
        if ( opts.buttonZoom != "left" &&
             opts.buttonZoom != "right" &&
             opts.buttonZoom != "middle" ) {
            fprintf(stderr, "koviz [error]: -buttonZoom is set to "
                            "\"%s\", should be left,right or middle.\n",
                    opts.buttonZoom.toLatin1().constData());
            exit(-1);
        }
        if ( opts.buttonReset != "left" &&
             opts.buttonReset != "right" &&
             opts.buttonReset != "middle" ) {
            fprintf(stderr, "koviz [error]: -buttonReset is set to "
                            "\"%s\", should be left,right or middle.\n",
                    opts.buttonReset.toLatin1().constData());
            exit(-1);
        }
        if ( opts.buttonSelectAndPan == opts.buttonZoom ) {
            fprintf(stderr, "koviz [error]: buttonSelectAndPan and "
                    "buttonZoom both set to \"%s\".\n",
                    opts.buttonSelectAndPan.toLatin1().constData());
            exit(-1);
        }
        if ( opts.buttonSelectAndPan == opts.buttonReset ) {
            fprintf(stderr, "koviz [error]: buttonSelectAndPan and "
                    "buttonReset both set to \"%s\".\n",
                    opts.buttonSelectAndPan.toLatin1().constData());
            exit(-1);
        }
        if ( opts.buttonZoom == opts.buttonReset ) {
            fprintf(stderr, "koviz [error]: buttonZoom and "
                    "buttonReset both set to \"%s\".\n",
                    opts.buttonZoom.toLatin1().constData());
            exit(-1);
        }

        // Axis labels
        QString xaxislabel = opts.xaxislabel;
        if ( xaxislabel.isEmpty() && session ) {
            xaxislabel = session->xAxisLabel();
        }
        QString yaxislabel = opts.yaxislabel;
        if ( yaxislabel.isEmpty() && session ) {
            yaxislabel = session->yAxisLabel();
        }

        // Create book model
        PlotBookModel* bookModel = new PlotBookModel(timeNames,runs,0,1);
        if ( titles.size() == 4 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "DefaultPageTitles","");
            bookModel->addChild(citem, "Title1",titles.at(0));
            bookModel->addChild(citem, "Title2",titles.at(1));
            bookModel->addChild(citem, "Title3",titles.at(2));
            bookModel->addChild(citem, "Title4",titles.at(3));
            bookModel->addChild(rootItem, "TimeNames", timeNames);
            if ( opts.liveTime.isEmpty() ) {
                bookModel->addChild(rootItem, "LiveCoordTime","");
            } else {
                bookModel->addChild(rootItem, "LiveCoordTime",opts.liveTime);
            }
            bookModel->addChild(rootItem, "LiveCoordTimeIndex",0);
            bookModel->addChild(rootItem, "StartTime",startTime);
            bookModel->addChild(rootItem, "StopTime",stopTime);
            bookModel->addChild(rootItem, "Presentation",presentation);
            bookModel->addChild(rootItem, "IsShowLiveCoord",true);
            bookModel->addChild(rootItem, "RunToShiftHash",shifts);
        }
        if ( legends.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "LegendLabels","");
            bookModel->addChild(citem, "Label1",legends.at(0));
            bookModel->addChild(citem, "Label2",legends.at(1));
            bookModel->addChild(citem, "Label3",legends.at(2));
            bookModel->addChild(citem, "Label4",legends.at(3));
            bookModel->addChild(citem, "Label5",legends.at(4));
            bookModel->addChild(citem, "Label6",legends.at(5));
            bookModel->addChild(citem, "Label7",legends.at(6));
        }
        QStandardItem *rootItem = bookModel->invisibleRootItem();
        bookModel->addChild(rootItem, "Orientation", orient);
        bookModel->addChild(rootItem, "TimeMatchTolerance", tmt);
        bookModel->addChild(rootItem, "Frequency", frequency);
        bookModel->addChild(rootItem, "IsLegend", isLegend);
        if ( colors.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "LegendColors","");
            bookModel->addChild(citem, "Color1",colors.at(0));
            bookModel->addChild(citem, "Color2",colors.at(1));
            bookModel->addChild(citem, "Color3",colors.at(2));
            bookModel->addChild(citem, "Color4",colors.at(3));
            bookModel->addChild(citem, "Color5",colors.at(4));
            bookModel->addChild(citem, "Color6",colors.at(5));
            bookModel->addChild(citem, "Color7",colors.at(6));
        }
        bookModel->addChild(rootItem, "ForegroundColor", fg);
        bookModel->addChild(rootItem, "BackgroundColor", bg);
        if ( linestyles.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "Linestyles","");
            bookModel->addChild(citem, "Linestyle1",linestyles.at(0));
            bookModel->addChild(citem, "Linestyle2",linestyles.at(1));
            bookModel->addChild(citem, "Linestyle3",linestyles.at(2));
            bookModel->addChild(citem, "Linestyle4",linestyles.at(3));
            bookModel->addChild(citem, "Linestyle5",linestyles.at(4));
            bookModel->addChild(citem, "Linestyle6",linestyles.at(5));
            bookModel->addChild(citem, "Linestyle7",linestyles.at(6));
        }
        if ( symbolstyles.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "Symbolstyles","");
            bookModel->addChild(citem, "Symbolstyle1",symbolstyles.at(0));
            bookModel->addChild(citem, "Symbolstyle2",symbolstyles.at(1));
            bookModel->addChild(citem, "Symbolstyle3",symbolstyles.at(2));
            bookModel->addChild(citem, "Symbolstyle4",symbolstyles.at(3));
            bookModel->addChild(citem, "Symbolstyle5",symbolstyles.at(4));
            bookModel->addChild(citem, "Symbolstyle6",symbolstyles.at(5));
            bookModel->addChild(citem, "Symbolstyle7",symbolstyles.at(6));
        }
        if ( symbolends.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "Symbolends","");
            bookModel->addChild(citem, "Symbolend1",symbolends.at(0));
            bookModel->addChild(citem, "Symbolend2",symbolends.at(1));
            bookModel->addChild(citem, "Symbolend3",symbolends.at(2));
            bookModel->addChild(citem, "Symbolend4",symbolends.at(3));
            bookModel->addChild(citem, "Symbolend5",symbolends.at(4));
            bookModel->addChild(citem, "Symbolend6",symbolends.at(5));
            bookModel->addChild(citem, "Symbolend7",symbolends.at(6));
        }
        if ( groups.size() == 7 ) {
            QStandardItem *rootItem = bookModel->invisibleRootItem();
            QStandardItem *citem;
            citem = bookModel->addChild(rootItem, "Groups","");
            bookModel->addChild(citem, "Group1",groups.at(0));
            bookModel->addChild(citem, "Group2",groups.at(1));
            bookModel->addChild(citem, "Group3",groups.at(2));
            bookModel->addChild(citem, "Group4",groups.at(3));
            bookModel->addChild(citem, "Group5",groups.at(4));
            bookModel->addChild(citem, "Group6",groups.at(5));
            bookModel->addChild(citem, "Group7",groups.at(6));
        }
        bookModel->addChild(rootItem,"StatusBarMessage", "");
        bookModel->addChild(rootItem,"IsShowPageTitle", isShowPageTitle );
        bookModel->addChild(rootItem,"IsShowPlotLegend", isShowPlotLegend );
        bookModel->addChild(rootItem,"PlotLegendPosition",
                                     plotLegendPosition );
        bookModel->addChild(rootItem,"ButtonSelectAndPan",
                                     opts.buttonSelectAndPan );
        bookModel->addChild(rootItem,"ButtonZoom",opts.buttonZoom );
        bookModel->addChild(rootItem,"ButtonReset",opts.buttonReset );
        bookModel->addChild(rootItem,"XAxisLabel",xaxislabel );
        bookModel->addChild(rootItem,"YAxisLabel",yaxislabel );
        bookModel->addChild(rootItem,"ShowVideo",showVideo );
        bookModel->addChild(rootItem,"VideoDir", videoDir);

        if ( isDP2Csv || isDP2Trk ) {

            if ( runPaths.size() != 1 ) {
                fprintf(stderr, "koviz [error]: Exactly one RUN dir must be "
                                "specified with the -csv option.\n");
                exit(-1);
            }

            // If there are no tables, print a message
            int nTables = 0;
            foreach ( QString dpFileName, dps ) {
                DPProduct dp(dpFileName);
                nTables += dp.tables().size();
            }
            if ( nTables == 0 ) {
                fprintf(stderr, "koviz [error]: In order to create csv files "
                        "using the -dp2csv option, there must be "
                        "data product TABLEs in the DP files.\n");
                exit(-1);
            }

            int i = 0;
            foreach ( QString dpFileName, dps ) {
                DPProduct dp(dpFileName);
                foreach ( DPTable* dpTable, dp.tables() ) {
                    QString fname;
                    if ( isDP2Csv ) {
                        fname = opts.dp2csvOutFile;
                    } else if ( isDP2Trk ) {
                        fname = opts.dp2trkOutFile;
                    }
                    if ( dp.tables().size() > 1 ) {
                        // Multiple files to output, so index the name
                        QString dpName = QFileInfo(dpFileName).baseName();
                        QFileInfo fi(fname);
                        QString extension;
                        if ( isDP2Csv ) {
                            extension = "csv";
                        } else if ( isDP2Trk ) {
                            extension = "trk";
                        }
                        if ( !fi.suffix().isEmpty() ) {
                            extension = fi.suffix();
                        }
                        fname = fi.completeBaseName() +
                                "_" +
                                dpName +
                                QString("_%1.").arg(i) +
                                extension;
                        ++i;
                    }

                    bool r = false;
                    if ( isDP2Csv ) {
                        r = writeCsv(fname,timeNames,dpTable->vars(),
                                     runPaths.at(0),
                                     startTime, stopTime, timeShift, tmt);
                    } else if ( isDP2Trk ) {
                        r = writeTrk(fname,timeNames,dpTable->vars(),
                                     runPaths.at(0),
                                     startTime, stopTime, timeShift, tmt);
                    }
                    if ( r ) {
                        ret = 0;
                    } else {
                        fprintf(stderr, "koviz [error]: Failed to write: %s\n",
                                fname.toLatin1().constData());
                        ret = -1;
                        break;
                    }
                }
                if ( ret == -1 ) {
                    break;
                }
            }


        } else if ( isVars2Csv ) {

            if ( runPaths.size() != 1 ) {
                fprintf(stderr, "koviz [error]: Exactly one RUN dir must be "
                                "specified with the -vars2csv option.\n");
                exit(-1);
            }

            if ( opts.vars.isEmpty() ) {
                fprintf(stderr, "koviz [error]: -vars2csv option specified but"
                                "-vars option has no vars listed.\n");
                exit(-1);
            }

            QList<DPVar> dpvars = makeVarsList(opts.vars,runs);
            if ( dpvars.isEmpty() ) {
                fprintf(stderr, "koviz [error]: -vars2csv option could not "
                                "find var from -vars list in run.  Bailing!\n");
                exit(-1);
            }
            bool r = writeCsv(opts.vars2csvFile,timeNames,dpvars,
                              runPaths.at(0),
                              startTime,stopTime,timeShift,tmt);
            if ( !r ) {
                fprintf(stderr, "koviz [error]: -vars2csv had issue with "
                                "writing a csv file.  Aborting!\n");
                exit(-1);
            }


        } else if ( isVars2Vals ) {
            if ( runPaths.size() != 1 ) {
                fprintf(stderr, "koviz [error]: Exactly one RUN dir/file "\
                                "must be specified with the -vars2valsAtTime "\
                                "option.\n");
                exit(-1);
            }
            printVarValuesAtTime(opts.vars2valsAtTime,tmt,timeNames,
                                 opts.vars,runPaths.at(0),runs);
        } else {

            QString dpDir;
            QStringList listDPs;
            if ( dps.size() > 0 ) {
                listDPs = dps;
                dpDir = ".";
            } else if ( runPaths.size() > 0 ) {
                dpDir = runPaths.at(0);
            }

            PlotMainWindow w(bookModel,
                             opts.trickhost,
                             opts.trickport,
                             opts.trickoffset,
                             videos,
                             excludePattern,
                             filterPattern,
                             opts.isFilterOutFlatlineZeros,
                             opts.scripts,
                             opts.isDebug,
                             opts.isPlotAllVars,
                             timeNames,
                             dpDir,
                             listDPs,
                             isShowTables,
                             unitOverridesList,
                             mapString,
                             mapFile,
                             runs,
                             varsModel);

            //
            // Handle -vars commandline option
            //
            QStringList vars = opts.vars.split(",", skipEmptyParts);
            foreach (QString var, vars ) {
                QString v = var;
                if ( v.at(0) == '@' ) {
                    v = var.mid(1);
                }
                if ( !runs->params().contains(v) ) {
                    fprintf(stderr, "koviz [error]: Cannot find var=\"%s\" "
                                    "from -vars option.  Run(s) do not contain "
                                    "this variable.\n",
                                    var.toLatin1().constData());
                    exit(-1);
                }
            }
            int i = 0;
            QStandardItem* pageItem = 0;
            QModelIndex plotIdx;
            foreach ( QString var, vars ) {
                if ( i > 0 && var.at(0) == '@' ) {
                    // If var begins with @, place on same plot as last var
                    QModelIndex pageIdx = pageItem->index();
                    QModelIndex plotsIdx = bookModel->getIndex(pageIdx,
                                                                "Plots","Page");
                    int nplots = bookModel->rowCount(plotsIdx);
                    plotIdx = bookModel->index(nplots-1,0,plotsIdx);
                    QModelIndex curvesIdx = bookModel->getIndex(plotIdx,
                                                         "Curves", "Plot");
                    QString v = var.mid(1);
                    bookModel->createCurves(curvesIdx,timeName,v,
                                            unitOverridesList, 0,0);

                    // Set y axis label to empty string (since multiple vars)
                    QModelIndex yAxisLabelIdx = bookModel->getDataIndex(plotIdx,
                                                       "PlotYAxisLabel","Plot");
                    bookModel->setData(yAxisLabelIdx, "");
                } else {
                    if ( i%6 == 0 ) {
                        pageItem = bookModel->createPageItem();
                    }

                    QStandardItem* plotItem = bookModel->createPlotItem(
                                                          pageItem,
                                                          timeName,
                                                          var.trimmed(),
                                                          unitOverridesList,0);
                    plotIdx = plotItem->index();
                    ++i;
                }

                // Presentation
                QModelIndex presIdx = bookModel->getDataIndex(plotIdx,
                                                    "PlotPresentation", "Plot");
                if ( runs->runPaths().size() == 2 ) {
                    QModelIndex curvesIdx = bookModel->getIndex(plotIdx,
                                                               "Curves","Plot");
                    QModelIndexList curveIdxs = bookModel->getIndexList(
                                                    curvesIdx,"Curve","Curves");
                    if ( curveIdxs.size() == 2 && !presentation.isEmpty()) {
                        bookModel->setData(presIdx,presentation);
                        QRectF bbox = bookModel->calcCurvesBBox(curvesIdx);
                        bookModel->setPlotMathRect(bbox,plotIdx);
                    }
                }
            }

            if ( !opts.liveTime.isEmpty() ) {
                w.selectFirstCurve();
            }

            if ( isPdf ) {
                w.savePdf(pdfOutFile);
                ret = 0;
            } else if ( isJpg ) {
                QSize sz = str2size(opts.jpgSize);
                if ( !sz.isValid() ) {
                    fprintf(stderr, "koviz [error]: Invalid jpg size=%s. "
                                    "Size should be in the form widthxheight "
                                    "e.g. 2560x1440\n",
                                    opts.jpgSize.toLatin1().constData());
                    exit(-1);
                }
                w.saveJpgs(jpgOutFile,sz);
                ret = 0;
            } else {
                w.show();
                ret = a.exec();
            }
        }

        delete varsModel;
        delete monteInputsModel;
        delete runs;
        delete session;
        delete bookModel;

    } catch (std::exception &e) {
        fprintf(stderr,"\n%s\n",e.what());
        exit(-1);
    }

    return ret;
}

void presetRunsDPs(QStringList* defRunPaths,
                   const QStringList& rundps,bool* ok)
{
    Q_UNUSED(defRunPaths);

    foreach ( QString f, rundps ) {
        QFileInfo fi(f);
        if ( !fi.exists() ) {
            if (f.startsWith('-')) {
                fprintf(stderr,
                      "koviz [error] : option: \"%s\" does not exist.  "
                      "Try koviz -h to see options.\n",
                      f.toLatin1().constData());
            } else {
                fprintf(stderr,
                      "koviz [error] : couldn't find file/directory: \"%s\".\n",
                      f.toLatin1().constData());
            }
            *ok = false;
            return;
        }
    }
}

// Remove trailing /s on dir names
void postsetRunsDPs (QStringList* rundps, bool* ok)
{
    Q_UNUSED(ok);
    QStringList dirs = rundps->replaceInStrings(QRegularExpression("/*$"), "");
    Q_UNUSED(dirs);
}

QStandardItemModel* createVarsModel(Runs* runs)
{
    if ( runs == 0 ) return 0;

    QStandardItemModel* varsModel = new QStandardItemModel(0,1);

    QStringList params = runs->params();
    params.sort();
    QStandardItem *rootItem = varsModel->invisibleRootItem();
    foreach (QString param, params) {
        if ( param == "sys.exec.out.time" ) continue;
        QStandardItem *varItem = new QStandardItem(param);
        varItem->setData("VarsModel",Qt::UserRole); // For Drag-n-drop
        rootItem->appendRow(varItem);
    }

    return varsModel;
}

void presetBeginRun(uint* beginRunId, uint runId, bool* ok)
{
    Q_UNUSED(beginRunId);

    if ( runId > opts.endRun ) {
        fprintf(stderr,"koviz [error] : option -beginRun, set to %d, "
                "must be greater than "
                " -endRun which is set to %d\n",
                runId, opts.endRun);
        *ok = false;
    }
}

void presetEndRun(uint* endRunId, uint runId, bool* ok)
{
    Q_UNUSED(endRunId);

    if ( runId < opts.beginRun ) {
        fprintf(stderr,"koviz [error] : option -endRun, set to %d, "
                "must be greater than "
                "-beginRun which is set to %d\n",
                runId,opts.beginRun);
        *ok = false;
    }
}

void presetPresentation(QString* presVar, const QString& pres, bool* ok)
{
    Q_UNUSED(presVar);

    if ( !pres.isEmpty() && pres != "compare" && pres != "error" &&
         pres != "error+compare" ) {
        fprintf(stderr,"koviz [error] : option -presentation, set to \"%s\", "
                "should be \"compare\", \"error\" or \"error+compare\"\n",
                pres.toLatin1().constData());
        *ok = false;
    }
}

void presetOutputFile(QString* presVar, const QString& fname, bool* ok)
{
    Q_UNUSED(presVar);

    QFileInfo fi(fname);
    if ( fi.exists() ) {
        fprintf(stderr, "koviz [error]: Will not overwrite %s\n",
                fname.toLatin1().constData());
        *ok = false;
    }
}

void presetOrientation(QString* presVar, const QString& orient, bool* ok)
{
    Q_UNUSED(presVar);

    if ( orient != "landscape" && orient != "portrait" && !orient.isEmpty() ) {
        fprintf(stderr, "koviz [error]: option -orient set to \"%s\", "
                        "should be \"landscape\" or \"portrait\"\n",
                orient.toLatin1().constData());
        *ok = false;
    }
}

void presetIsShowPlotLegend(QString* presVar, const QString& isShow, bool* ok)
{
    Q_UNUSED(presVar)

    if ( !isShow.isEmpty() ) {
        bool yesNo = Options::stringToBool(isShow,ok);
        Q_UNUSED(yesNo)
        if ( !*ok ) {
            fprintf(stderr, "koviz [error]: option -isShowPlotLegend "
                            "set to \"%s\" should be \"yes\",\"no\" or \"\"\n",
                    isShow.toLatin1().constData());
        }
    }
}

void presetplotLegendPosition(QString* presVar,const QString& position,bool* ok)
{
    Q_UNUSED(presVar)

    if ( !position.isEmpty() ) {
        QStringList positions;
        positions << "ne" << "n" << "nw" << "w" << "sw" << "s" << "se" << "e";
        *ok = false;
        foreach (QString pos, positions) {
            if ( pos == position ) {
                *ok = true;
                break;
            }
        }
        if ( !*ok ) {
            fprintf(stderr,"koviz [error]: option -plotLegendPosition "
                           "set to \"%s\" should be \"ne\",\"n\",\"nw\" etc.\n",
                    position.toLatin1().constData());
        }
    }
}

bool writeCsv(const QString& fcsv,
               const QStringList& timeNames,
               const QList<DPVar>& vars,
               const QString& runPath,
               double start, double stop, double timeShift, double tmt)
{
    if ( timeNames.size() != 1 ) {
        fprintf(stderr, "koviz [error]: writeCsv expects -timeNames to be a "\
                        "single time name e.g. sys.exec.out.time\n");
        return false;
    }

    QFileInfo fcsvi(fcsv);
    if ( fcsvi.exists() ) {
        fprintf(stderr, "koviz [error]: Will not overwrite %s\n",
                fcsv.toLatin1().constData());
        return false;
    }

    // Open csv file for writing
    QFile csv(fcsv);
    if (!csv.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz: [error] could not open %s\n",
                fcsv.toLatin1().constData());
        return false;
    }
    QTextStream out(&csv);

    // Format output
    out.setRealNumberPrecision(DBL_DECIMAL_DIG);  // Precision normally 17
    out.setRealNumberNotation(QTextStream::SmartNotation); // like printf %g

    bool r = writeData(nullptr,&out,vars,runPath,timeNames,
                       start,stop,timeShift,tmt);

    // Clean up
    csv.close();

    return r;
}

bool writeTrk(const QString& ftrk,
               const QStringList& timeNames,
               const QList<DPVar>& vars,
               const QString& runPath,
               double start, double stop,
               double timeShift, double tmt)
{
    if ( timeNames.size() != 1 ) {
        fprintf(stderr, "koviz [error]: writeTrk expects -timeNames to be a "\
                        "single time name e.g. sys.exec.out.time\n");
        return false;
    }

    QFileInfo ftrki(ftrk);
    if ( ftrki.exists() ) {
        fprintf(stderr, "koviz [error]: Will not overwrite %s\n",
                ftrk.toLatin1().constData());
        return false;
    }

    // Print message
    fprintf(stderr, "\nkoviz [info]: extracting the following params "
                    "into %s:\n\n",
                    ftrk.toLatin1().constData());
    foreach ( const DPVar var, vars ) {
        fprintf(stderr, "    %s\n", var.name().toLatin1().constData());
    }
    fprintf(stderr, "\n");

    // Open trk file for writing
    QFile trk(ftrk);
    if (!trk.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz: [error] could not open %s\n",
                ftrk.toLatin1().constData());
        return false;
    }
    QDataStream out(&trk);

    // Write Trk
    writeData(&out,nullptr,vars,runPath,timeNames,start,stop,timeShift,tmt);

    // Clean up
    trk.close();

    return true;
}

// If outTrk/Csv streams are not-null, they will be written to.  This makes
// it so writing to trk and csv are as close as possible with just minor
// output changes (other than headers!)  Lambdas could have been used
// or maybe an abstract DataBodyWriter class with TrkBodyWriter
// and CsvBodyWriter, but this keeps it simple. This also outputs
// csv/trk headers.
bool writeData(QDataStream* outTrk, QTextStream* outCsv,
               const QList<DPVar>& vars,
               const QString& runPath,
               const QStringList& timeNames,
               double start, double stop, double timeShift, double tmt)
{
    // Get data models for given run
    QList<DataModel*> dataModels = runDataModels(runPath,timeNames);

    QString dp_tunit("s"); // If vars doesn't have time, default to seconds
    double ts = 1.0;
    double tb = 0.0;
    foreach ( const DPVar var, vars ) {
        if ( timeNames.contains(var.name()) ) {
            if ( !var.unit().isEmpty() ) {
                dp_tunit = var.unit();
            }
            ts = var.scaleFactor();
            tb = var.bias();
            break;
        }
    }

    // Get index aligned list of params, iterators, units, scales and biases
    QStringList params;
    QList<ModelIterator*> its;
    QList<double> tuss; // time unit scales
    QList<QString> uns;
    QList<double> uss;
    QList<double> ubs;
    QList<double> dpss;
    QList<double> dpbs;
    foreach ( const DPVar var, vars ) {
        if ( timeNames.contains(var.name()) ) {
            // Skip time since time printed to csv independently
            continue;
        }

        // Params
        params << var.name();

        bool isFound = false;
        foreach ( DataModel* dataModel, dataModels ) {
            int ycol = dataModel->paramColumn(var.name());
            if ( ycol >= 0 ) {
                int tcol = dataModel->paramColumn(timeNames.at(0));
                int xcol = dataModel->paramColumn(timeNames.at(0));

                // Iterator
                ModelIterator* it = dataModel->begin(tcol,xcol,ycol);
                its.append(it);

                // Time unit scale
                QString mo_tunit = dataModel->param(tcol)->unit();
                double tus = Unit::scale(mo_tunit,dp_tunit);
                tuss.append(tus);

                // Unit name
                QString dp_unit = var.unit();
                QString mo_unit = dataModel->param(ycol)->unit();
                if ( !dp_unit.isEmpty() ) {
                    uns.append(dp_unit);
                } else {
                    uns.append(mo_unit);
                }

                // Unit bias/scale
                double us = 1.0;
                double ub = 0.0;
                if ( !mo_unit.isEmpty() && !dp_unit.isEmpty() ) {
                    if ( Unit::canConvert(mo_unit,dp_unit) ) {
                        us = Unit::scale(mo_unit,dp_unit);
                        ub = Unit::bias(mo_unit,dp_unit);
                    } else {
                        fprintf(stderr, "koviz [error]: writeCsv is getting "
                                "conflicting units model_unit=%s dp_unit=%s "
                                "for var=%s. Aborting!\n",
                                mo_unit.toLatin1().constData(),
                                dp_unit.toLatin1().constData(),
                                var.name().toLatin1().constData());
                        return false;
                    }
                }
                uss.append(us);
                ubs.append(ub);

                // Var bias/scale
                dpss.append(var.scaleFactor());
                dpbs.append(var.bias());

                isFound = true;
                break;
            }
        }
        if ( !isFound ) {
            fprintf(stderr, "koviz [error]: writeCsv could not find"
                            "var=%s in run=%s\n",
                    var.name().toLatin1().constData(),
                    runPath.toLatin1().constData());
            foreach (ModelIterator *it, its) {
                delete it;
            }
            foreach (DataModel *dataModel, dataModels) {
                delete dataModel;
            }
            return false;
        }
    }

    // Csv header
    if ( outCsv ) {
        QString header;
        int c = 0;
        header += timeNames.at(0) + " {" + dp_tunit + "},";
        foreach ( QString param, params ) {
            header += param + " {" + uns.at(c++) + "}" + ",";
        }
        header.chop(1);
        *outCsv << header;
        *outCsv << "\n";
    }

    // Trk header
    if ( outTrk ) {
        // Make list of Trick params for making Trick header
        bool isTimeInserted = false;
        QList<TrickParameter> trick_params;
        foreach ( const DPVar var, vars ) {
            TrickParameter p;
            p.setName(var.name());
            if ( !var.unit().isEmpty() ) {
                p.setUnit(var.unit());
            } else {
                // Use model unit
                foreach ( DataModel* dataModel, dataModels ) {
                    int col = dataModel->paramColumn(var.name());
                    if ( col >= 0 ) {
                        QString mo_unit = dataModel->param(col)->unit();
                        p.setUnit(mo_unit);
                        break;
                    }
                }
            }
            p.setType(TRICK_10_DOUBLE);
            p.setSize(sizeof(double));
            if ( var.name() == timeNames.at(0) ) {
                if ( isTimeInserted ) {
                    continue;  // ignore multiple timestamps
                }
                trick_params.prepend(p);
                isTimeInserted = true;
            } else {
                trick_params.append(p);
            }
        }
        if ( !isTimeInserted ) {
            TrickParameter p;
            p.setName(timeNames.at(0));
            // Use model unit
            foreach ( DataModel* dataModel, dataModels ) {
                int col = dataModel->paramColumn(timeNames.at(0));
                if ( col >= 0 ) {
                    QString mo_unit = dataModel->param(col)->unit();
                    p.setUnit(mo_unit);
                    break;
                }
            }
            p.setType(TRICK_10_DOUBLE);
            p.setSize(sizeof(double));
            trick_params.prepend(p);
        }

        // Write Trk Header
        TrickModel::writeTrkHeader(*outTrk,trick_params);
    }

    // Write data block
    foreach ( DataModel* dataModel, dataModels ) {
        dataModel->map();
    }
    while ( 1 ) {

        // Break if iterators all done or past stop time
        bool isDone = true;
        foreach ( ModelIterator* it, its ) {
            if ( !it->isDone() ) {
                isDone = false;
                break;
            }
        }
        if ( isDone ) {
            break;
        }

        // Get current min time from iterators
        double minTime = DBL_MAX;
        int c = 0;
        foreach ( ModelIterator* it, its ) {
            if ( it->isDone() ) {
                ++c;
                continue;
            }
            double time = it->t();
            time = ts*tuss.at(c)*time + tb + timeShift;
            if ( time < minTime ) {
                minTime = time;
            }
            ++c;
        }

        if ( start-tmt <= minTime && minTime <= stop+tmt ) {
            // Output timestamp
            if ( outTrk ) *outTrk << minTime;
            if ( outCsv ) *outCsv << minTime << ",";

            // Output values at min/current time
            bool isFirst = true;
            c = 0;
            foreach ( ModelIterator* it, its ) {
                double tval = it->t();
                tval = ts*tuss.at(c)*tval + tb + timeShift;
                double yval = it->y();
                yval = dpss.at(c)*(uss.at(c)*yval+ubs.at(c)) + dpbs.at(c);
                if ( !isFirst ) {
                    if ( outCsv ) *outCsv << ",";
                }
                if ( qAbs(minTime-tval) <= tmt ) {
                    if ( outTrk ) *outTrk << yval;
                    if ( outCsv ) *outCsv << yval;
                } else {
                    // No value since time dne for this point
                    if ( outTrk ) *outTrk << qQNaN();  // Output NaN for trk
                    if ( outCsv ) {} // Leave field empty - do nothing
                }
                isFirst = false;
                ++c;
            }
            if ( outCsv ) *outCsv << "\n";
        }

        // Increment iterators that are on min time to next time
        c = 0;
        foreach ( ModelIterator* it, its ) {
            double tval = it->t();
            tval = ts*tuss.at(c)*tval + tb + timeShift;
            if ( qAbs(minTime-tval) <= tmt ) {
                it->next();
            }
            ++c;
        }

        // Break if past stop time
        if ( minTime > stop+tmt ) {
            break;
        }
    }

    // Clean up
    foreach ( ModelIterator* it, its ) {
        delete it;
    }
    foreach ( DataModel* dataModel, dataModels ) {
        dataModel->unmap();
        delete dataModel;
    }

    return true;
}

// Client must clean up returned allocated dataModels
QList<DataModel*> runDataModels(const QString& runPath,
                                const QStringList& timeNames)
{
    // Make list of dataModels for given run
    QList<DataModel*> dataModels;
    QFileInfo fi(runPath);
    if ( fi.isFile() ) {
        DataModel* dataModel = DataModel::createDataModel(timeNames,
                                                          runPath, runPath);
        dataModels.append(dataModel);
    } else if ( fi.isDir() ) {

        QDir dir(runPath);

        QStringList filter;
        filter << "*.trk" << "*.csv" << "*.mot";
        foreach(QString fileName, dir.entryList(filter, QDir::Files)) {
            if ( fileName == "_init_log.csv" ||
                 fileName == "log_timeline.csv" ||
                 fileName == "log_timeline_init.csv" ) {
                continue;
            }
            QString fullName = dir.absoluteFilePath(fileName);
            DataModel* dataModel = DataModel::createDataModel(timeNames,runPath,
                                                              fullName);
            dataModels.append(dataModel);
        }
        if ( dataModels.empty() ) {
            fprintf(stderr,"koviz [error]: no trk,csv,mot logfiles found in "
                           "runPath=%s\n", runPath.toLatin1().constData());
            exit(-1);
        }
    }

    return dataModels;
}


// This will print the value closest to the record with timestamp.  This is
// important if the RUN has files with multiple frequencies or if timestamp
// given is recorded.
bool printVarValuesAtTime(double time, double tmt, const QStringList& timeNames,
                          const QString& varsOptString, const QString& runPath,
                          Runs* runs)
{
    if ( timeNames.size() != 1 ) {
        fprintf(stderr, "koviz [error]: printVarValuesAtTime expects "
                        "-timeNames to be a single time name "\
                        "e.g. sys.exec.out.time\n");
        return false;
    }

    QList<DPVar> dpvars = makeVarsList(varsOptString,runs);

    QList<DataModel*> dataModels;
    QFileInfo fi(runPath);
    if ( fi.isFile() ) {
        DataModel* dataModel = DataModel::createDataModel(timeNames,
                                                          runPath, runPath);
        dataModels.append(dataModel);
    } else if ( fi.isDir() ) {

        QDir dir(runPath);

        QStringList filter;
        filter << "*.trk" << "*.csv" << "*.mot";
        foreach(QString fileName, dir.entryList(filter, QDir::Files)) {
            if ( fileName == "_init_log.csv" ||
                 fileName == "log_timeline.csv" ||
                 fileName == "log_timeline_init.csv" ) {
                continue;
            }
            QString fullName = dir.absoluteFilePath(fileName);
            DataModel* dataModel = DataModel::createDataModel(timeNames,runPath,
                                                              fullName);
            dataModels.append(dataModel);
        }
        if ( dataModels.empty() ) {
            fprintf(stderr,"koviz [error]: no trk,csv,mot logfiles found in "
                           "runPath=%s\n", runPath.toLatin1().constData());
            exit(-1);
        }
    }

    // Get time unit, scale and bias
    QString dp_tunit("s"); // If vars doesn't have time, default to seconds
    double ts = 1.0;
    double tb = 0.0;
    foreach ( const DPVar var, dpvars ) {
        if ( timeNames.contains(var.name()) ) {
            if (!var.unit().isEmpty()) {
                dp_tunit = var.unit();
            }
            ts = var.scaleFactor();
            tb = var.bias();
            break;
        }
    }

    bool isValAtTime = false;
    bool isFirst = true;
    foreach (DPVar dpvar, dpvars) {
        foreach ( DataModel* dataModel, dataModels ) {
            dataModel->map();
            int tcol = dataModel->paramColumn(timeNames.at(0));
            int ycol = dataModel->paramColumn(dpvar.name());
            if ( tcol >= 0 && ycol >= 0 ) {

                const Parameter* tparam = dataModel->param(tcol);
                QString mo_tunit = tparam->unit();
                double tus = 1.0; // Time unit has no bias, just scale
                if ( Unit::canConvert(mo_tunit,dp_tunit) ) {
                    tus = Unit::scale(mo_tunit,dp_tunit);
                } else {
                    fprintf(stderr, "koviz [error]: -vars option has time "
                                    "variable with unit=%s but is logged with "
                                    "unit=%s.  Cannot convert!  Bailing!\n",
                            dp_tunit.toLatin1().constData(),
                            mo_tunit.toLatin1().constData());
                    return false;
                }

                const Parameter* yparam = dataModel->param(ycol);
                double us = 1.0;
                double ub = 0.0;
                double ds = dpvar.scaleFactor();
                double db = dpvar.bias();
                if ( !dpvar.unit().isEmpty() ) {
                    if ( Unit::canConvert(yparam->unit(),dpvar.unit()) ) {
                        us = Unit::scale(yparam->unit(),dpvar.unit());
                        ub = Unit::bias(yparam->unit(),dpvar.unit());
                    } else {
                        fprintf(stderr, "koviz [error]: -vars option has a "
                                "variable=%s with unit=%s but is logged with "
                                "unit=%s.  Cannot convert!  Bailing!\n",
                                dpvar.name().toLatin1().constData(),
                                dpvar.unit().toLatin1().constData(),
                                yparam->unit().toLatin1().constData());
                        exit(-1);
                    }
                }
                double mo_time = (time-tb)/ts/tus;
                int row = dataModel->indexAtTime(mo_time);
                ModelIterator* it = dataModel->begin(tcol,ycol,ycol);
                double tval = it->at(row)->t();
                tval = ts*(tus*tval)+tb;
                double yval = it->at(row)->y();
                yval = ds*(us*yval+ub)+db;
                if ( !isFirst ) {
                    printf(",");
                }
                if ( qAbs(time-tval) <= tmt ) {
                    printf("%.*g",DBL_DECIMAL_DIG,yval);
                    isValAtTime = true;
                } else {
                    // If this is time, print time, otherwise leave empty field
                    if ( dpvar.name() == timeNames.at(0) ) {
                        printf("%.*g",DBL_DECIMAL_DIG,time);
                    }
                }
                isFirst = false;
                dataModel->unmap();
                delete it;
                break;  // Found value so break from iterating over dataModels
            }
            dataModel->unmap();
        }
    }
    if ( !isFirst ) {
        printf("\n");
    }
    if ( !isValAtTime ) {
        fprintf(stderr, "koviz [warning]: no values at time=%g\n",time);
    }

    return true;
}

QList<DPVar> makeVarsList(const QString& varsOptString,Runs* runs)
{
    QList<DPVar> dpvars;
    QStringList vars = varsOptString.split(",", skipEmptyParts);
    bool isError = false;
    foreach (QString var, vars ) {
        QString v = var;
        if ( v.at(0) == '@' ) {
            v = var.mid(1);
        }

        // Get unit from var {unit} string
        QString dpunit;
        int i = v.lastIndexOf('{');
        int j = -1;
        if ( i != -1 ) {
            j = v.indexOf('}',i);
        }
        if ( i != -1 && j != -1 && i < j ) {
            dpunit = v.mid(i+1,j-i-1).trimmed();
            v = v.remove(i,j-i+1).trimmed();
        }
        if ( !dpunit.isEmpty() && !Unit::isUnit(dpunit) ) {
            fprintf(stderr, "koviz [error]: var=\"%s\" "
                            "from -vars option has bad unit\n",
                    var.toLatin1().constData());
            isError = true;
            break;
        }

        // Get scale from var {unit} scale(double) string
        double scale = 1.0;
        int i0 = v.indexOf(QRegularExpression("scale\\s*\\("));
        if ( i0 != -1 ) {
            i = v.indexOf('(',i0);
            j = v.indexOf(')',i+1);
            if ( i != -1 && j != -1 && i < j ) {
                QString s = v.mid(i+1, j-i-1).trimmed();
                bool ok = false;
                double val = s.toDouble(&ok);
                if ( ok ) {
                    scale = val;
                } else {
                    fprintf(stderr, "koviz [error]: Bad scale value=%s "
                                    "for var=%s\n",
                            s.toLatin1().constData(),
                            var.toLatin1().constData());
                    isError = true;
                    break;
                }
                v = v.remove(i0,j-i0+1).trimmed();
            }
        }

        // Get bias from var {unit} scale(double) bias(double) string
        double bias = 0.0;
        i0 = v.indexOf(QRegularExpression("bias\\s*\\("));
        if ( i0 != -1 ) {
            i = v.indexOf('(',i0);
            j = v.indexOf(')',i+1);
            if ( i != -1 && j != -1 && i < j ) {
                QString s = v.mid(i+1, j-i-1).trimmed();
                bool ok = false;
                double val = s.toDouble(&ok);
                if ( ok ) {
                    bias = val;
                } else {
                    fprintf(stderr, "koviz [error]: Bad bias value=%s "
                                    "for var=%s\n",
                            s.toLatin1().constData(),
                            var.toLatin1().constData());
                    isError = true;
                    break;
                }
                v = v.remove(i0,j-i0+1).trimmed();
            }
        }

        if ( !runs->params().contains(v) ) {
            fprintf(stderr, "koviz [error]: Cannot find var=\"%s\" "
                            "from -vars option.  Run(s) do not contain "
                            "this variable or syntax error with unit,"
                            "scale or bias specs.\n",
                    var.toLatin1().constData());
            isError = true;
            break;
        }

        DPVar dpvar(v.toLatin1().constData());
        dpvar.setUnit(dpunit.toLatin1().constData());
        dpvar.setScaleFactor(scale);
        dpvar.setBias(bias);
        dpvars.append(dpvar);
    }
    if ( isError ) {
        dpvars.clear();
        exit(-1);
    }

    return dpvars;
}

void preset_start(double* time, double new_time, bool* ok)
{
    *ok = true;

    if ( *ok ) {
        // Start time should be less than stop time
        if ( new_time > opts.stop ) {
            fprintf(stderr,"koviz [error] : Trying to set option -start to "
                    "%g; however stop time is %g.  Start should be less than "
                    "stop time.  Current start time is t=%g.\n",
                    new_time, opts.stop,*time);
            *ok = false;
        }
    }
}

void preset_stop(double* time, double new_time, bool* ok)
{
    *ok = true;

    if ( *ok ) {
        // Stop time should be greater than start time
        if ( new_time < opts.start ) {
            fprintf(stderr,"koviz [error] : Trying to set option -stop to "
                    "%g; however start time is %g.  Start should be less than "
                    "stop time.  -stop is currently t=%g.\n",
                    new_time, opts.start,*time);
            *ok = false;
        }
    }
}

void presetExistsFile(QString* ignoreMe, const QString& fname, bool* ok)
{
    Q_UNUSED(ignoreMe);

    QFileInfo fi(fname);
    if ( !fi.exists() ) {
        fprintf(stderr,
                "koviz [error] : Couldn't find file: \"%s\".\n",
                fname.toLatin1().constData());
        *ok = false;
        return;
    }
}

bool convert2csv(const QStringList& timeNames,
                 const QString& ftrk, const QString& fcsv,
                 double start, double stop, double shift, double tmt)
{
    TrickModel m(timeNames, ftrk, ftrk);

    QList<DPVar> vars;
    for ( int c = 0; c < m.columnCount(); ++c ) {
        const Parameter* param = m.param(c);
        DPVar var(param->name().toLatin1().constData());
        var.setUnit(param->unit().toLatin1().constData());
        vars.append(var);
    }

    bool r = writeCsv(fcsv,timeNames,vars,ftrk,start,stop,shift,tmt);

    return r;
}

bool convert2trk(const QString& csvFileName, const QString& trkFileName,
                 const QStringList& timeNames,
                 double start, double stop, double shift, double tmt)
{
    CsvModel csvModel(timeNames,csvFileName,csvFileName);

    QList<DPVar> vars;
    for ( int c = 0; c < csvModel.columnCount(); ++c ) {
        const Parameter* param = csvModel.param(c);
        DPVar var(param->name().toLatin1().constData());
        var.setUnit(param->unit().toLatin1().constData());
        vars.append(var);
    }

    bool r = writeTrk(trkFileName, timeNames, vars, csvFileName,
                      start,stop,shift,tmt);

    return r;
}

// shiftString has the form "[RUN_0:]val0[,RUN_1:val1,...]"
// This function returns a hash RUN_0->val0, RUN_1->val1...
QHash<QString,QVariant> getShiftHash(const QString& shiftString,
                                     const QStringList& runPaths)
{
    QHash<QString,QVariant> shifts;

    if (shiftString.isEmpty() || runPaths.isEmpty() ) return shifts; //empty map

    QStringList shiftStrings = shiftString.split(',',skipEmptyParts);
    foreach ( QString s, shiftStrings ) {

        s = s.trimmed();
        QString shiftRunFullPath;
        double shiftVal;
        if ( s.contains(':') ) {
            // e.g. koviz RUN_a RUN_b -shift "RUN_a:0.00125"
            QString shiftRun       = s.split(':').at(0).trimmed();
            QString shiftValString = s.split(':').at(1).trimmed();
            if ( shiftRun.isEmpty() || shiftValString.isEmpty() ) {
                fprintf(stderr,"koviz [error] : -shift option value \"%s\""
                               "is malformed.\n"
                               "Use this syntax -shift \"<run>:<val>\"\n",
                        opts.shiftString.toLatin1().constData());
                exit(-1);
            }

            bool isFound = false;
            QFileInfo fi(shiftRun);
            shiftRunFullPath = fi.absoluteFilePath();
            foreach ( QString runPath, runPaths ) {
                QFileInfo fir(runPath);
                QString runPathFullPath = fir.absoluteFilePath();
                if ( runPathFullPath == shiftRunFullPath ) {
                    isFound = true;
                    break;
                }
            }
            if ( !isFound ) {
                fprintf(stderr,"koviz [error] : -shift option \"%s\" "
                               "does not specify a valid run to shift.\n"
                               "Use this syntax -shift \"<run>:<val>\" "
                               "where <run> is one of the runs in the \n"
                               "commandline set of runs e.g. %s.\n",
                        s.toLatin1().constData(),
                        runPaths.at(0).toLatin1().constData());
                exit(-1);
            }

            QVariant q(shiftValString);
            bool ok;
            shiftVal = q.toDouble(&ok);
            if ( !ok ) {
                fprintf(stderr,"koviz [error] : option -shift \"%s\" "
                               "does not specify a valid shift value.\n"
                               "Use this syntax -shift \"[<run>:]<val>\"\n",
                        s.toLatin1().constData());
                exit(-1);
            }

        } else {
            // e.g. koviz RUN_a -shift 0.00125
            if ( runPaths.size() != 1 ) {
                fprintf(stderr,"koviz [error] : option -shift \"%s\" "
                               "does not specify a valid shift string.\n"
                               "Use the run:val syntax when there are "
                               "multiple runs.\n"
                               "Use this syntax -shift \"[<run>:]<val>\"\n",
                        s.toLatin1().constData());
                exit(-1);
            }
            QVariant q(s);
            bool ok;
            shiftVal = q.toDouble(&ok);
            if ( !ok ) {
                fprintf(stderr,"koviz [error] : option -shift \"%s\" "
                               "does not specify a valid shift value.\n"
                               "Use this syntax -shift \"[<run>:]<val>\"\n",
                        s.toLatin1().constData());
                exit(-1);
            }

            QFileInfo fi(runPaths.at(0));
            shiftRunFullPath = fi.absoluteFilePath();
        }

        shifts.insert(shiftRunFullPath,shiftVal);

    }

    return shifts;
}

// An example mapString is "px=trick.pos[0]=spots.posx,trick.pos[1]=spots.posy"
// In this example, getVarMap would return the following hash:
//                 px->[trick.pos[0],spots.posx]
//                 trick.pos[1]->[spots.posy]
QHash<QString,QStringList> getVarMap(const QString& mapString)
{
    QHash<QString,QStringList> varMap;

    if (mapString.isEmpty() ) return varMap; // empty map

    QStringList maps = mapString.split(',',skipEmptyParts);
    foreach ( QString s, maps ) {
        s = s.trimmed();
        if ( s.contains('=') ) {
            QStringList list = s.split('=');
            QString key = list.at(0).trimmed();
            if ( key.isEmpty() ) {
                fprintf(stderr,"koviz [error] : -map option value \"%s\""
                        "is malformed.\n"
                        "Use this syntax -map \"key=val1=val2...,key=val...\"\n",
                        mapString.toLatin1().constData());
                exit(-1);
            }
            QStringList vals;
            for (int i = 1; i < list.size(); ++i ) {
                QString val = list.at(i).trimmed();
                if ( val.isEmpty() ) {
                    fprintf(stderr,"koviz [error] : -map option value \"%s\""
                       "is malformed.\n"
                       "Use this syntax -map \"key=val1=val2...,key=val...\"\n",
                       mapString.toLatin1().constData());
                    exit(-1);
                }
                vals << val;
            }
            varMap.insert(key,vals);
        } else {
            // error
            fprintf(stderr,"koviz [error] : -map option value \"%s\""
                       "is malformed.\n"
                       "Use this syntax -map \"key=val1=val2...,key=val...\"\n",
                       mapString.toLatin1().constData());
            exit(-1);
        }

    }

    return varMap;
}

QHash<QString,QStringList> getVarMapFromFile(const QString& mapFileName)
{
    QHash<QString,QStringList> varMap;

    if (mapFileName.isEmpty() ) return varMap; // empty map

    QFile file(mapFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stderr, "koviz [error]: Cannot read map file \"%s\".\n",
                mapFileName.toLatin1().constData());
        fprintf(stderr, "Aborting!!!\n");
        exit(-1);
    }

    QTextStream in(&file);

    QString mapString;
    while (!in.atEnd()) {
        QString line = in.readLine();
        mapString += line;
    }

    file.close();

    varMap = getVarMap(mapString);

    return varMap;
}

QStringList getTimeNames(const QString& timeName)
{
    QStringList timeNames;
    QStringList names = timeName.split('=',skipEmptyParts);
    foreach ( QString s, names ) {
        timeNames << s.trimmed();
    }
    return timeNames;
}

// Returns invalid size if str isn't in the form widthxheight
QSize str2size(const QString& str)
{
    QStringList fields = str.toLower().split('x');
    if (fields.size() != 2) {
        return QSize(-1,-1);
    }

    bool okw = false;
    bool okh = false;

    int w = fields[0].trimmed().toUInt(&okw);
    int h = fields[1].trimmed().toUInt(&okh);

    if ( !okw || !okh ) {
        return QSize(-1,-1);
    }

    return QSize(w, h);
}
