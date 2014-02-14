
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <cv.h>
#include <highgui.h>

#include "utility.h"
#include "feature.h"

using namespace std;
using namespace boost;
using namespace cv;

FeatureExtract::FeatureExtract(Size size) : buffer(length),
    frame_size(size)
{

    r = 1;
    fs = 25;
    w = 4.0/fs * 2.0 * M_PI;

}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& image)
{
    cvtColor(image,image,CV_BGR2GRAY);
    image.convertTo(image,CV_32FC1);
    //normalize(image,image, 0, 1, NORM_MINMAX, CV_32FC1);


    //adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,63,-150);

    buffer.push_back(image.clone()); // Oh honey. This is going to churn the heap.

    Mat buf_mean,buf_norm;

    buf_mean = Mat::zeros(frame_size,CV_32FC1);
    buf_norm = Mat::zeros(frame_size,CV_32FC1);

    for(Mat& x : buffer) {
        buf_mean += x;
        buf_norm += x.mul(x);
    }

    buf_norm = buf_norm - buf_mean.mul(buf_mean) / buffer.size();
    sqrt(buf_norm,buf_norm);

    buf_mean /= buffer.size();

    vector<Mat> corrs;

    for(size_t m = 0; m < buffer.size(); ++m) {
        Mat buf_corr = Mat::zeros(frame_size,CV_32FC1);

        for(size_t i = 0; i < buffer.size(); ++i) {
            buf_corr += (buffer[i] - buf_mean) * square_wave(w*i + m);
        }

        buf_corr /= buf_norm * sqrt(buffer.size());

        corrs.push_back(buf_corr);

    }

    auto big_corr = max_element(begin(corrs),end(corrs),[](const Mat &x,const Mat &y) {return sum(x)[0] < sum(y)[0];});

    //cout << "Frame" << endl;

    //if(big_corr != end(corrs)) {
        //imshow("Out",*big_corr * 0.5 + 0.5);
        //waitKey(1);
    //}




    *big_corr *= 128;
    *big_corr += 128;
    big_corr->convertTo(image,CV_8UC1);


    return optional<Point2f>(); 
        //getBiggestBlob(image);

}

