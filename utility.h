
#pragma once

#include <cv.h>
#include <boost/optional.hpp>

#include <cmath>

inline void cross(cv::Mat& img,cv::Point pt,int size=10) {
    using namespace cv;


    Scalar color = Scalar(0,0,255);
    int thick = 3;
    line(img,pt - Point(size/2,size/2),pt + Point(size/2,size/2),color,thick);
    line(img,pt - Point(-size/2,size/2),pt + Point(-size/2,size/2),color,thick);
}

inline boost::optional<cv::Point2f> getBiggestBlob(const cv::Mat &image) {
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

// Mean of 0; ||square_wave|| = sqrt(N)
inline float square_wave(int t) {
    return 4.0*std::floor(t/(2*M_PI)) - 2.0*std::floor(2.0*t/(2*M_PI)) + 1.0;
}

