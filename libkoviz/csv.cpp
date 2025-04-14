#include "csv.h"
#include <QTextDecoder>

CSV::CSV(QIODevice * device)
{
	m_device = device;
}

CSV::~CSV()
{
}

QStringList CSV::parseLine(){
    QStringList list;
    QString line(m_device->readLine(1024*1024));
    if ( line.isEmpty() ) {
        return list; // empty list - file end, hopefully!!!
    }
    line.chop(1); // chops off \n
    list = line.split(',');
    return list;
}
