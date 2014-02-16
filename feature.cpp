
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

FeatureExtract::FeatureExtract(Size size) : corr_buffer(length),
    filt_buffer((length+1)/2),
    frame_size(size)
{

    fs = 25;
    w = 4.0/fs * 2.0 * M_PI;

}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& image)
{
    cvtColor(image,image,CV_BGR2GRAY);
    image.convertTo(image,CV_32FC1);
    //normalize(image,image, 0, 1, NORM_MINMAX, CV_32FC1);

    image /= 255;


    corr_buffer.push_back(image.clone()); // Oh honey. This is going to churn the heap.

    Mat buf_mean,buf_norm;

    buf_mean = Mat::zeros(frame_size,CV_32FC1);
    buf_norm = Mat::zeros(frame_size,CV_32FC1);

    for(Mat& x : corr_buffer) {
        buf_mean += x;
        buf_norm += x.mul(x);
    }

    buf_norm = buf_norm - buf_mean.mul(buf_mean) / corr_buffer.size();
    sqrt(buf_norm,buf_norm);

    buf_mean /= corr_buffer.size();

    Mat acc_corr = Mat::zeros(frame_size,CV_32FC1);

    for(size_t i = 0; i < corr_buffer.size(); ++i) {
        acc_corr += (corr_buffer[i] - buf_mean) * square_wave(w*i);
    }

    acc_corr /= buf_norm * sqrt(corr_buffer.size());

    acc_corr = abs(acc_corr);

    filt_buffer.push_front(acc_corr.clone());

    Mat acc_filt = Mat::zeros(frame_size,CV_32FC1);
    for(Mat &x : filt_buffer) {
        acc_filt += x;
    }

    acc_filt /= 0.5*filt_buffer.size();



    //adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,63,-150);
    
    threshold(acc_filt,acc_filt,0.7,1.0,THRESH_BINARY);

    image = image.mul(acc_filt);

    threshold(image,image,0.7,1.0,THRESH_BINARY);
    




    //cout << "Frame" << endl;

    //if(big_corr != end(corrs)) {
        //imshow("Out",*big_corr * 0.5 + 0.5);
        //waitKey(1);
    //}




    image *= 255;
//    acc_filt += 128;
    image.convertTo(image,CV_8UC1);




    return getBiggestBlob(image);

}

