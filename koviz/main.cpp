#include <QApplication>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QString>
#include <QDate>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>

#include <string>
using namespace std;

#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <stdio.h>
#include <float.h>

#include "libkoviz/options.h"
#include "libkoviz/runs.h"
#include "libkoviz/plotmainwindow.h"
#include "libkoviz/roundoff.h"
#include "libkoviz/timestamps.h"
#include "libkoviz/tricktablemodel.h"
#include "libkoviz/dp.h"
#include "libkoviz/snap.h"
#include "libkoviz/csv.h"
#include "libkoviz/datamodel_trick.h"
#include "libkoviz/curvemodel.h"
#include "libkoviz/trick_types.h"
#include "libkoviz/session.h"
#include "libkoviz/versionnumber.h"
#include "libkoviz/mapvalue.h"

QStandardItemModel* createVarsModel(Runs* runs);
bool writeTrk(const QString& ftrk, const QString &timeName,
              double start, double stop, double timeShift,
              QStringList& paramList, Runs* runs);
bool writeCsv(const QString& fcsv, const QStringList& timeNames,
              DPTable* dpTable, const QString &runPath,
              double startTime, double stopTime, double tolerance);
bool convert2csv(const QStringList& timeNames,
                 const QString& ftrk, const QString& fcsv);
bool convert2trk(const QString& csvFileName, const QString &trkFileName);
QHash<QString,QVariant> getShiftHash(const QString& shiftString,
                                const QStringList &runPaths);
