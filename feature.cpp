
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


vector<float> conv(vector<float> f, vector<float> g) {
    int N = f.size();
    vector<float> ret(N);
    fill(ret.begin(),ret.end(),0);
    for(int n = 0; n < N; ++n) {
        for(int m = 0; m < N; ++m) {
            ret[n] += f[m] * g[mod(n-m,N)];
        }
    }
    return ret;
}



FeatureExtract::FeatureExtract(Size size, ImLogger &log) : 
    corr_buffer(length),
    filt_buffer(length),
    frame_size(size),
    logger(log)
{

    fs = 25;
    w = 4.0/fs * 2.0 * M_PI;

    // Precompute filter for faster computation


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


    corr_buffer.push_back(image.clone()); // Oh honey. This is going to churn the heap.

    Mat buf_mean,buf_norm;

    auto stats = getStatistics(begin(corr_buffer),end(corr_buffer));

    buf_mean = stats.first;
    buf_norm = stats.second;

    Mat acc_corr = Mat::zeros(work_size,CV_32FC1);

    for(size_t i = 0; i < corr_buffer.size(); ++i) {
        acc_corr += (corr_buffer[i] - buf_mean) * (bit_pattern(-i,221,8,25)*2-1);
    }

    //acc_corr /= buf_norm * sqrt(corr_buffer.size());
    

    logger.log("meaned", image - buf_mean + 0.5);
    logger.log("acc_corr", acc_corr);

    threshold(acc_corr,acc_corr,0.4,1.0,THRESH_BINARY);

    logger.log("thresh1", acc_corr);


    filt_buffer.push_front(acc_corr.clone());

    Mat acc_filt = Mat::zeros(work_size,CV_32FC1);
    for(Mat &x : filt_buffer) {
        acc_filt += x;
    }

    acc_filt /= 0.5*filt_buffer.size();

    acc_filt = acc_filt;

    logger.log("acc_filt", acc_filt);


    //adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,63,-150);
    

    image = acc_filt;

    threshold(image,image,0.5,1.0,THRESH_BINARY);
     

    logger.log("thresh2", image);



    //cout << "Frame" << endl;

    //if(big_corr != end(corrs)) {
        //imshow("Out",*big_corr * 0.5 + 0.5);
        //waitKey(1);
    //}

    //resize(image,image,frame_size);


    image *= 255;
//    acc_filt += 128;
    image.convertTo(image,CV_8UC1);




    return getBiggestBlob(image);
    //return optional<Point2f>();

}

