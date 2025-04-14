/* This was intitially taken off
 * the internet but the class
 * has been whittled down
 * to a wrapper around splitting
 * a QString!
 */
#ifndef CSV_H
#define CSV_H

#include <QObject>
#include <QStringList>
#include <QIODevice>

class  CSV
{
public:
	CSV(QIODevice * device);
	~CSV();
	QStringList parseLine();

private:
	QIODevice *m_device;
};

#endif // CSV_H
