#include "tracking.h"

using namespace boost;
using namespace cv;

Tracking::Tracking() : KF(4, 2, 0)
{
    KF.transitionMatrix = *(Mat_<float>(4, 4) << 
            1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1);

    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(1e+4));

    randn(KF.statePost, Scalar::all(0), Scalar::all(100));
}

Point2f Tracking::operator()(optional<Point2f> measurement)
{
    Mat prediction = KF.predict();
    if(measurement)
        KF.correct(Mat(*measurement));
    return Point2f(KF.statePost(Range(0,2),Range(0,1)));
}

