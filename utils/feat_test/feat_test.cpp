#include <fstream>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <ml.h>

#include <thread>
#include <queue>


#include <boost/timer/timer.hpp>
using namespace std;
using namespace cv;
using namespace boost;

vector<pair<Point2f,float>> getBlobs(Mat &image) {
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

    vector<pair<Point2f,float>> ret(contours.size());

    for( size_t i = 0; i < contours.size(); i++ ) { 
        ret[i] = make_pair(mc[i],mu[i].m00);
    }

    return ret;
}

inline void cross(cv::Mat& img,cv::Point pt,int size=10) {
    using namespace cv;


    Scalar color = Scalar(128,0,128);
    int thick = 3;
    line(img,pt - Point(size/2,size/2),pt + Point(size/2,size/2),color,thick);
    line(img,pt - Point(-size/2,size/2),pt + Point(-size/2,size/2),color,thick);
}

int main(int argc, char *argv[])
{

    if(argc != 2) { 
        cerr << "Not enough arguments" << endl;
        exit(1);
    }

    VideoCapture capture(argv[1]);

    if(!capture.isOpened()) {
        cerr << "Can't open input" << endl;
        exit(1);
    }


    KalmanFilter KF(4,2,0);

    KF.transitionMatrix = *(Mat_<float>(4, 4) << 
            1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1);

    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(100));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    //setIdentity(KF.errorCovPost, Scalar::all(1e+4));

    KF.errorCovPost = *(Mat_<float>(4,4) <<
62231212, 0, 758041.62, 0,
  0, 62231212, 0, 758041.62,
  758041.62, 0, 12363.456, 0,
  0, 758041.62, 0, 12363.456);


    randn(KF.statePost, Scalar::all(0), Scalar::all(100));

    Mat iobs_cov = KF.measurementMatrix * KF.errorCovPost * KF.measurementMatrix.t() + KF.measurementNoiseCov;
    invert(iobs_cov,iobs_cov,DECOMP_SVD);

    vector<float> masslog,distlog;
    
    int frames = 0;

    timer::cpu_timer timer;
    
    Mat image;

    while(capture >> image, !image.empty()) {

        if(!capture.isOpened()) {
            break;
        }

        cvtColor(image,image,CV_BGR2GRAY);

        Mat im;

        image.copyTo(im);

        GaussianBlur(im,im,Size(9,9),0);

        dilate(im,im,getStructuringElement(MORPH_ELLIPSE,Size(7,7)));
        //morphologyEx(im,im,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(21,21)));

        threshold(im,im,200,255,THRESH_BINARY);

        Mat i2;

        im.copyTo(i2);

        Mat prediction = KF.predict();
        Point2f expected = Point2f(prediction(Range(0,2),Range(0,1)));;

        auto blobs = getBlobs(i2);

        vector<pair<Point2f,float>> mags;

        for (int i = 0; i < blobs.size(); ++i)
        {
            masslog.push_back(blobs[i].second);
            float mahal = Mahalanobis(Mat(blobs[i].first),KF.statePost(Range(0,2),Range(0,1)),iobs_cov);
            distlog.push_back(mahal);
            if(!(blobs[i].second > 61 && blobs[i].second < 295)) continue;
            if(mahal > 1) continue;
            mags.push_back(blobs[i]);

        }

        auto nearest = min_element(begin(mags),end(mags),[=](pair<Point2f,float> x,pair<Point2f,float> y) { return x.second < y.second; });
        

        if(nearest != end(mags)) {
            KF.correct(Mat(nearest->first));
        }

        Point2f estimated = Point2f(KF.statePost(Range(0,2),Range(0,1)));;

        cross(image,estimated);
        cross(im,estimated);



        //imshow("out",im);
        //waitKey(1);


        
    ++frames;


    }

    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;



    ofstream os("distlog.txt");
    for (int i = 0; i < distlog.size(); ++i)
    {
        os << distlog[i] << endl;
    }
    os.close();

    ofstream os2("masslog.txt");
    for (int i = 0; i < masslog.size(); ++i)
    {
        os2 << masslog[i] << endl;
    }
    os2.close();

    cout << "Kalman" << endl << KF.errorCovPost << endl << endl;


    Mat tr = Mat(distlog).reshape(1);

    EM em(1);

    em.train(tr);

    Mat m = em.get<Mat>("means");
    Mat w = em.get<Mat>("weights");
    vector<Mat> c = em.get<vector<Mat>>("covs");
    //cv::sort(m,m,CV_SORT_EVERY_COLUMN | CV_SORT_ASCENDING);

    for (int i = 0; i < m.rows; ++i)
    {
        cout << w.at<double>(0,i) << "\t" << m.row(i) << "\t" << c[i] << "\n";
    }

    return 0;
}

