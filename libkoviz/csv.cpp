#include "csv.h"
#include <QTextDecoder>

CSV::CSV(QIODevice * device)
{
	m_device = device;
	m_codec = QTextCodec::codecForLocale();
    m_pos = 0;
}

CSV::~CSV()
{
	//delete m_codec;
}

void CSV::setCodec(const char* codecName){
	//delete m_codec;
    m_codec = QTextCodec::codecForName(codecName);
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
