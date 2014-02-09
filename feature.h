#pragma once

#include <cv.h>
#include <cvaux.h>
#include <boost/optional.hpp>
#include <boost/circular_buffer.hpp>

class FeatureExtract {
    public:
        FeatureExtract(cv::Size size);
        boost::optional<cv::Point2f> operator() (cv::Mat& image);
    private:
        boost::circular_buffer<cv::Mat> buffer;
        cv::Size frame_size;

        double r;
        double fs;
        double fn;
        double w;

        static constexpr size_t length = 25;

};

