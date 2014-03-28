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
        boost::circular_buffer<cv::Mat> corr_buffer,filt_buffer;
        cv::Size frame_size;

        double fs;
        double fn;
        double w;

        static constexpr size_t length = 25;

        std::vector<float> filt_coeffs;

        ImLogger &logger;

        

};

template<class T>
std::pair<cv::Mat,cv::Mat> getStatistics(T first,T last) {
    using namespace cv;
    Mat buf_mean, buf_norm;

    if(first == last)
        return std::make_pair(buf_mean, buf_norm);

    buf_mean = Mat::zeros(first->size(),CV_32FC1);
    buf_norm = Mat::zeros(first->size(),CV_32FC1);

    for(T i = first; i != last; ++i) {
        buf_mean += *i;
        buf_norm += i->mul(*i);
    }

    buf_norm = buf_norm - buf_mean.mul(buf_mean) / (last - first);
    sqrt(buf_norm,buf_norm);

    buf_mean /= (last - first);

    return std::make_pair(buf_mean, buf_norm);
}
