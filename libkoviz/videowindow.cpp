#include "videowindow.h"

#ifdef HAS_MPV
static void wakeup(void *ctx)
{
    VideoWindow *mainwindow = (VideoWindow *)ctx;
    mainwindow->wrap_mpv_events();
}
#endif

VideoWindow::VideoWindow(const QList<QPair<QString, double> > &videos,
                         QWidget *parent) :
    QMainWindow(parent),
    _startTime(0.0)
{
#ifndef HAS_MPV
    Q_UNUSED(videos)
#endif
#ifdef HAS_MPV
    std::setlocale(LC_NUMERIC, "C");
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Koviz MPV");
    setMinimumSize(640, 480);

    statusBar();

    connect(this, &VideoWindow::mpv_events,
            this, &VideoWindow::on_mpv_events,
            Qt::QueuedConnection);

    QPair<QString,double> videoIn;
    foreach (videoIn, videos) {
        Video* video = _create_video();
        video->fileName = videoIn.first;
        video->timeOffset = videoIn.second;
        if ( videos.size() > 1 ) {
            mpv_set_option_string(video->mpv,"pause","yes");
        }
        _videos.append(video);
    }

    _grid = new QGridLayout();
    int nVideos = static_cast<int>(_videos.size());
    int ncols = ceil(sqrt(_videos.size()));
    std::div_t q = std::div(nVideos,ncols);
    int nrows = 0;
    if ( q.rem == 0 ) {
        nrows = q.quot;
    } else {
        nrows = q.quot+1;
    }
    int k = 0;
    for ( int i = 0; i < nrows; ++i ) {
        for ( int j = 0; j < ncols; ++j ) {
            _grid->addWidget(_videos.at(k++)->videoWidget,i,j);
            _grid->setRowStretch(i, 1);
            _grid->setColumnStretch(j, 1);
            if ( k == _videos.size() ) {
                break;
            }
        }
    }

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(_grid);
    setCentralWidget(centralWidget);

#endif
}

void VideoWindow::wrap_mpv_events()
{
    emit mpv_events();
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    QSettings settings("JSC", "koviz");

    settings.beginGroup("VideoWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

#ifdef HAS_MPV
    // Shutdown mpv videos
    foreach (Video* video, _videos) {
        if (video->mpv) {
            const char *args[] = {"quit", NULL};
            mpv_command(video->mpv, args);
            mpv_event *event;
            int i = 0;
            while ( true ) {
                event = mpv_wait_event(video->mpv, 0);
                if (event->event_id == MPV_EVENT_SHUTDOWN) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                if ( i++ > 10 ) {
                    fprintf(stderr,"koviz [error]: Trouble terminating mpv\n");
                    break;
                }
            }
            mpv_terminate_destroy(video->mpv);
            video->mpv = nullptr;
        }
    }
#endif

    emit closeVidView();
    QMainWindow::closeEvent(event);
}

bool VideoWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::MouseButtonRelease) {

        bool isSingleView = false;
        foreach ( Video* video, _videos ) {
            if ( video->videoWidget->isHidden() ) {
                isSingleView = true;
                break;
            }
        }

        if ( isSingleView ) {
            // Toggle to view of all videos at once (multi view)

            // Clear layout
            QLayoutItem *child;
            while ((child = _grid->takeAt(0)) != nullptr) {
                _grid->removeWidget(child->widget());
                delete child;   // delete the layout item
            }
            for (int r = 0; r < _grid->rowCount(); ++r ) {
                _grid->setRowStretch(r,0);
            }
            for (int c = 0; c < _grid->columnCount(); ++c ) {
                _grid->setColumnStretch(c,0);
            }
            _grid->update();

            // Toggle to grid of videos
            int nVideos = static_cast<int>(_videos.size());
            int ncols = ceil(sqrt(_videos.size()));
            std::div_t q = std::div(nVideos,ncols);
            int nrows = 0;
            if ( q.rem == 0 ) {
                nrows = q.quot;
            } else {
                nrows = q.quot+1;
            }
            int k = 0;
            for ( int i = 0; i < nrows; ++i ) {
                for ( int j = 0; j < ncols; ++j ) {
                    _videos.at(k)->videoWidget->show();
                    _grid->addWidget(_videos.at(k++)->videoWidget,i,j);
                    _grid->setRowStretch(i, 1);
                    _grid->setColumnStretch(j, 1);
                    if ( k == _videos.size() ) {
                        break;
                    }
                }
            }
        } else {
            // Toggle to single expanded view of clicked video

            QWidget* clickedVideo = 0;
            foreach ( Video* video, _videos ) {
                if ( video->videoWidget->whiteBox == obj ) {
                    clickedVideo = video->videoWidget;
                }
            }

            // Clear layout (and hide all non-clicked widgets)
            QLayoutItem *child;
            while ((child = _grid->takeAt(0)) != nullptr) {
                if ( child->widget() != clickedVideo ) {
                    child->widget()->hide();
                    _grid->removeWidget(child->widget());
                }
                delete child;   // delete the layout item
            }
            for (int r = 0; r < _grid->rowCount(); ++r ) {
                _grid->setRowStretch(r,0);
            }
            for (int c = 0; c < _grid->columnCount(); ++c ) {
                _grid->setColumnStretch(c,0);
            }
            _grid->update();

            foreach ( Video* video, _videos ) {
                if ( obj == video->videoWidget->whiteBox ) {
                    _grid->addWidget(video->videoWidget,0,0);
                    _grid->setRowStretch(0, 1);
                    _grid->setColumnStretch(0, 1);
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj,event);
}

void VideoWindow::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Space ) {
        foreach (Video* video, _videos) {
            if (video->mpv) {
                char* prop = mpv_get_property_string(video->mpv,"pause");
                if ( prop ) {
                    if ( !strcmp(prop,"no") ) {
                        mpv_set_option_string(video->mpv,"pause","yes");
                    } else {
                        mpv_set_option_string(video->mpv,"pause","no");
                    }
                    mpv_free(prop);
                }
            }
        }
    } else {
       QWidget::keyPressEvent(event);
    }
}

