#include "MosaicDescriptor.h"
#include <QDebug>
#include <float.h>
#include <algorithm>
#include <vector>

MosaicDescriptor::MosaicDescriptor():_mosaicOrigin(0,0,0),
    _pixelSize(0,0),_mosaicSize(0,0), _utmHemisphere("UNDEF"),
    _utmZone(-1),_camerasOwner(true), _isInitialized(false)
{

    _mosaic_ullr.zeros(1,4,CV_64F);

}

MosaicDescriptor::~MosaicDescriptor()
{
    if(_camerasOwner){

        if (!_cameraNodes.isEmpty()){
            foreach (ProjectiveCamera* Cam, _cameraNodes) {
                delete Cam;
            }
        }
    }
}

Point3d MosaicDescriptor::mosaicOrigin() const
{
    return _mosaicOrigin;
}

void MosaicDescriptor::setMosaicOrigin(const Point3d &mosaicOrigin)
{
    _mosaicOrigin = mosaicOrigin;
}
Point2d MosaicDescriptor::pixelSize() const
{
    return _pixelSize;
}

void MosaicDescriptor::setPixelSize(const Point2d &pixelSize)
{
    _pixelSize = pixelSize;

    _Hs = (cv::Mat_<qreal>(3,3) <<
           _pixelSize.x,             0,       0,
           0,             _pixelSize.y,       0,
           0,                         0,       0);
}
Point2d MosaicDescriptor::mosaicSize() const
{
    return _mosaicSize;
}

void MosaicDescriptor::setMosaicSize(const Point2d &mosaicSize)
{
    _mosaicSize = mosaicSize;
}
QString MosaicDescriptor::utmHemisphere() const
{
    return _utmHemisphere;
}

void MosaicDescriptor::setUtmHemisphere(const QString &utmHemisphere)
{
    _utmHemisphere = utmHemisphere;
}
int MosaicDescriptor::utmZone() const
{
    return _utmZone;
}

void MosaicDescriptor::setUtmZone(int utmZone)
{
    _utmZone = utmZone;
}
Mat MosaicDescriptor::mosaic_ullr() const
{
    return _mosaic_ullr;
}

void MosaicDescriptor::setMosaic_ullr(const Mat &mosaic_ullr)
{
    _mosaic_ullr = mosaic_ullr;
}

void MosaicDescriptor::initCamerasAndFrames(QVector<ProjectiveCamera*> cameras_p, bool camerasOwner_p)
{
    _camerasOwner = camerasOwner_p;

    _cameraNodes = cameras_p;
    qreal meanLat=0;
    qreal meanLon=0;
    qreal meanAlt=0;
    qreal meanPixelSizeFor1meter=0;
    qreal minX, maxX, minY, maxY;
    qreal X,Y;
    QString utmZone;
    bool first=true;

    // Compute and set UTM Zone using all images
    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        meanLat += Cam->image()->navInfo().latitude();
        meanLon += Cam->image()->navInfo().longitude();
    }

    meanLat /= (qreal)_cameraNodes.size();
    meanLon /= (qreal)_cameraNodes.size();

    if ( !(_T.LatLongToUTM(meanLat, meanLon, X, Y, utmZone)) ){
        qDebug() << "Cannot retrieve UTM Zone\n";
        exit(1);
    }else{
        QStringList utmParams = utmZone.split(" ");
        setUtmZone(utmParams.at(0).toInt());
        setUtmHemisphere(utmParams.at(1));
    }

    // Projection of all images navigation to utmZone of the MosaicDescriptor
    // Compute at the same time min and max for X & Y and meanAlt
    foreach (ProjectiveCamera* Cam, _cameraNodes) {

        if ( !(_T.LatLongToUTM(Cam->image()->navInfo().latitude(),
                               Cam->image()->navInfo().longitude(),
                               X, Y, utmZone, true)) ){
            qDebug() << "Cannot convert point to UTM\n";
            exit(1);
        }else{

            // Affect UTM values
            Cam->image()->navInfo().setUtmX(X);
            Cam->image()->navInfo().setUtmX(Y);
            Cam->image()->navInfo().setUtmZone(utmZone);

            // Compute min max mean
            if (first){
                minX = X;
                maxX = minX;
                minY = Y;
                maxY = minY;
                meanAlt = Cam->image()->navInfo().altitude();
                meanPixelSizeFor1meter = 2/(Cam->K().at<qreal>(0,0)+Cam->K().at<qreal>(1,1));

                first = false;
            }else{
                if (X < minX) minX = X;
                if (X > maxX) maxX = X;
                if (Y < minY) minY = Y;
                if (Y > maxY) maxY = Y;
                meanAlt = meanAlt + Cam->image()->navInfo().altitude();
                meanPixelSizeFor1meter += 2/(Cam->K().at<qreal>(0,0)+Cam->K().at<qreal>(1,1));
            }

        }
    }
    meanAlt /= (qreal)_cameraNodes.size();
    meanPixelSizeFor1meter /= (qreal)_cameraNodes.size();

    this->setMosaicOrigin(Point3d(minX,maxY,meanAlt));
    this->setPixelSize(Point2d(meanPixelSizeFor1meter*meanAlt,meanPixelSizeFor1meter*meanAlt));

    // Compute the Transformation 3D Mosaic Frame -> 3D World Frame: W_X = W_R_M * M_X + W_T_M
    // Mosaic Rotation w.r.t. 3D World Frame
    _W_R_M = _T.RotX(CV_PI);
    // Mosaic Translation w.r.t. 3D World Frame
    // (The mosaic is translated according to the X, Y origin but not in moved in Z).
    _W_T_M = (cv::Mat_<qreal>(3,1) << _mosaicOrigin.x, _mosaicOrigin.y, 0 );

    // Compute Inverse Transformation 3D World Frame -> 3D Mosaic Frame: M_X = M_R_W * W_X + M_T_W
    cv::transpose(_W_R_M, _M_R_W);
    _M_T_W = -_M_R_W * _W_T_M;

    _isInitialized = true;

    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        computeCameraHomography(Cam);
    }

}