QHash<QString,QStringList> getVarMap(const QString& mapString);
QHash<QString,QStringList> getVarMapFromFile(const QString& mapFileName);
QStringList getTimeNames(const QString& timeName);

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
    QString dp2trkOutFile;
    QString csvOutFile;
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
    opts.add("-dp2trk", &opts.dp2trkOutFile, QString(""),
             "Create trk from DP_ vars, "
             "e.g. koviz DP_foo RUN_a -dp2trk foo.trk");
    opts.add("-dp2csv", &opts.csvOutFile, QString(""),
             "Create csv from DP_ vars, "
             "e.g. koviz DP_foo RUN_a -dp2csv foo.csv");
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
        bool ret;
        try {
            ret = convert2csv(timeNames,opts.trk2csvFile, csvOutFile);
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
        bool ret = convert2trk(opts.csv2trkFile, trkOutFile);
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

        bool isTrk = false;
        if ( !opts.dp2trkOutFile.isEmpty() ) {
            isTrk = true;
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

        bool isCsv = false;
        if ( !opts.csvOutFile.isEmpty() ) {
            isCsv = true;
        }

        if ( (isPdf && isTrk) || (isPdf && isCsv) || (isTrk && isCsv) ) {
            fprintf(stderr,
                    "koviz [error] : you may not use the -pdf, -trk and -csv "
                    "options together.");
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

        // If outputting to trk, you must have a DP file and RUN
        if ( isTrk && (dps.size() == 0 || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -trk option you must "
                    "specify a DP product file and RUN directory\n");
            exit(-1);
        }

        // If outputting to csv, you must have a DP file and RUN
        if ( isCsv && (dps.size() == 0 || runPaths.size() == 0) ) {
            fprintf(stderr,
                    "koviz [error] : when using the -csv option you must "
                    "specify a DP product file and RUN directory\n");
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
        if ( isPdf ) {
            isShowProgress = false;
        }

        // Time match tolerance
        double tolerance = opts.timeMatchTolerance;
        if ( tolerance == DBL_MAX ) {
            if ( session ) {
                tolerance = session->timeMatchTolerance();
            } else {
                tolerance = 0.0000001;  // 10th of a microsecond
            }
        }

        runs = new Runs(timeNames,tolerance,runPaths,varMap,
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
        bookModel->addChild(rootItem, "TimeMatchTolerance", tolerance);
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

        if ( isTrk ) {

            QStringList params = DPProduct::tableParamList(dps,timeName);
            if ( params.isEmpty() ) {
                params = DPProduct::paramList(dps,timeName);
            }

            if ( runs->runPaths().count() == 1 ) {
                QHash<QString,QVariant> shifts = getShiftHash(shiftString,
                                                              runPaths);
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
                bool r = writeTrk(opts.dp2trkOutFile,
                                  timeName,
                                  startTime,
                                  stopTime,
                                  timeShift,
                                  params,runs);
                if ( r ) {
                    ret = 0;
                } else {
                    fprintf(stderr, "koviz [error]: Failed to write: %s\n",
                            opts.dp2trkOutFile.toLatin1().constData());
                    ret = -1;
                }
            } else {
                fprintf(stderr, "koviz [error]: Only one RUN allowed with "
                                 "the -trk and -dp2trk options.\n");
                exit(-1);
            }

        } else if ( isCsv ) {


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
                    QString fname = opts.csvOutFile;
                    if ( dp.tables().size() > 1 ) {
                        // Multiple files to output, so index the name
                        QString dpName = QFileInfo(dpFileName).baseName();
                        QFileInfo fi(fname);
                        QString extension("csv");
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

                    bool r = writeCsv(fname,timeNames,dpTable,runPaths.at(0),
                                      startTime, stopTime, tolerance);
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

bool writeTrk(const QString& ftrk, const QString& timeName,
              double start, double stop, double timeShift,
              QStringList& paramList, Runs* runs)
{
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
    foreach ( QString param, paramList ) {
        fprintf(stderr, "    %s\n", param.toLatin1().constData());
    }
    fprintf(stderr, "\n");

    //
    // Make param list
    // And make curves list (based on param list)
    //
    QList<TrickParameter> params;
    QList<CurveModel*> curves;

    // Time is first "param"
    TrickParameter timeParam;
    timeParam.setName(timeName);
    timeParam.setUnit("s");
    timeParam.setType(TRICK_10_DOUBLE);
    timeParam.setSize(sizeof(double));
    params << timeParam;

    // Each param gets a curve. Make the first curve null since
    // there is no actual curve to go with timeStamps
    curves << 0;

    foreach ( QString yParam, paramList ) {

        // If time is in param list, then skip it  since timestamps
        // are generated and inserted into the first column of the trk
        if ( yParam == timeName ) {
            continue;
        }

        CurveModel* c = runs->curveModel(0,timeName,timeName,yParam);

        // Error check: see if MonteModel could not find curve (timeName,yParam)
        if ( !c) {
            fprintf(stderr, "koviz [error]: could not find curve: \n    ("
                    "%s,%s)\n",
                    timeName.toLatin1().constData(),
                    yParam.toLatin1().constData());
            foreach ( CurveModel* curveModel, curves ) {
                delete curveModel;
            }
            delete c;
            return false;
        }

        // Map Curve
        c->map();

        // Error check:   make sure curve has data
        if ( c->rowCount() == 0 ) {
            // No data
            fprintf(stderr, "koviz [error]: no data found in %s\n",
                    c->fileName().toLatin1().constData());
            foreach ( CurveModel* curveModel, curves ) {
                delete curveModel;
            }
            delete c;
            return false;
        }

        // Make a Param (for trk header)
        TrickParameter p;
        p.setName(yParam);
        p.setUnit(c->y()->unit());
        p.setType(TRICK_10_DOUBLE);
        p.setSize(sizeof(double));

        // Make params/curves lists (lazily mapping params to curves)
        params.append(p);
        curves.append(c);

        // Unmap Curve
        c->unmap();
    }
    if ( params.size() < 2 ) {
        fprintf(stderr,"koviz [error]: Could not find any params in RUN that "
                       "are in DP files\n\n");
        return false;
    }

    // Make time stamps list
    QList<double> timeStamps;
    foreach ( CurveModel* curve, curves ) {

        if ( !curve ) continue ;

        curve->map();

        ModelIterator* it = curve->begin();
        while ( !it->isDone() ) {
            double t = it->t();
            if ( t < start ) {
                it->next();
                continue;
            }
            if ( t > stop ) {
                break;
            }
            TimeStamps::insert(t,timeStamps);
            it->next();
        }
        delete it;
        curve->unmap();
    }

    // Open trk file for writing
    QFile trk(ftrk);
    if (!trk.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz: [error] could not open %s\n",
                ftrk.toLatin1().constData());
        return false;
    }
    QDataStream out(&trk);

    // Write Trk Header
    TrickModel::writeTrkHeader(out,params);
    trk.flush();

    // Resize Trk file to fit all the data + header
    qint64 headerSize = trk.size();
    qint64 nParams = params.size();
    qint64 recordSize = nParams*sizeof(double);
    qint64 nRecords = timeStamps.count();
    qint64 dataSize = nRecords*recordSize;
    qint64 fileSize = headerSize + dataSize;
    trk.resize(fileSize);

    int nTimeStamps = timeStamps.count();

    // Write time stamps and data
    int i = 0;
    foreach ( CurveModel* curve, curves ) {

        if ( !curve ) {
            // write time stamps
            for ( int j = 0 ; j < nTimeStamps; ++j ) {
                qint64 recordOffset = j*recordSize;
                qint64 paramOffset = 0;
                qint64 offset = headerSize + recordOffset + paramOffset;
                trk.seek(offset);
                out << timeStamps.at(j)+timeShift;
            }
        } else {
            // write curve data
            curve->map();
            ModelIterator* it = curve->begin();
            for ( int j = 0 ; j < nTimeStamps; ++j ) {

                double timeStamp = timeStamps.at(j);
                int k = curve->indexAtTime(timeStamp);
                double v = it->at(k)->y();

                qint64 recordOffset = j*recordSize;
                qint64 paramOffset = i*sizeof(double);
                qint64 offset = headerSize + recordOffset + paramOffset;
                trk.seek(offset);
                out << v;
            }
            delete it;
            curve->unmap();
        }
        ++i;
    }

    //
    // Clean up
    //
    trk.close();
    foreach ( CurveModel* curveModel, curves ) {
        delete curveModel;
    }

    return true;
}

bool writeCsv(const QString& fcsv, const QStringList& timeNames,
              DPTable* dpTable, const QString& runPath,
              double startTime, double stopTime, double tolerance)
{
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
    out.setFieldAlignment(QTextStream::AlignRight);
    out.setFieldWidth(16);
    out.setPadChar(' ');
    out.setRealNumberPrecision(15);

    // Csv header
    QString header;
    header = timeNames.at(0) + ",";
    foreach ( DPVar* var, dpTable->vars() ) {
        if ( timeNames.contains(var->name()) ) {
            // Skip time since auto included
            continue;
        }
        QString unit("");
        //unit = " {--}"; // TODO: Unit name and unit conversion
        header += var->name() +  unit + ",";
    }
    header.chop(1);
    out << header;
    out << "\n";

    // Csv body
    QStringList params;
    foreach ( DPVar* var, dpTable->vars() ) {
        if ( timeNames.contains(var->name()) ) {
            // Skip time since auto included
            continue;
        }
        params << var->name() ;
    }

    TrickTableModel ttm(timeNames, runPath, params);
    int rc = ttm.rowCount();
    int cc = ttm.columnCount();
    double epsilon = tolerance/2.0;
    for ( int r = 0 ; r < rc; ++r ) {
        bool isWriteRecord = true;
        for ( int c = 0 ; c < cc; ++c ) {
            QModelIndex idx = ttm.index(r,c);
            double v = ttm.data(idx).toDouble();
            if ( c == 0 ) {
                if ( v < startTime-epsilon || v > stopTime+epsilon ) {
                    isWriteRecord = false;
                    break;
                }
            }
            out << v;
            if ( c < cc-1 ) {
                int fw = out.fieldWidth();
                out.setFieldWidth(0);
                out << ",";
                out.setFieldWidth(fw);
            }
        }
        if ( isWriteRecord && r < rc-1 ) {
            out << "\n";
        }
    }

    // Clean up
    csv.close();

    return true;
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
                 const QString& ftrk, const QString& fcsv)
{
    TrickModel m(timeNames, ftrk, ftrk);

    QFileInfo fcsvi(fcsv);
    if ( fcsvi.exists() ) {
        fprintf(stderr, "koviz [error]: Will not overwrite %s\n",
                fcsv.toLatin1().constData());
        return false;
    }

    // Open csv file stream
    QFile csv(fcsv);
    if (!csv.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz: [error] could not open %s\n",
                fcsv.toLatin1().constData());
        return false;
    }
    QTextStream out(&csv);

    // Write csv param list (top line in csv file)
    int cc = m.columnCount();
    for ( int i = 0; i < cc; ++i) {
        QString pName = m.param(i)->name();
        QString pUnit = m.param(i)->unit();
        out << pName << " {" << pUnit << "}";
        if ( i < cc-1 ) {
            out << ",";
        }
    }
    out << "\n";

    //
    // Write param values
    //
    QLocale cLocale(QLocale::C);
    int rc = m.rowCount();
    for ( int r = 0 ; r < rc; ++r ) {
        for ( int c = 0 ; c < cc; ++c ) {
            QModelIndex idx = m.index(r,c);
            out << cLocale.toString(m.data(idx).toDouble());
            if ( c < cc-1 ) {
                out << ",";
            } else {
                out << "\n";
            }
        }
    }

    // Clean up
    csv.close();

    return true;
}

bool convert2trk(const QString& csvFileName, const QString& trkFileName)
{
    QFile file(csvFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stderr, "koviz [error]: Cannot read file %s!",
                csvFileName.toLatin1().constData());
    }
    CSV csv(&file);

    // Parse first line to get param list
    QList<TrickParameter> params;
    QStringList list = csv.parseLine() ;
    if ( list.isEmpty() ) {
        fprintf(stderr, "koviz [error]: Empty csv file \"%s\"",
                csvFileName.toLatin1().constData());
        return false;
    }
    foreach ( QString s, list ) {
        TrickParameter p;
        QStringList plist = s.split(" ", skipEmptyParts);
        p.setName(plist.at(0));
        if ( plist.size() > 1 ) {
            QString unitString = plist.at(1);
            if ( unitString.startsWith('{') ) {
                unitString = unitString.remove(0,1);
            }
            if ( unitString.endsWith('}') ) {
                unitString.chop(1);
            }
            Unit u;
            if ( u.isUnit(unitString.toLatin1().constData()) ) {
                p.setUnit(unitString);
            }
        }
        p.setType(TRICK_10_DOUBLE);
        p.setSize(sizeof(double));
        params.append(p);
    }

    QFileInfo ftrki(trkFileName);
    if ( ftrki.exists() ) {
        fprintf(stderr, "koviz [error]: Will not overwrite %s\n",
                trkFileName.toLatin1().constData());
        return false;
    }

    QFile trk(trkFileName);

    if (!trk.open(QIODevice::WriteOnly)) {
        fprintf(stderr,"koviz [error]: could not open %s\n",
                trkFileName.toLatin1().constData());
        return false;
    }
    QDataStream out(&trk);

    // Write trk header
    TrickModel::writeTrkHeader(out,params);

    //
    // Write param values
    //
    int line = 1;
    while ( 1 ) {
        ++line;
        QStringList list = csv.parseLine() ;
        if ( list.isEmpty() ) break;  // end of file, hopefully!!!
        foreach ( QString s, list ) {
            bool ok;
            double val = s.toDouble(&ok);
            if ( !ok ) {
                QStringList vals = s.split(":");
                if (vals.length() == 3 ) {
                    // Try converting to a utc timestamp
                    val = 3600.0*vals.at(0).toDouble(&ok);
                    if ( ok ) {
                        val += 60.0*vals.at(1).toDouble(&ok);
                        if ( ok ) {
                            val += vals.at(2).toDouble(&ok);
                        }
                    }
                }
            }
            if ( !ok ) {
                // If a single char, convert to unicode numeric value
                if ( s.size() == 1 ) {
                    val = s.at(0).unicode();
                    ok = true;
                }
            }
            if ( !ok ) {
                QFileInfo fi(csvFileName);
                fprintf(stderr,
                 "koviz [error]: Bad value \"%s\" on line %d in file %s\n",
                        s.toLatin1().constData(),
                        line,
                        fi.absoluteFilePath().toLatin1().constData());
                file.close();
                trk.remove();
                return false;
            }
            out << val;
        }
    }

    file.close();

    return true;
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
