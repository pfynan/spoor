#pragma once

#include <cv.h>
#include <cvaux.h>
#include <boost/optional.hpp>

class FeatureExtract {
    public:
        FeatureExtract();
        boost::optional<cv::Point2f> operator() (cv::Mat& image);
    private:
        cv::BackgroundSubtractorMOG bsub;
};

