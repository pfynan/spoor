#pragma once

#include <cv.h>
#include <cvaux.h>
#include <boost/optional.hpp>
#include <boost/circular_buffer.hpp>

#include <utility>
#include <vector>

#include "ImLogger.h"

class FeatureExtract {
    public:
        FeatureExtract(cv::Size size,ImLogger &log);
        boost::optional<cv::Point2f> operator() (cv::Mat& image);
    private:
        cv::Size frame_size;

        ImLogger &logger;
        cv::BackgroundSubtractorMOG bg_sub;
        cv::KalmanFilter KF;

        

};
