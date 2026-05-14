#include <QFileInfo>
#include "datamodel.h"
#include "datamodel_trick.h"
#include "datamodel_csv.h"
#include "datamodel_optitrack_csv.h"
#include "datamodel_mot.h"
#include "datamodel_hdf5.h"
#include "datamodel_acssl_xls.h"

QList<DataModel*> DataModel::createDataModels(const QStringList &timeNames,
                                              const QString &runPath,
                                              const QString &fileName)
{
    QList<DataModel*> models;

    QFileInfo fi(fileName);
    if ( fi.suffix() == "trk") {
        DataModel* m = new TrickModel(timeNames,runPath,fileName);
        models.append(m);
    } else if ( fi.suffix() == "csv" ) {
        if ( OptiTrackCsvModel::isValid(fileName) ) {
            DataModel* m = new OptiTrackCsvModel(timeNames,runPath,fileName);
            models.append(m);
        } else {
            DataModel* m = new CsvModel(timeNames,runPath,fileName);
            models.append(m);
        }
    } else if ( fi.suffix() == "mot" ) {
        DataModel* m = new MotModel(timeNames,runPath,fileName);
        models.append(m);
    } else if ( fi.suffix() == "h5" || fi.suffix() == "hdf5" ) {
        #ifdef HAS_HDF5
            models = Hdf5Model::dataModels(timeNames, runPath,fileName);
        #else
            fprintf(stderr,"koviz [error]: DataModel::createDataModels() cannot"
                    " handle file=\"%s\" because system does not have the "
                    "hdf5 develoment library or cannot find "
                    "/usr/include/H5Cpp.h\n",
                    fileName.toLatin1().constData());
            exit(-1);
        #endif
    } else if ( fi.suffix() == "xls" ) {
        DataModel* m = new AcsslXlsModel(timeNames,runPath,fileName);
        models.append(m);
    } else {
        fprintf(stderr,"koviz [error]: DataModel::createDataModel() cannot "
                       "handle file=\"%s\"\n",fileName.toLatin1().constData());
        exit(-1);
    }

    return models;
}
