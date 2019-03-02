﻿#include "Init3DRecon.h"
#include "NavImage.h"

#include "reconstructioncontext.h"

#include "Polygon.h"

#include "Dim2FileReader.h"

//#define PROGRESS
//#define PROGRESS_DISPLAY

#define OPENMVG_SFM_SFM_REGIONS_PROVIDER_HPP
#define OPENMVG_SFM_SFM_REGIONS_PROVIDER_CACHE_HPP

//#include "exifreader.hpp"
#include "openMVG/exif/exif_IO_EasyExif.hpp"

//#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"
#include "openMVG/geodesy/geodesy.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/stl/split.hpp"
#include "openMVG/sfm/sfm.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::exif;
using namespace openMVG::geodesy;
//using namespace openMVG::image;
using namespace openMVG::sfm;

typedef enum
{
    DIM2,
    EXIF,
    NONE
}NavMode;

// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init3DRecon, Init3DRecon)
#endif

/// Get camera matrix "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool Init3DRecon::getCameraIntrinsics(double & focal, double & ppx, double & ppy, const double &width, const double &height)
{
    QString camera_preset =  _matisseParameters->getStringParamValue("cam_param", "camera_preset");

    double scaling_factor;

    // Custom case
    if (camera_preset=="Custom")
    {
        bool Ok=false;
        QMatrix3x3 qK = _matisseParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
        double full_sensor_width=width;
        if(Ok)
            full_sensor_width=(double)_matisseParameters->getIntParamValue("cam_param","sensor_width",Ok);

        if(Ok)
        {
            scaling_factor = width/full_sensor_width;
            focal = scaling_factor*(qK(0,0)+qK(1,1))/2;
            ppx = scaling_factor*qK(0,2);
            ppy = scaling_factor*qK(1,2);
            return true;
        }
        else
        {
            camera_preset="Unknown";
        }
    }

    if (camera_preset=="Victor HD")
    {
        scaling_factor=width/1920.0;
        focal = scaling_factor*1430;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

     if (camera_preset=="Ariane Princ.")
     {
         scaling_factor=width/1920.0;
         focal = scaling_factor*1170;
         ppx = width / 2.0;
         ppy = height / 2.0;
         return true;
     }

    if (camera_preset=="Victor 4K" || camera_preset=="Nautile 4K")
    {
        scaling_factor=width/3840.0;
        focal = scaling_factor*2370;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    if (camera_preset=="Otus 2" || camera_preset=="Ariane Still camera")
    {
        scaling_factor=width/6000.0;
        focal = scaling_factor*6700;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    if (camera_preset=="Nautile Still camera")
    {
        scaling_factor=width/6000.0;
        focal = scaling_factor*5875.0;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    if (camera_preset=="Ariane Science." || camera_preset=="Scampi HD" || camera_preset=="Nautile HD")
    {
        scaling_factor=width/1920.0;
        focal = scaling_factor*1880;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    if (camera_preset=="Vortex still camera")
    {
        scaling_factor=width/6000.0;
        focal = scaling_factor*6000;
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    // Unkown case
    if (camera_preset=="Unknown")
    {
        focal = width; // approx in case we don't know
        ppx = width / 2.0;
        ppy = height / 2.0;
        return true;
    }

    return false;

}

static std::pair<bool, Vec3> checkGPS(const std::string & filename)
{
    std::pair<bool, Vec3> val(false, Vec3::Zero());
    std::unique_ptr<Exif_IO_EasyExif> exifReader(new Exif_IO_EasyExif);
    if (exifReader)
    {
        // Try to parse EXIF metada & check existence of EXIF data
        if ( exifReader->open( filename ) && exifReader->doesHaveExifInfo() )
        {
            // Check existence of GPS coordinates
            double latitude, longitude, altitude;
            if ( exifReader->GPSLatitude( &latitude ) &&
                 exifReader->GPSLongitude( &longitude ) &&
                 exifReader->GPSAltitude( &altitude ) )
            {
                // déjà traité par exifreader
                //if(exifReader->GPSAltitudeRef() != 0)
                //    altitude = -altitude; // dans ce cas, la valeur absolue est stockée dans le champ GPS altitude

                // Add position to the GPS position array
                val.first = true;
                val.second = Vec3(latitude, longitude, altitude);
            }
        }
    }
    return val;
}

static std::pair<bool, Vec3> checkDIM2
(
        const std::string & filename,
        Dim2FileReader *dim2Reader,
        std::map<QString, int> &dim2FileMap
        )
{
    QString fname = stlplus::filename_part(filename).c_str();

    std::pair<bool, Vec3> val(false, Vec3::Zero());
    if (dim2Reader && dim2Reader->isFileValid())
    {
        std::map<QString, int>::iterator it = dim2FileMap.find(fname);
        if(it != dim2FileMap.end())
        {
            NavInfo nav = dim2Reader->getNavInfo(it->second);
            val.first = true;
            val.second = Vec3( nav.latitude(), nav.longitude(), -nav.depth()  );
            return val;
        }
    }
    return val;
}

/// Check string of prior weights
static std::pair<bool, Vec3> checkPriorWeightsString
(
        const std::string &sWeights
        )
{
    std::pair<bool, Vec3> val(true, Vec3::Zero());
    std::vector<std::string> vec_str;
    stl::split(sWeights, ';', vec_str);
    if (vec_str.size() != 3)
    {
        std::cerr << "\n Missing ';' character in prior weights" << std::endl;
        val.first = false;
    }
    // Check that all weight values are valid numbers
    for (size_t i = 0; i < vec_str.size(); ++i)
    {
        double readvalue = 0.0;
        std::stringstream ss;
        ss.str(vec_str[i]);
        if (! (ss >> readvalue) )  {
            std::cerr << "\n Used an invalid not a number character in local frame origin" << std::endl;
            val.first = false;
        }
        val.second[i] = readvalue;
    }
    return val;
}


Init3DRecon::Init3DRecon() :
    Processor(NULL, "Init3DRecon", "Init 3D mosaic Descriptor with navigation", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "navFile");   // dim2 - défaut OTUS.dim2
    addExpectedParameter("dataset_param", "navSource"); // AUTO, GPS, DIM2, NO_NAV
    addExpectedParameter("dataset_param", "usePrior");

    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("cam_param",  "camera_preset");
    addExpectedParameter("cam_param",  "sensor_width");

    // unused
    //addExpectedParameter("algo_param", "scale_factor");
    // unused
    //addExpectedParameter("cam_param",  "V_Pose_C");
    // unused
    //addExpectedParameter("algo_param","filter_overlap");
    //addExpectedParameter("algo_param","min_overlap");
    //addExpectedParameter("algo_param","max_overlap");
}

Init3DRecon::~Init3DRecon(){

}

bool Init3DRecon::configure()
{
    return true;
}

void Init3DRecon::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool Init3DRecon::start()
{
    setOkStatus();

    std::cerr << "***********Init3DRecon start" << std::endl;
    return true;
}

bool Init3DRecon::stop()
{
    std::cerr << "\n***********Init3DRecon stop" << std::endl;
    return true;
}

void Init3DRecon::onFlush(quint32 port)
{
    reconstructionContext *reconstruction_context = new reconstructionContext();

    qDebug() << logPrefix() << "flush port " << port;

    emit signal_processCompletion(0);
    emit signal_userInformation("Init3DRecon - start");

    std::ostringstream error_report_stream;

    std::string sImageDir,
            //sfileDatabase = "",
            sOutputDir = "";

    std::string sPriorWeights;
    std::pair<bool, Vec3> prior_w_info(false, Vec3(1.0,1.0,1.0));

    int i_User_camera_model = PINHOLE_CAMERA_RADIAL3;

    bool b_Group_camera_model = true;
    bool Ok;
    bool use_prior = _matisseParameters->getBoolParamValue("dataset_param", "usePrior",Ok);
    if(!Ok)
        use_prior = false;
    //int i_GPS_XYZ_method = 0;

    Vec3 firstImagePos = Vec3::Zero();
    Vec3 firstImagePosUtm = Vec3::Zero();
    bool firstImage = true;

    // Expected properties for each image
    double width = -1, height = -1, focal = -1, ppx = -1,  ppy = -1;

    const EINTRINSIC e_User_camera_model = EINTRINSIC(i_User_camera_model);

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    sImageDir = rootDirnameStr.toStdString();
    QString QSep = QDir::separator();
    std::string SEP = QSep.toStdString();
    sOutputDir = sImageDir + SEP + "matches";

    if ( !stlplus::folder_exists( sImageDir ) )
    {
        fatalErrorExit("The input directory doesn't exist");
        return;
    }

    // Navigation source
    QString navSource =  _matisseParameters->getStringParamValue("dataset_param", "navSource");
    NavMode navMode = NONE;
    if(navSource == "NO_NAV")
        navMode = NONE;
    else if(navSource == "EXIF")
        navMode = EXIF;
    else if(navSource == "DIM2")
        navMode = DIM2;
    else
        navMode = DIM2;

    rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString navigationFile = _matisseParameters->getStringParamValue("dataset_param", "navFile");

    if (navigationFile.isEmpty())
        navigationFile = QString("noNav.dim2");

    std::string dim2FileName;

    if(navigationFile.contains(":/") || navigationFile.startsWith(QSep) )
    {
        // chemin absolu
        dim2FileName = navigationFile.toStdString();
    }
    else
    {
        dim2FileName = rootDirnameStr.toStdString() + SEP + navigationFile.toStdString();
    }

    if( navMode == DIM2 && !stlplus::file_exists(dim2FileName))
    {
        if( navSource == "DIM2")
        {
            QString msg = "Dim2 file not found " + navigationFile;
            emit signal_showInformationMessage(this->logPrefix(),msg);
            navMode = NONE;
        }
        else
            navMode = EXIF;

        dim2FileName = "";
    }

    std::map<QString, int> dim2FileMap;
    std::unique_ptr<Dim2FileReader> dim2FileReader(new Dim2FileReader(dim2FileName.c_str()));
    if(navMode == DIM2 && dim2FileReader != NULL && dim2FileReader->isFileValid() )
    {
        for(int i=1; i<= dim2FileReader->getNumberOfImages(); i++ )
        {
            dim2FileMap.insert(std::make_pair(dim2FileReader->getImageFilename(i),i));
        }

    }
    else if(navMode == DIM2 && dim2FileReader != NULL && !dim2FileReader->isFileValid() )
    {
        if( navSource == "DIM2")
        {
            QString msg = "Dim2 file invalid " + navigationFile;
            emit signal_showInformationMessage(this->logPrefix(),msg);
            navMode = NONE;
        }
        else
            navMode = EXIF;
    }

    if (sOutputDir.empty())
    {
        fatalErrorExit("Invalid output directory");
        return;
    }

    if ( !stlplus::folder_exists( sOutputDir ) )
    {
        if ( !stlplus::folder_create( sOutputDir ))
        {
            fatalErrorExit("Cannot create output directory");
            return;
        }
    }

    /*if (!getCameraIntrinsics(focal, ppx, ppy) )
    {
        fatalErrorExit("Invalid camera matrix input");
        return;
    }*/


    // Check if prior weights are given
    if (!sPriorWeights.empty() && use_prior)
    {
        prior_w_info = checkPriorWeightsString(sPriorWeights);
    }else if (use_prior){
        prior_w_info.first = true;
    }

    std::vector<std::string> vec_image = stlplus::folder_files( sImageDir );
    std::sort(vec_image.begin(), vec_image.end());

    // Configure an empty scene with Views and their corresponding cameras
    SfM_Data *sfm_data = new SfM_Data();
    sfm_data->s_root_path = sImageDir; // Setup main image root_path
    //Views & views = sfm_data->views;
    //Intrinsics & intrinsics = sfm_data->intrinsics;

    int counter=0;
    for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin();
          iter_image != vec_image.end();
          ++iter_image)
    {
        counter++;
        emit signal_processCompletion(100.0*(double)counter/(double)vec_image.size());

        // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
        width = height = ppx = ppy = focal = -1.0;

        const std::string sImageFilename = stlplus::create_filespec( sImageDir, *iter_image );
        const std::string sImFilenamePart = stlplus::filename_part(sImageFilename);

        // Test if the image format is supported:
        if (openMVG::image::GetFormat(sImageFilename.c_str()) == openMVG::image::Unknown)
        {
            continue; // image cannot be opened
        }

        if(sImFilenamePart.find("mask.png") != std::string::npos
                || sImFilenamePart.find("_mask.png") != std::string::npos)
        {
            error_report_stream
                    << sImFilenamePart << " is a mask image" << "\n";
            continue;
        }

        openMVG::image::ImageHeader imgHeader;
        if (!openMVG::image::ReadImageHeader(sImageFilename.c_str(), &imgHeader))
            continue; // image cannot be read

        width = imgHeader.width;
        height = imgHeader.height;
        ppx = width / 2.0;
        ppy = height / 2.0;

        std::unique_ptr<Exif_IO> exifReader(new Exif_IO_EasyExif);
        exifReader->open( sImageFilename );

        // Get camera intrinsics
        getCameraIntrinsics(focal, ppx, ppy, width, height);

        // Build intrinsic parameter related to the view
        std::shared_ptr<IntrinsicBase> intrinsic (NULL);

        if (focal > 0 && ppx > 0 && ppy > 0 && width > 0 && height > 0)
        {
            // Create the desired camera type
            switch(e_User_camera_model)
            {
            case PINHOLE_CAMERA:
                intrinsic = std::make_shared<Pinhole_Intrinsic>
                        (width, height, focal, ppx, ppy);
                break;
            case PINHOLE_CAMERA_RADIAL1:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K1>
                        (width, height, focal, ppx, ppy, 0.0); // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_RADIAL3:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K3>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0);  // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_BROWN:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Brown_T2>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0, 0.0); // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_FISHEYE:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Fisheye>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0); // setup no distortion as initial guess
                break;
            default:
                QString msg = "Error: unknown camera model: " +  QString::number((int) e_User_camera_model);
                fatalErrorExit(msg);
                return;
            }
        }

        // Build the view corresponding to the image
        // Navigation
        //
        std::pair<bool, Vec3> gps_info(false, Vec3::Zero());
        if(navMode == EXIF)
            gps_info = checkGPS(sImageFilename);
        else if(navMode == DIM2)
        {
            gps_info = checkDIM2(sImageFilename, dim2FileReader.get(), dim2FileMap);
        }

        // get first image info for relative shift (in UTM)
        if(gps_info.first && firstImage)
        {
            firstImage = false;
            // lat, lon, alt
            firstImagePos = gps_info.second;
            // UTM x, y, alt
            firstImagePosUtm = lla_to_utm( gps_info.second[0], gps_info.second[1], gps_info.second[2] );
        }

        if (gps_info.first && use_prior)
        {
            // utiliser la nav
            ViewPriors v(*iter_image, sfm_data->views.size(), sfm_data->views.size(), sfm_data->views.size(), width, height);

            // Add intrinsic related to the image (if any)
            if (intrinsic == NULL)
            {
                //Since the view have invalid intrinsic data
                // (export the view, with an invalid intrinsic field value)
                v.id_intrinsic = UndefinedIndexT;
            }
            else
            {
                // Add the defined intrinsic to the sfm_container
                sfm_data->intrinsics[v.id_intrinsic] = intrinsic;
            }

            v.b_use_pose_center_ = true;
            // utm
            gps_info.second = lla_to_utm( gps_info.second[0], gps_info.second[1], gps_info.second[2] );
            v.pose_center_ = gps_info.second - firstImagePosUtm;

            // prior weights
            if (prior_w_info.first == true)
            {
                v.center_weight_ = prior_w_info.second;
            }

            // Add the view to the sfm_container
            sfm_data->views[v.id_view] = std::make_shared<ViewPriors>(v);
        }
        else
        {
            View v(*iter_image, sfm_data->views.size(), sfm_data->views.size(), sfm_data->views.size(), width, height);

            // Add intrinsic related to the image (if any)
            if (intrinsic == NULL)
            {
                //Since the view have invalid intrinsic data
                // (export the view, with an invalid intrinsic field value)
                v.id_intrinsic = UndefinedIndexT;
            }
            else
            {
                // Add the defined intrinsic to the sfm_container
                sfm_data->intrinsics[v.id_intrinsic] = intrinsic;
            }

            // Add the view to the sfm_container
            sfm_data->views[v.id_view] = std::make_shared<View>(v);
        }
    }

    // Display saved warning & error messages if any.
    if (!error_report_stream.str().empty())
    {
        emit signal_showInformationMessage(this->logPrefix(),error_report_stream.str().c_str());
    }

    // Group camera that share common properties if desired (leads to more faster & stable BA).
    if (b_Group_camera_model)
    {
        GroupSharedIntrinsics(*sfm_data);
    }

    emit signal_userInformation("Init3DRecon - saving...");
    // Store SfM_Data views & intrinsic data
    if (!Save(  *sfm_data,
                stlplus::create_filespec( sOutputDir, "sfm_data.json" ).c_str(),
                ESfM_Data(VIEWS|INTRINSICS)))
    {
        fatalErrorExit("Error saving sfm_data.json");
        delete sfm_data;

        return;
    }

    // Add origin to reconstruction context
    reconstruction_context->lat_origin = firstImagePos[0];
    reconstruction_context->lon_origin = firstImagePos[1];
    reconstruction_context->alt_origin = firstImagePos[2];
    QVariant * reconstruction_context_stocker = new QVariant();
    reconstruction_context_stocker->setValue(reconstruction_context);
    _context->addObject("reconstruction_context",reconstruction_context_stocker);

    // Backup first image position to file
    // firstImagePos : en Lat, Lon, Alt
    QString coords = QString::number(firstImagePos[0],'f',6) +";"
            + QString::number(firstImagePos[1],'f',6) + ";"
            + QString::number(firstImagePos[2],'f',3);
    QString geoFileName = stlplus::create_filespec(rootDirnameStr.toStdString(),"georefpos.txt").c_str();
    QFile geoFile(geoFileName);
    if( !geoFile.open(QIODevice::WriteOnly) )
    {
        fatalErrorExit("Error saving " + geoFileName);
    }

    // Save the pos
    QTextStream outputGeoStream(&geoFile);
    outputGeoStream << coords;
    geoFile.close();

    // firstImagePosUtm : in UTM x, y, Alt
    QString utmcoords = QString::number(firstImagePosUtm[0],'f',3) +";"
            + QString::number(firstImagePosUtm[1],'f',3) + ";"
            + QString::number(firstImagePosUtm[2],'f',3);
    QString fileName = stlplus::create_filespec(rootDirnameStr.toStdString(),"utmrefpos.txt").c_str();
    QFile utmFile(fileName);
    if( !utmFile.open(QIODevice::WriteOnly) )
    {
        fatalErrorExit("Error saving " + fileName);
    }

    // Save the pos
    QTextStream outputStream(&utmFile);

    outputStream << utmcoords;
    utmFile.close();

    qDebug() << "\n"
             << "SfMInit_ImageListing report:\n"
             << "listed #File(s): " << vec_image.size() << "\n"
             << "usable #File(s) listed in sfm_data: " << sfm_data->GetViews().size() << "\n"
             << "usable #Intrinsic(s) listed in sfm_data: " << sfm_data->GetIntrinsics().size() << "\n";

    if(sfm_data->GetViews().size() == 0)
    {
        fatalErrorExit("No valid images found");
        delete sfm_data;

        return;
    }

    // do not crash...
    //delete sfm_data;

    sfm_data->poses.clear();
    sfm_data->control_points.clear();
    sfm_data->views.clear();
    std::shared_ptr<IntrinsicBase> intrinsic (NULL);
    sfm_data->intrinsics[0] = intrinsic;
    sfm_data->s_root_path = "";
    //crash
    //sfm_data->intrinsics.clear();


    emit signal_userInformation("Init3DRecon - end");

    flush(0);
}

