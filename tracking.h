#pragma once

#include <cv.h>
#include <boost/optional.hpp>

class Tracking {
    public:
        Tracking();
        cv::Point2f operator() (boost::optional<cv::Point2f> measurement);
    private:
        cv::KalmanFilter KF;
};

