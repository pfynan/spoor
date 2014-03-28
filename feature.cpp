
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <cv.h>
#include <highgui.h>

#include "utility.h"
#include "feature.h"

#include <boost/timer/timer.hpp>

using namespace std;
using namespace boost;
using namespace cv;


boost::optional<cv::Point2f> getBiggestBlob(const cv::Mat &image) {
    using namespace cv;
    using namespace boost;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    vector<Moments> mu(contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mu[i] = moments( contours[i], false );
    }

    ///  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    }

  /*for( size_t  i = 0; i< contours.size(); i++ )
      {
      Scalar color = Scalar(0,255,0);
      drawContours( disp, contours, i, color, 2, 8, hierarchy, 0, Point() );

      }*/

    auto blob = max_element(begin(mu),end(mu),[](Moments a, Moments b){ return a.m00 < b.m00; });

    Point2f measurement;

    if(blob != end(mu) && blob->m00 > 40) {
        measurement.x = blob->m10/blob->m00;
        measurement.y = blob->m01/blob->m00;
        return optional<Point2f>(measurement);

    } else {
        return optional<Point2f>();
    }

}



FeatureExtract::FeatureExtract(Size size, ImLogger &log) : 
    frame_size(size),
    logger(log),
    bg_sub(100,3,0.1,30*0.5)
{

}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& image)
{
    //timer::auto_cpu_timer time_it("%u\n"); 

    cvtColor(image,image,CV_BGR2GRAY);
    //image.convertTo(image,CV_32FC1);
    //normalize(image,image, 0, 1, NORM_MINMAX, CV_32FC1);

    //image /= 255;

    //Size work_size = frame_size;//Size((image.cols + 16)/32,(image.rows + 16)/32);
    //resize(image,image,work_size);




    Mat mog;

    bg_sub(image,mog);

    logger.log("mog", mog);





    return getBiggestBlob(mog);
    //return optional<Point2f>();

}

