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


    Scalar color = Scalar(0,0,255);
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
    setIdentity(KF.processNoiseCov, Scalar::all(1e-2));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(1e+4));

    randn(KF.statePost, Scalar::all(0), Scalar::all(100));


    vector<float> logs;
    
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

        GaussianBlur(im,im,Size(7,7),0);

        threshold(im,im,220,255,THRESH_BINARY);

        Mat i2;

        im.copyTo(i2);

        Mat prediction = KF.predict();
        Point2f expected = Point2f(prediction(Range(0,2),Range(0,1)));;

        auto blobs = getBlobs(i2);


        vector<pair<Point2f,float>> masses,distances;
        transform(begin(blobs),end(blobs),back_inserter(masses),[&](pair<Point2f,float> x) { return make_pair(x.first,abs(x.second - 80)); } );

        //transform(begin(blobs),end(blobs),back_inserter(distances),[&](pair<Point2f,float> x) { return make_pair(x.first,norm(x.first - expected)); } );

        vector<pair<Point2f,float>> scores;

        //transform(begin(masses),end(masses),begin(distances),back_inserter(scores),[&](pair<Point2f,float> x,pair<Point2f,float> y) { return make_pair(x.first,norm(Point2f(x.second,y.second))); } );

        auto nearest = min_element(begin(masses),end(masses),[=](pair<Point2f,float> x,pair<Point2f,float> y) { return x.second < y.second; });

        //transform(begin(blobs),end(blobs),back_inserter(sizes),[] (pair<Point2f,float> x) {return x.second;} );
        

        if(nearest != end(scores) && nearest->second < sqrt(230) * 2) {
            cross(image,nearest->first);
            cross(im,nearest->first);
            KF.correct(Mat(nearest->first));
        }

        if(nearest != end(scores))
            logs.push_back(nearest->second);


        imshow("out",im);
        waitKey(25);

        //image.convertTo(image,CV_32FC1);

        //image = image / 255;
        
        //im = 10 * image - 9;
        
    ++frames;


    }

    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;

    EM em(2);


    ofstream os("log.txt");

    for (int i = 0; i < logs.size(); ++i)
    {
        os << logs[i] << endl;
    }

    os.close();


    Mat tr(logs);


    em.train(tr.t());

    Mat m = em.get<Mat>("means");
    Mat w = em.get<Mat>("weights");
    vector<Mat> c = em.get<vector<Mat>>("covs");
    //cv::sort(m,m,CV_SORT_EVERY_COLUMN | CV_SORT_ASCENDING);

    for (int i = 0; i < m.rows; ++i)
    {
        cout << w.at<double>(0,i) << "\t" << m.at<double>(i,0) << "\t" << c[i] << "\n";
    }

    return 0;
}

