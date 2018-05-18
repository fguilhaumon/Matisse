﻿#ifndef ReconstructionLister_H
#define ReconstructionLister_H


#include "RasterProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

using namespace MatisseCommon;

class ReconstructionLister : public RasterProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::RasterProvider)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ReconstructionLister")
#endif

public:
    explicit ReconstructionLister(QObject *parent = 0);
    virtual ~ReconstructionLister();

    virtual void onNewImage(quint32 port, Image &image);
    virtual void onFlush(quint32 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> _rastersInfo;

signals:
    
public slots:
    
};

#endif // ReconstructionLister_H
