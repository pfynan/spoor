#include <iostream>

#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

RNG rng(12345);



int main() {

    VideoCapture capture("test.mp4");
    VideoWriter 
      writer( "out.avi"
            , CV_FOURCC('X','V','I','D')
            , capture.get(CV_CAP_PROP_FPS)
            , Size
                ( capture.get(CV_CAP_PROP_FRAME_WIDTH)
                , capture.get(CV_CAP_PROP_FRAME_HEIGHT)));

    Mat image;
    //namedWindow("Out",1);

    KalmanFilter KF(4, 2, 0);
    Mat state(4, 1, CV_32F); /* (phi, delta_phi) */
    //Mat processNoise(2, 1, CV_32F);
    Mat measurement = Mat::zeros(2, 1, CV_32F);

    randn( state, Scalar::all(0), Scalar::all(0.1) );
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


    while(capture >> image, !image.empty()) {

        Mat prediction = KF.predict();

        Mat disp;

        
        Point2f pp = Point(prediction.at<float>(0),prediction.at<float>(1));

        image.copyTo(disp);
        circle( disp , pp, 2, Scalar(255,0,0) , -1, 8, 0 );
        

        cvtColor(image,image,CV_BGR2GRAY);
        normalize(image,image, 0, 255, NORM_MINMAX, CV_8UC1);


        adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,21,-180);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

        vector<Moments> mu(contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ) { 
            mu[i] = moments( contours[i], false );
        }

        ///  Get the mass centers:
        vector<Point2f> mc( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ) { 
            mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
        }

        for( size_t  i = 0; i< contours.size(); i++ )
        {
            //drawContours( disp, contours, i, color, 2, 8, hierarchy, 0, Point() );
        }

        if(contours.size() && (mu[0].m00 > 0.1)) {
            measurement.at<float>(0) = mc[0].x;
            measurement.at<float>(1) = mc[0].y;

            KF.correct(measurement);
        }

        //imshow("Out",disp);
        
        writer << disp;

        //if(waitKey(33) == 27) break;
    }


    return 0;
}
