
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

vector<pair<Point2f,float>> getBlobs(Mat &image) {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    vector<Moments> mu(contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mu[i] = moments( contours[i], false );
    }

    ///  Get the mass centers:
    vector<pair<Point2f,float>> mc( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ) { 
        mc[i] = make_pair(Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ),mu[i].m00);
    }
    return mc;
}

FeatureExtract::FeatureExtract(Size size, boost::shared_ptr<ImLogger> log) : 
    frame_size(size),
    logger(log),
    bg_sub(100,3,0.1,30*0.5),
    KF(4,2,0)
{
    KF.transitionMatrix = *(Mat_<float>(4, 4) << 
            1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1);

    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-2));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(1e+4));

    randn(KF.statePost, Scalar::all(0), Scalar::all(100));

}


boost::optional<cv::Point2f> FeatureExtract::operator()(cv::Mat& image)
{
    //timer::auto_cpu_timer time_it("%u\n"); 

    cvtColor(image,image,CV_BGR2GRAY);

    Mat mog;

    bg_sub(image,mog);

    logger->log("mog", mog);

    /*
     * MOG that crap
     * Find nearest neighbor to expected location
     * Check what series is correlated to
     * *or* pick nearest neighbor based on what code it is
     */

    auto blobs = getBlobs(mog);

    vector<pair<Point2f,float>> distance(blobs.size());

    Mat prediction = KF.predict();
    Point2f expected = Point2f(prediction(Range(0,2),Range(0,1)));;

    transform(begin(blobs),end(blobs),begin(distance),[&](pair<Point2f,float> x) { return make_pair(x.first,norm(expected - x.first)); } );

    auto nearest = min_element(begin(distance),end(distance),[=](pair<Point2f,float> x,pair<Point2f,float> y) { return x.second < y.second; });

    if(nearest != end(distance)) {
        KF.correct(Mat(nearest->first));
    }

    


    

    return optional<Point2f>(Point2f(KF.statePost(Range(0,2),Range(0,1))));

}