void MosaicDescriptor::computeCameraHomography(ProjectiveCamera *camera_p)
{
    if(!isInitialized()){
        qDebug() << "Initialize the MosaicDescriptor before calling this function. Exiting... \n";
        exit(1);
    }

    NavInfo * navdata = &(camera_p->image()->navInfo());

    // The Yaw angle is Clockwise
    double Yaw = -navdata->yaw();



    // Vehicle Rotation w.r.t. 3D World Frame (Computed always in mobile axis: post-multiplication)
    //  Rotation in Z (pi/2): Point X axis of the vehicle to the north to use the Yaw.
    //  Rotation in Z (Yaw): Yaw of the vehicle in the world frame.
    //  Rotation in X (Pi): Point Z axis of the vehicle looking down.
    //  Rotation in Y (Pitch): Pitch in the vehicle frame.
    //  Rotation in X (Roll): Roll in the vehicle frame.
    _W_R_V = _T.RotZ ( CV_PI / 2 ) * _T.RotZ ( Yaw ) * _T.RotX ( CV_PI ) * _T.RotY ( navdata->pitch() ) * _T.RotX ( navdata->roll() );
    // Vehicle Translation w.r.t. 3D World Frame
    _W_T_V = (cv::Mat_<qreal>(3,1) << navdata->utmX(), navdata->utmY(), navdata->altitude() );

    // Convert a Pose (V_T_C, V_R_C) w.r.t. 3D Vehicle Frame to the 3D World Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D World Frame: W_X = W_R_C * C_X + W_T_C

    _W_R_C = _W_R_V * camera_p->V_R_C();
    _W_T_C = _W_R_V * camera_p->V_T_C() + _W_T_V;

    // Convert a Pose (W_T_C, W_R_C) w.r.t. 3D Camera Frame to the 3D Mosaic Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D Mosaic Frame: M_X = M_R_C * C_X + M_T_C
    _M_R_C = _M_R_W * _W_R_C;
    _M_T_C = _M_R_W * _W_T_C + _M_T_W;

    // Compute Inverse Transformation 3D Mosaic Frame -> 3D Camera Frame: C_X = C_R_M * M_X + C_T_M
    cv::transpose(_M_R_C,_C_R_M);
    _C_T_M = -_C_R_M * _M_T_C;

    // Compute Extrinsics using the 3D Mosaic Frame
    cv::hconcat(_C_R_M, _C_T_M, _C_M_M);
    // Compute Projection Matrix (Intrinsics * Extrinsics): 3D Mosaic Frame -> 2D Image Plane
    _i_P_M = camera_p->K() * _C_M_M;

    // Set Z = 0 (Delete 3th Column) in the Projection Matrix: 3D world to 2D
    // Up-To-Scale Planar Projective Homography: 2D Mosaic Frame to 2D Image Frame
    //_i_H_m = _i_P_M(:, [1 2 4]);
    cv::hconcat(_i_P_M.colRange(0,2),_i_P_M.colRange(3,4), _i_H_m);

    // Absolute Homography to be stored into the mosaic: 2D Image Plane to 2D Mosaic Frame
    // Add the scaling factor to have it in pixels.
    cv::invert( _i_H_m, _m_H_i);
    _m_H_i = _Hs * _m_H_i;

    // Store Normalized Homography into the mosaic structure ([3,3] element is one).
    camera_p->set_m_H_i( _m_H_i / _m_H_i.at<qreal>(2,2) );
}