void VideoWindow::seek_time(double time) {
#ifndef HAS_MPV
    Q_UNUSED(time)
#endif
#ifdef HAS_MPV
    int isIdle;
    int i = 0;
    foreach (Video* video, _videos) {
        int ret = mpv_get_property(video->mpv,"core-idle",
                                   MPV_FORMAT_FLAG,&isIdle);
        if ( ret >= 0 ) {
            if ( isIdle && parentWidget()->isActiveWindow() ) {
                // Koviz is driving time
                double timeOffset = video->timeOffset;
                QString com = QString("seek %1 absolute").arg(time+timeOffset);
                mpv_command_string(video->mpv, com.toLocal8Bit().data());
            }
        }
        ++i;
    }
#endif
}

#ifdef HAS_MPV
void VideoWindow::handle_mpv_event(Video* video, mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data - video->timeOffset;
                std::stringstream ss;
                ss << "At: " << time;
                statusBar()->showMessage(QString::fromStdString(ss.str()));
                int isIdle;
                int ret = mpv_get_property(video->mpv,"core-idle",
                                           MPV_FORMAT_FLAG, &isIdle);
                if ( ret >= 0 ) {
                    if ( !isIdle || isActiveWindow() ) {
                        // If mpv playing, update time
                        // If mpv paused but still active, update time
                        // If mpv paused and koviz active, don't emit signal
                        emit timechangedByMpv(time);
                    }
                }
            } else if (prop->format == MPV_FORMAT_NONE) {
                statusBar()->showMessage("");
            }
        }
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(video->mpv);
        video->mpv = NULL;
        break;
    }
    default: ;
        // Ignore event
    }
}
#endif

// This slot is invoked by wakeup() (through the mpv_events signal).
void VideoWindow::on_mpv_events()
{
#ifdef HAS_MPV
    // Process all events, until the event queue is empty.
    foreach (Video* video, _videos) {
        while (video->mpv) {
            mpv_event *event = mpv_wait_event(video->mpv, 0);
            if (event->event_id == MPV_EVENT_NONE) {
                break;
            }
            handle_mpv_event(video,event);
        }
    }
#endif
}

