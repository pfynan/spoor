
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
    corr_buffer(length),
    filt_buffer(length),
    frame_size(size),
    logger(log)
{

    fs = 25;
    w = 4.0/fs * 2.0 * M_PI;


    for (int i = 0; i < length; ++i)
    {
        corr_buffer.push_back(Mat::zeros(frame_size,CV_32FC1));
    }

}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& image)
{
    //timer::auto_cpu_timer time_it("%u\n"); 

    cvtColor(image,image,CV_BGR2GRAY);
    image.convertTo(image,CV_32FC1);
    //normalize(image,image, 0, 1, NORM_MINMAX, CV_32FC1);

    image /= 255;

    Size work_size = frame_size;//Size((image.cols + 16)/32,(image.rows + 16)/32);
    //resize(image,image,work_size);



    Mat buf_mean,buf_norm;


    auto stats = getStatistics(begin(corr_buffer),end(corr_buffer));

    corr_buffer.push_back(image.clone()); // Oh honey. This is going to churn the heap.

    buf_mean = stats.first;
    buf_norm = stats.second;


    /*for(size_t i = 0; i < corr_buffer.size(); ++i) {
        acc_corr += (corr_buffer[i] - buf_mean) * (bit_pattern(-i,221,8,25)*2-1);
    }*/

    //acc_corr /= buf_norm * sqrt(corr_buffer.size());
    
    Mat sd;
    sqrt(buf_norm,sd);

    logger.log("meaned", image - buf_mean + 0.5);
    logger.log("variance", buf_norm);
    logger.log("sigmas", (image - buf_mean) / sd);

    Mat bg_time = (buf_mean);

    logger.log("bg_time", bg_time);

    Mat bg_space;
    blur(image,bg_space,Size(11,11));
    logger.log("bg_space", image - bg_space + 0.5);

    //acc_corr = acc_corr * 0.75 + image * 0.25;
    Mat acc_corr = (image - bg_time)/sd;
    acc_corr = acc_corr.mul(image-bg_space);


    logger.log("acc_corr", acc_corr);

    //threshold(acc_corr,acc_corr,0.2,1,THRESH_BINARY);
    logger.log("thresh1", acc_corr);

    //acc_corr *= 0.5;
    acc_corr *= 255;
    acc_corr.convertTo(acc_corr,CV_8UC1);

    logger.log("acc_corri", acc_corr);

    
    /*Mat gb,bb;
    Size blur_size = Size(51,51);

    GaussianBlur(acc_corr,gb,blur_size,0);
    logger.log("gb", gb);
    blur(acc_corr,bb,blur_size);
    logger.log("bb", bb);
    */

    //adaptiveThreshold(acc_corr,acc_corr,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,0);



    return getBiggestBlob(acc_corr);
    //return optional<Point2f>();

}

