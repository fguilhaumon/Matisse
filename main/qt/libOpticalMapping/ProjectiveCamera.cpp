#include "ProjectiveCamera.h"


ProjectiveCamera::ProjectiveCamera()
{

}

ProjectiveCamera::ProjectiveCamera(NavImage *image_p, Mat cameraMatrixK_p, Mat V_T_C, Mat V_R_C, qreal scaleFactor_p)
{

    this->setImage(image_p);
    this->setK(cameraMatrixK_p);
    this->setScalingFactor(scaleFactor_p);
    this->setV_T_C(V_T_C);
    this->setV_R_C(V_R_C);

}
cv::Mat ProjectiveCamera::K() const
{
    return _K;
}

void ProjectiveCamera::setK(const cv::Mat &K)
{
    _K = _scalingFactor*K;
}
cv::Mat ProjectiveCamera::m_H_i() const
{
    return _m_H_i;
}

void ProjectiveCamera::set_m_H_i(const cv::Mat &m_H_i)
{
    _m_H_i = m_H_i;
}
cv::Mat ProjectiveCamera::V_T_C() const
{
    return _V_T_C;
}

void ProjectiveCamera::setV_T_C(const cv::Mat &V_T_C)
{
    _V_T_C = V_T_C;
}
cv::Mat ProjectiveCamera::V_R_C() const
{
    return _V_R_C;
}

void ProjectiveCamera::setV_R_C(const cv::Mat &V_R_C)
{
    _V_R_C = V_R_C;
}
NavImage *ProjectiveCamera::image() const
{
    return _image;
}

void ProjectiveCamera::setImage(MatisseCommon::NavImage *image)
{
    _image = image;
}
qreal ProjectiveCamera::scalingFactor() const
{
    return _scalingFactor;
}

void ProjectiveCamera::setScalingFactor(const qreal &scalingFactor)
{
    if (scalingFactor >= 1){
        _scalingFactor = 1;
    }else if (scalingFactor <= 0){
        exit(1);
    }else{
        _scalingFactor = scalingFactor;
    }

}

void ProjectiveCamera::projectPtOnMosaickingPlane(const Mat camPlanePt_p, Mat &mosaicPlanePt_p)
{
    mosaicPlanePt_p = _m_H_i * camPlanePt_p;
}

void ProjectiveCamera::projectImageOnMosaickingPlane(Mat &mosaicPlaneImage_p, Mat &mosaicPlaneMask_p, cv::Point & corner_p)
{

    cv::Mat pt1,pt2,pt3,pt4;
    std::vector<qreal> xArray, yArray;
    std::vector<qreal>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    int w = image()->width();
    int h = image()->height();


    // Project corners_p on mosaic plane
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   0,   1), pt1);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, 0,   1), pt2);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, h-1, 1), pt3);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   h-1, 1), pt4);

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

    // Compute dstSize
    int dstWidth = ceil(*max_x_it)-floor(*min_x_it) + 1;
    int dstHeight = ceil(*max_y_it)-floor(*min_y_it) + 1;

    cv::Size dstSize(dstWidth,dstHeight);

    corner_p.x = floor(*min_x_it);
    corner_p.y = floor(*min_y_it);

    // Project image on mosaic
    cv::Mat H = (cv::Mat_<qreal>(3,3) <<1, 0, -corner_p.x,0, 1, -corner_p.y,0, 0, 1)*_m_H_i;

    cv::warpPerspective(*(image()->imageData()), mosaicPlaneImage_p, H, dstSize);

    std::cerr << "_m_H_i =" << _m_H_i ;

    imshow("Test",mosaicPlaneImage_p);
    waitKey();

    // Project mask corresponding to images
    cv::Mat imageMask;
    imageMask.create(image()->imageData()->size(), CV_8U);
    imageMask.setTo(Scalar::all(255));

    cv::warpPerspective(imageMask, mosaicPlaneMask_p, H, dstSize);

}