// Input is a list of "filename,timeoffset" pairs
void VideoWindow::set_videos(const QList<QPair<QString,double> >& videos)
{
#ifndef HAS_MPV
    Q_UNUSED(videos)
#endif
#ifdef HAS_MPV
    if ( videos.size() != _videos.size() ) {
        _resize_videos(videos);
    }

    QPair<QString,double> videoIn;
    int i = 0 ;
    foreach ( videoIn, videos ) {
        Video* video = _videos.at(i);
        video->fileName = videoIn.first;
        video->timeOffset = videoIn.second;
        ++i;
    }

    foreach (Video* video, _videos ) {
        if (video->mpv) {
            if ( _startTime + video->timeOffset > 0 ) {
                QString offset = QString("%1").arg(_startTime +
                                                   video->timeOffset);
                mpv_set_option_string(video->mpv,"start",
                                      offset.toLatin1().constData());
            }
            QString fname = video->fileName;
            const QByteArray c_filename = fname.toUtf8();
            const char *args[] = {"loadfile", c_filename.data(), NULL};
            mpv_command(video->mpv, args);

            // Without sleeping mpv will sometimes miss loading videos
            struct timespec req = {0, 500000000};
            nanosleep(&req, NULL);
        } else {
            fprintf(stderr, "koviz [bad scoobs]: VideoWindow::set_videos()\n");
            exit(-1);
        }
        ++i;
    }
#endif
}

void VideoWindow::set_start(double startTime)
{
    _startTime = startTime;
}

void VideoWindow::pause()
{
#ifdef HAS_MPV
    foreach (Video* video, _videos) {
        if (video->mpv) {
            mpv_set_option_string(video->mpv,"pause","yes");
        }
    }
#endif
}

void VideoWindow::_resize_videos(const QList<QPair<QString, double> > &videos)
{
#ifndef HAS_MPV
    Q_UNUSED(videos)
#endif
#ifdef HAS_MPV
    if ( videos.size() > _videos.size() ) { // Add videos
        int d = videos.size() - _videos.size();
        for (int i = 0; i < d; ++i ) {
            Video* video = _create_video();
            if ( videos.size() > 1 ) {
                // Bring up paused if more than a single video
                // since they cannot be perfectly synced
                mpv_set_option_string(video->mpv,"pause","yes");
            }
            _videos.append(video);
        }

        // Clear layout (but keep widgets)
        QLayoutItem *child;
        while ((child = _grid->takeAt(0)) != nullptr) {
            child->widget()->hide();
            delete child;   // delete the layout item
        }
        for (int r = 0; r < _grid->rowCount(); ++r ) {
            _grid->setRowStretch(r,0);
        }
        for (int c = 0; c < _grid->columnCount(); ++c ) {
            _grid->setColumnStretch(c,0);
        }
        _grid->update();

        // Redo layout with new number of videos
        if ( _videos.size() > 0 ) {
            int nVideos = static_cast<int>(_videos.size());
            int ncols = ceil(sqrt(_videos.size()));
            std::div_t q = std::div(nVideos,ncols);
            int nrows = 0;
            if ( q.rem == 0 ) {
                nrows = q.quot;
            } else {
                nrows = q.quot+1;
            }
            int k = 0;
            for ( int i = 0; i < nrows; ++i ) {
                for ( int j = 0; j < ncols; ++j ) {
                    _videos.at(k)->videoWidget->show();
                    _grid->addWidget(_videos.at(k++)->videoWidget,i,j);
                    _grid->setRowStretch(i, 1);
                    _grid->setColumnStretch(j, 1);
                    if ( k == _videos.size() ) {
                        break;
                    }
                }
            }
        }
    } else if ( videos.size() < _videos.size() ) {

        // Remove videos
        int d = _videos.size() - videos.size();
        for ( int i = 0; i < d; ++i ) {
            Video* video = _videos.takeLast();
            if ( video->mpv ) {
                mpv_set_option_string(video->mpv,"pause","yes");
                mpv_set_property_string(video->mpv,"eof-reached","yes");
                mpv_terminate_destroy(video->mpv);
            }
            delete video->videoWidget;
        }

        // Clear layout (but keep widgets)
        QLayoutItem *child;
        while ((child = _grid->takeAt(0)) != nullptr) {
            child->widget()->hide();
            _grid->removeWidget(child->widget());
            delete child;   // delete the layout item
        }
        for (int r = 0; r < _grid->rowCount(); ++r ) {
            _grid->setRowStretch(r,0);
        }
        for (int c = 0; c < _grid->columnCount(); ++c ) {
            _grid->setColumnStretch(c,0);
        }
        _grid->update();

        // Redo layout with new number of videos
        if ( _videos.size() > 0 ) {
            int nVideos = static_cast<int>(_videos.size());
            int ncols = ceil(sqrt(_videos.size()));
            std::div_t q = std::div(nVideos,ncols);
            int nrows = 0;
            if ( q.rem == 0 ) {
                nrows = q.quot;
            } else {
                nrows = q.quot+1;
            }
            int k = 0;
            for ( int i = 0; i < nrows; ++i ) {
                for ( int j = 0; j < ncols; ++j ) {
                    _videos.at(k)->videoWidget->show();
                    _grid->addWidget(_videos.at(k++)->videoWidget,i,j);
                    _grid->setRowStretch(i, 1);
                    _grid->setColumnStretch(j, 1);
                    if ( k == _videos.size() ) {
                        break;
                    }
                }
            }
        }
    } else {
        // Nothing to do
        return;
    }
#endif
}

