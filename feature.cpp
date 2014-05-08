
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <cv.h>
#include <highgui.h>

#include "feature.h"

#include <boost/timer/timer.hpp>

using namespace std;
using namespace boost;
using namespace cv;

vector<pair<Point2f,Moments>> getBlobs(Mat &image);
template<class T>
int circ_hamm_dist(T a, T b);

FeatureExtract::FeatureExtract(Size size, boost::shared_ptr<ImLogger> log) : 
    frame_size(size),
    logger(log)
{
    initKalman();
    initCorr();
    has_lock = false;
}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& im)
{

    // So, I truly apologize for this function. It was written the night
    // before...
    //
    // But it works! And is somewhat clever.
    //
    
    const int blur_size = 9;
    const int morph_size = 5;
    const int thresh_value = 200;
    const double blob_min_area = 16;
    const double blob_max_area = 160;
    const double blob_max_hu0 = 0.3;
    const double blob_max_distance_sds = 2;
    const int max_hamming_distance = 5;

    cvtColor(im,im,CV_BGR2GRAY);


    GaussianBlur(im,im,Size(blur_size,blur_size),0);

    //dilate(im,im,getStructuringElement(MORPH_ELLIPSE,Size(7,7)));
    morphologyEx(im,im,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(morph_size,morph_size)));

    threshold(im,im,thresh_value,255,THRESH_BINARY);


    auto blobs = getBlobs(im);

    // Blob classification/elimination

    vector<pair<Point2f,Moments>> mags;

    if(has_lock) {
        KF.predict();
    }

    Mat iobs_cov = KF.measurementMatrix * KF.errorCovPost * KF.measurementMatrix.t() + KF.measurementNoiseCov;
    invert(iobs_cov,iobs_cov,DECOMP_SVD);

    for (int i = 0; i < blobs.size(); ++i)
    {
        float mahal = Mahalanobis(Mat(blobs[i].first),KF.statePost(Range(0,2),Range(0,1)),iobs_cov);
        vector<double> humoments;
        HuMoments(blobs[i].second,humoments);

        // Eliminate on:

        //Size
        if(!(blobs[i].second.m00 > blob_min_area && blobs[i].second.m00 < blob_max_area)) continue;
        // Shape
        if(humoments[0] > blob_max_hu0) continue;

        // Unlikely position
        if(has_lock && mahal > blob_max_distance_sds) continue;

        // Otherwise:
        mags.push_back(blobs[i]);

    }

    // Chose biggest
    auto nearest = min_element(begin(mags),end(mags),[=](pair<Point2f,Moments> x,pair<Point2f,Moments> y) { return x.second.m00 < y.second.m00; });


    if(nearest != end(mags)) {
        KF.correct(Mat(nearest->first));
    }

    was_there.push_back(mags.empty() ? false : true);

    int hamm = circ_hamm_dist(was_there,expected_there);

    if(hamm >= max_hamming_distance) {
        has_lock = false;
        return optional<Point2f>();
    }
    else {
        has_lock = true;
        Point2f estimated = Point2f(KF.statePost(Range(0,2),Range(0,1)));;

        return optional<Point2f>(estimated);
    }

}

void FeatureExtract::initKalman() {
    KF = KalmanFilter(4,2,0);
    KF.transitionMatrix = *(Mat_<float>(4, 4) << 
            1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1);

    setIdentity(KF.measurementMatrix);

    setIdentity(KF.processNoiseCov);
    KF.processNoiseCov *= 144;


    setIdentity(KF.measurementNoiseCov, Scalar::all(9));
    setIdentity(KF.errorCovPost, Scalar::all(1e+4));


    KF.statePost = *(Mat_<float>(4,1) << frame_size.width/2, frame_size.height/2, 0, 0);

}

void FeatureExtract::initCorr() {
    was_there = circular_buffer<bool>(15);
    expected_there = circular_buffer<bool>(15);

    for (int i = 0; i < 8; ++i)
    {
        expected_there.push_back(true);
    }

    for (int i = 0; i < 7; ++i)
    {
        expected_there.push_back(false);
    }

    for (int i = 0; i < 15; ++i)
    {
        was_there.push_back(false);
    }
}

vector<pair<Point2f,Moments>> getBlobs(Mat &image) {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);


    vector<Moments> mu(contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mu[i] = moments( contours[i], false );
    }

    ///  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);
    }

    vector<pair<Point2f,Moments>> ret(contours.size());

    for( size_t i = 0; i < contours.size(); i++ ) { 
        ret[i] = make_pair(mc[i],mu[i]);
    }

    return ret;
}

template<class T>
int circ_hamm_dist(T a, T b) {
    assert(a.size() == b.size());

    int acc_0 = INT_MAX;

    for(int j = 0; j < a.size(); ++j) {
        int acc = 0;
        for(int i = 0; i < a.size(); ++i) {
            if(a[(i + j) % a.size()] != b[i]) ++acc;
        }

        if(acc < acc_0) acc_0 = acc;
    }

    return acc_0;
}

