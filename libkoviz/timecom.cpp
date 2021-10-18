#include "timecom.h"
TimeCom::TimeCom( const QString& host,int port,QObject* parent) :
    QObject(parent),
    _host(host),
    _port(port)
{
    socket = new QTcpSocket();
    connect(socket,SIGNAL(readyRead()),
            this,SLOT(_timeComRead()));
    _connect2Bvis();
}

TimeCom::~TimeCom()
{
    socket->close();
    delete socket;
}

int TimeCom::_connect2Bvis()
{
    socket->connectToHost(_host,_port);
    if (socket->waitForConnected(500)) {
        printf("Connected to host=%s port=%d!\n",
               _host.toLatin1().constData(), _port );
    } else {
        // timed out
        return 1;
    }
    return 0;
}

void TimeCom::_sendMsg2Bvis(const QString &msg)
{
    if (socket->state() != QTcpSocket::ConnectedState) {
        if(_connect2Bvis() != 0) {
            return;
        }
    }

    qint64 bytesWritten = socket->write(msg.toLocal8Bit().data());

    if ( bytesWritten < 0 ) {
        fprintf(stderr, "koviz [error]: bvis socket write failed!!!\n");
    }
}

void TimeCom::sendTime2Bvis(double liveTime)
{
#if QT_VERSION >= 0x050000
    Qt::ApplicationState state = qApp->applicationState();
    if ( state & Qt::ApplicationActive) {
        QString msg = QString("t=%1").arg(liveTime);
        _sendMsg2Bvis(msg);
    }
#else
    fprintf(stderr, "koviz [error]: Bvis needs qt5 or greater!\n");
    exit(-1);
#endif
}

void TimeCom::_timeComRead()
{
    QByteArray bytes = socket->readLine();

    // Read all packets, set bytes to last packet read
    while (1) {
        QByteArray line = socket->readLine();
        if ( line.size() <= 0 ) {
            break;
        } else {
            bytes = line;
        }
    }

    QString msg(bytes);
    QStringList fields = msg.split('=');
    if ( fields.at(0) == "TIME" ) {
        double time = fields.at(1).toDouble();
        emit timechangedByBvis(time);
    }
}

void TimeCom::sendRun2Bvis(const QString& iRunDir)
{
    QDir rdir(iRunDir);

    QString runDir = iRunDir;

    // If a run subdir contains *.motcsv, send run subdir
    QStringList filter;
    filter << "*.motcsv";
    QStringList dirs = rdir.entryList(QDir::Dirs);
    foreach ( QString dir, dirs ) {
        QString fdir = iRunDir + "/" + dir;
        QDir subdir(fdir);
        if ( ! subdir.entryList(filter,QDir::Files).isEmpty() ) {
            runDir = subdir.absolutePath();
            break;
        }
    }
    QString msg = QString("run=%1").arg(runDir);
#define VIS
#ifndef VIS
    _sendMsg2Bvis(msg);
#endif

#ifdef VIS
    // Reads single line from RUN_dir/rpxyz.csv
    // and sends that to blender
    QString r;
    QString p;
    QString x;
    QString y;
    QString z;
    filter.clear();
    filter << "*.csv";
    QStringList files = rdir.entryList(QDir::Files);
    QFile file(rdir.dirName() + "/" + files.at(0));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    int i = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if ( i == 1 ) {
            QString str(line);
            QStringList fields = str.split(',');
            r = fields.at(1).trimmed();
            p = fields.at(2).trimmed();
            x = fields.at(3).trimmed();
            y = fields.at(4).trimmed();
            z = fields.at(5).trimmed();
            break;
        }
        ++i;
    }
    file.close();

    msg = QString("%1,roll=%2,pitch=%3,x=%4,y=%5,z=%6").
                   arg(msg).arg(r).arg(p).arg(x).arg(y).arg(z);
    _sendMsg2Bvis(msg);
#endif
}

void TimeCom::sendList2Bvis(const QStringList &list)
{
    QString msg;
    foreach ( QString el, list ) {
        msg += el;
        msg += ',';
    }

    _sendMsg2Bvis(msg);
}