Video* VideoWindow::_create_video()
{
#ifdef HAS_MPV
    mpv_handle* mpv = mpv_create();
    if (!mpv) {
        throw std::runtime_error("can't create mpv instance");
    }

    VideoWidget* videoWidget = new VideoWidget(this);

    videoWidget->hbox = new QHBoxLayout(videoWidget);
    videoWidget->hbox->setContentsMargins(0,0,0,0);
    videoWidget->hbox->setSpacing(0);
    videoWidget->helperWidget = new QWidget(videoWidget);
    videoWidget->helperWidget->setStyleSheet("background-color: black;");
    videoWidget->helperWidget->setFixedWidth(20);
    ClickFrame* whiteBox = new ClickFrame(videoWidget->helperWidget);
    whiteBox->setFrameShape(QFrame::Box);
    whiteBox->setStyleSheet("background-color: none;"
                            "border: 1px solid #ededed;");
    whiteBox->setFixedSize(12,12);
    whiteBox->move(4,4);
    whiteBox->installEventFilter(this);
    videoWidget->whiteBox = whiteBox;

    QWidget* mpv_container = new QWidget(videoWidget);
    videoWidget->hbox->addWidget(videoWidget->helperWidget);
    videoWidget->hbox->addWidget(mpv_container);

    mpv_container->setAttribute(Qt::WA_DontCreateNativeAncestors);
    mpv_container->setAttribute(Qt::WA_NativeWindow);

#ifndef _WIN32
    mpv_set_option_string(mpv, "input-default-bindings", "yes");
    mpv_set_option_string(mpv, "input-vo-keyboard", "yes");
#endif
    mpv_set_option_string(mpv, "keep-open", "always");
    mpv_set_option_string(mpv, "osd-level", "0");

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_set_wakeup_callback(mpv, wakeup, this);

    int64_t wid = mpv_container->winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    if (mpv_initialize(mpv) < 0) {
        throw std::runtime_error("mpv failed to initialize");
    }

    Video* video = new Video();
    video->fileName = "";     // This will be set in set_videos()
    video->timeOffset = 0.0;  // Ditto
    video->mpv = mpv;
    video->videoWidget = videoWidget;
    video->videoWidget->mpvContainer = mpv_container;

    return video;
#else
    return 0; // Nothing to do with no mpv
#endif
}

VideoWindow::~VideoWindow()
{
#ifdef HAS_MPV
    foreach (Video* video, _videos) {
        if (video->mpv) {
            mpv_event *event;
            while ((event = mpv_wait_event(video->mpv, 0))) {
                if (event->event_id == MPV_EVENT_SHUTDOWN) {
                    break;
                }
            }
            mpv_terminate_destroy(video->mpv);
        }
        delete video;
    }
    _videos.clear();
#endif
}
