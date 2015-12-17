﻿#include "MosaicContext.h"
#include "MosaicDrawer.h"
#include "NavImage.h"
#include "DrawBlend2DMosaic.h"
#include "GeoTransform.h"
#include "RasterGeoreferencer.h"
#include "MosaicDescriptor.h"

// Export de la classe DrawAndWriteModule dans la bibliotheque de plugin DrawAndWriteModule
Q_EXPORT_PLUGIN2(DrawBlend2DMosaic, DrawBlend2DMosaic)



DrawBlend2DMosaic::DrawBlend2DMosaic() :
    Processor(NULL, "DrawBlend2DMosaic", "DrawBlend2DMosaic", 1, 1)
{
    qDebug() << logPrefix() << "DrawBlend2DMosaic in constructor...";

    addExpectedParameter("dataset_param",  "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param",  "output_filename");

}

DrawBlend2DMosaic::~DrawBlend2DMosaic(){

}

bool DrawBlend2DMosaic::configure()
{
    return true;
}

void DrawBlend2DMosaic::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port);
    Q_UNUSED(image);
}

bool DrawBlend2DMosaic::start()
{
    return true;
}

bool DrawBlend2DMosaic::stop()
{
    return true;
}

void DrawBlend2DMosaic::onFlush(quint32 port)
{

    MosaicDescriptor *pMosaicD = NULL;
    //QVector<ProjectiveCamera*> *pCams = NULL;

    // Get pCams from mosaic _context
    /*QVariant * pCamsStocker = _context->getObject("Cameras");
    if (pCamsStocker) {
        pCams = pCamsStocker->value< QVector<ProjectiveCamera*>* >();
        qDebug()<< logPrefix() << "Receiving Cameras on port : " << port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << port;
    }*/

    // Get pMosaicD from mosaic _context
    QVariant * pMosaicDStocker = _context->getObject("MosaicDescriptor");
    if (pMosaicDStocker) {
        pMosaicD = pMosaicDStocker->value<MosaicDescriptor *>();
        qDebug()<< logPrefix() << "Receiving MosaicDescriptor on port : " << port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << port;
    }


    //Draw mosaic
    MosaicDrawer mosaicDrawer;
    cv::Mat mosaicImage,mosaicMask;
    mosaicDrawer.drawAndBlend(*pMosaicD, mosaicImage, mosaicMask);

    // Write geofile
    QString datasetDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString outputDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    if (outputDirnameStr.isEmpty()
            || datasetDirnameStr.isEmpty()
            || outputFilename.isEmpty())
        return;

    QFileInfo outputDirInfo(outputDirnameStr);
    QFileInfo datasetDirInfo(datasetDirnameStr);

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
    }

    qDebug() << "output_dir = " << outputDirnameStr;
    qDebug() << "output_filename = " << outputFilename;

    pMosaicD->writeToGeoTiff(mosaicImage,mosaicMask,outputDirnameStr + QDir::separator() + outputFilename);

}



