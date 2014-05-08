#pragma once

#include <cv.h>
#include <cvaux.h>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>

#include <vector>

#include "ImLogger.h"

class FeatureExtract {
    public:
        FeatureExtract(cv::Size size,boost::shared_ptr<ImLogger> log);
        boost::optional<cv::Point2f> operator() (cv::Mat& image);
    private:
        void initKalman();
        void initCorr();
        cv::Size frame_size;

        boost::shared_ptr<ImLogger> logger;
        cv::KalmanFilter KF;

        boost::circular_buffer<bool> was_there, expected_there;

        bool has_lock;
        

};
