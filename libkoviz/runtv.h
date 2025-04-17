#ifndef RUNTV_H
#define RUNTV_H

#include <QString>
#include <QStringList>
#include "run.h"

// For now, this class is a placeholder that returns empty

class RunTV : public Run
{
public:
    RunTV(const QString& host_port_path); // e.g. trick://127.0.0.1:4545
    ~RunTV() {}
    QStringList params() override;
    DataModel *dataModel(const QString& param) override;
private:
    QString _host_port_path;
    QStringList _params;
};

#endif // RUNTV_H
