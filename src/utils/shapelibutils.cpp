#include "shapelibutils.h"

#include <QFile>


bool readShapeFileData(const QString &filename, QVector<MapRegion> &regions, bool convertToPositive)
{
    // open shape file and get handle
    SHPHandle handle = SHPOpen(filename.toStdString().c_str(), "rb");
    if(!handle) {
        throw QString("File %1 not found").arg(filename);
    }

    int shapeType;
    int entities;
    double adfMinBound[4];
    double adfMaxBound[4];

    // get info about shapes in the file
    SHPGetInfo(handle, &entities, &shapeType, adfMinBound, adfMaxBound);

    // init the regions vector for faster adding
    regions.resize(entities);

    // iterate over every entry in shape file
    for(int i = 0; i < entities; ++i) {
        // get shape object
        SHPObject *shape = SHPReadObject(handle, i);

        // get unique shape id in file, corresponds to id of data table
        int id = shape->nShapeId;

        // get amount of parts in the shape
        regions[id].parts = shape->nParts;

        // the data is scaled by 100 for displaying reasons
        double factor = 100.0;

        // only polygon shapes will be used.
        if(shape->nSHPType == SHPT_POLYGON) {
            // if there are more parts, the data must be seperated into differnet poylgons
            if(regions[id].parts > 1) {
                // init start indices of new polygons
                QVector<int> startIndices(regions[id].parts + 1);
                for(int part = 0; part < regions[id].parts; ++part) {
                    startIndices[part] = shape->panPartStart[part];
                }
                startIndices[regions[id].parts] = shape->nVertices;

                // add a poylgon for every part
                for(int part = 0; part < regions[id].parts; ++part) {
                    QPolygonF polygon;
                    double x = 0.0;
                    for(int vertex = startIndices[part]; vertex < startIndices[part + 1]; ++vertex) {
                        x = (convertToPositive && shape->padfX[vertex] < 0)
                                ? shape->padfX[vertex] + 360.0
                                : shape->padfX[vertex];
                        polygon << QPointF(x * factor,
                                           shape->padfY[vertex] * factor);
                    }
                    // add the first point again to get a closed polygon
                    x = (convertToPositive && shape->padfX[startIndices[part]] < 0)
                            ? shape->padfX[startIndices[part]] + 360.0
                            : shape->padfX[startIndices[part]];
                    polygon << QPointF(x * factor,
                                       shape->padfY[startIndices[part]] * factor);

                    regions[id].polygons.push_back(polygon);
                }
            } else {
                // add single polygon if there is only 1 part
                QPolygonF polygon;
                double x = 0.0;
                for(int v = 0; v < shape->nVertices; ++v) {
                    x = (convertToPositive && shape->padfX[v] < 0)
                            ? shape->padfX[v] + 360.0
                            : shape->padfX[v];
                    polygon << QPointF(x * factor,
                                       shape->padfY[v] * factor);
                }
                regions[id].polygons.push_back(polygon);
            }
        }

        SHPDestroyObject(shape);
    }
    SHPClose(handle);


    return true;
}

bool readDatabaseFileGermany(const QString &filename, QVector<MapRegion> &regions)
{
    // open database file and get handle
    DBFHandle dbfHandle = DBFOpen(filename.toStdString().c_str(), "rb");
    if(!dbfHandle) {
        throw QString("File %1 not found").arg(filename);
    }

    // get field (column) and record (row) count
    int fields = DBFGetFieldCount(dbfHandle);
    assert(fields == 23); // checking that the file is correct
    int records = DBFGetRecordCount(dbfHandle);

    // get data from database
    for(int i = 0; i < records; ++i) {
        regions[i].name     = DBFReadStringAttribute(dbfHandle, i, 5);
        regions[i].agsStr   = DBFReadStringAttribute(dbfHandle, i, 3);
        regions[i].ags      = regions[i].agsStr.toInt();
        regions[i].ewz      = DBFReadIntegerAttribute(dbfHandle, i, 18);
        regions[i].bez      = DBFReadStringAttribute(dbfHandle, i, 6);
    }

    DBFClose(dbfHandle);

    return true;
}

bool readDatabaseFileUsa(const QString &filename, QVector<MapRegion> &regions)
{
    // open database file and get handle
    DBFHandle dbfHandle = DBFOpen(filename.toStdString().c_str(), "rb");
    if(!dbfHandle) {
        throw QString("File %1 not found").arg(filename);
    }

    // get field (column) and record (row) count
    int fields = DBFGetFieldCount(dbfHandle);
    int records = DBFGetRecordCount(dbfHandle);

    assert(fields == 9);

    // get data from database
    for(int i = 0; i < records; ++i) {
        regions[i].name = DBFReadStringAttribute(dbfHandle, i, 5);
        regions[i].stateFips = DBFReadStringAttribute(dbfHandle, i, 0);
        regions[i].countyFips = DBFReadStringAttribute(dbfHandle, i, 1);
    }

    DBFClose(dbfHandle);

    return true;
}

double getDirection(const QPolygonF &polygon)
{
    double sum = 0.0;
    for(int i = 0; i < polygon.size() - 1; ++i) {
        sum += (polygon[i + 1].x() - polygon[i].x()) * (polygon[i + 1].y() + polygon[i].y());
    }
    sum += (polygon[0].x() - polygon[polygon.size() - 1].x()) * (polygon[0].y() + polygon[polygon.size() - 1].y());

    return sum;
}