void MosaicDescriptor::computeMosaicExtentAndShiftFrames()
{

    cv::Mat mosaicbounds = (cv::Mat_<qreal>(4,1) << FLT_MAX, FLT_MAX, 0, 0 );
    int w,h =0;

    cv::Mat pt1,pt2,pt3,pt4;
    std::vector<qreal> xArray, yArray;
    std::vector<qreal>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    foreach (ProjectiveCamera* Cam, _cameraNodes) {

        w = Cam->image()->width();
        h = Cam->image()->height();

        // Project corners_p on mosaic plane
        Cam->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   0,   1), pt1);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, 0,   1), pt2);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, h-1, 1), pt3);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   h-1, 1), pt4);

        // Fill x & y array
        xArray.clear();
        yArray.clear();
        xArray.push_back(pt1.at<qreal>(0,0)/pt1.at<qreal>(2,0));
        yArray.push_back(pt1.at<qreal>(1,0)/pt1.at<qreal>(2,0));
        xArray.push_back(pt2.at<qreal>(0,0)/pt2.at<qreal>(2,0));
        yArray.push_back(pt2.at<qreal>(1,0)/pt2.at<qreal>(2,0));
        xArray.push_back(pt3.at<qreal>(0,0)/pt3.at<qreal>(2,0));
        yArray.push_back(pt3.at<qreal>(1,0)/pt3.at<qreal>(2,0));
        xArray.push_back(pt4.at<qreal>(0,0)/pt4.at<qreal>(2,0));
        yArray.push_back(pt4.at<qreal>(1,0)/pt4.at<qreal>(2,0));


        // Compute min,max
        min_x_it = std::min_element(std::begin(xArray), std::end(xArray));
        min_y_it = std::min_element(std::begin(yArray), std::end(yArray));
        max_x_it = std::max_element(std::begin(xArray), std::end(xArray));
        max_y_it = std::max_element(std::begin(yArray), std::end(yArray));

        mosaicbounds.at<qreal>(0,0) = std::min(*min_x_it, mosaicbounds.at<qreal>(0, 0));
        mosaicbounds.at<qreal>(1,0) = std::max(*max_x_it,mosaicbounds.at<qreal>(1, 0));
        mosaicbounds.at<qreal>(0,1) = std::min(*min_y_it,mosaicbounds.at<qreal>(0, 1));
        mosaicbounds.at<qreal>(1,1) = std::max(*max_x_it,mosaicbounds.at<qreal>(1, 1));

    }

    // Shift all homographies with H
    cv::Mat H = (cv::Mat_<qreal>(3,3) << 1, 0, -mosaicbounds.at<qreal>(0,0),
                 0, 1, -mosaicbounds.at<qreal>(0,1),
                 0, 0, 1);

    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        Cam->set_m_H_i(H*Cam->m_H_i());
    }

    // Shift origin
    _mosaicOrigin.x = _mosaicOrigin.x - (-mosaicbounds.at<qreal>(0,0))*_pixelSize.x;
    _mosaicOrigin.y = _mosaicOrigin.y + (-mosaicbounds.at<qreal>(0,1))*_pixelSize.y;
    _mosaicSize.x = std::ceil(mosaicbounds.at<qreal>(1,0)-mosaicbounds.at<qreal>(0,0))+2; //+2 due to the 0 and the round
    _mosaicSize.y = std::ceil(mosaicbounds.at<qreal>(1,1)-mosaicbounds.at<qreal>(0,1))+2;


    // Upper Left and Lower Right corner coordinates
    qreal x_shift = (mosaicbounds.at<qreal>(1,0)-mosaicbounds.at<qreal>(0,0));
    qreal y_shift = (mosaicbounds.at<qreal>(1,1)-mosaicbounds.at<qreal>(0,1));
    _mosaic_ullr = (cv::Mat_<qreal>(4,1) << _mosaicOrigin.x, _mosaicOrigin.y,
                    _mosaicOrigin.x+x_shift*_pixelSize.x, _mosaicOrigin.y-y_shift*_pixelSize.y);


}
QVector<ProjectiveCamera *> MosaicDescriptor::cameraNodes() const
{
    return _cameraNodes;
}

bool MosaicDescriptor::isInitialized() const
{
    return _isInitialized;
}







