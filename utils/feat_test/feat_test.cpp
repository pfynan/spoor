#include <fstream>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <ml.h>

#include <thread>
#include <queue>


#include <boost/timer/timer.hpp>
#include <boost/circular_buffer.hpp>
using namespace std;
using namespace cv;
using namespace boost;

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

inline void cross(cv::Mat& img,cv::Point pt,int size=10) {
    using namespace cv;


    Scalar color = Scalar(128,0,128);
    int thick = 3;
    line(img,pt - Point(size/2,size/2),pt + Point(size/2,size/2),color,thick);
    line(img,pt - Point(-size/2,size/2),pt + Point(-size/2,size/2),color,thick);
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

template<class T>
void dump(string fn, T d) {
    ofstream os(fn);
    for (int i = 0; i < d.size(); ++i)
    {
        os << d[i] << endl;
    }
    os.close();
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

    setIdentity(KF.processNoiseCov);
    KF.processNoiseCov *= 144;


    setIdentity(KF.measurementNoiseCov, Scalar::all(9));
    setIdentity(KF.errorCovPost, Scalar::all(1e+4));


    KF.statePost = *(Mat_<float>(4,1) << 640/2, 480/2, 0, 0);



    circular_buffer<bool> was_there(15), expected_there(15);

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


    vector<float> masslog,distlog,hammlog,hu0log;

    bool has_lock = false;
    
    int frames = 0;

    int track_lost = 0;
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

        //dilate(im,im,getStructuringElement(MORPH_ELLIPSE,Size(7,7)));
        morphologyEx(im,im,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));

        threshold(im,im,200,255,THRESH_BINARY);

        Mat i2;

        im.copyTo(i2);

        if(has_lock) {
            KF.predict();
        }


        auto blobs = getBlobs(i2);

        vector<pair<Point2f,Moments>> mags;

        Mat iobs_cov = KF.measurementMatrix * KF.errorCovPost * KF.measurementMatrix.t() + KF.measurementNoiseCov;
        invert(iobs_cov,iobs_cov,DECOMP_SVD);

        for (int i = 0; i < blobs.size(); ++i)
        {
            masslog.push_back(blobs[i].second.m00);
            float mahal = Mahalanobis(Mat(blobs[i].first),KF.statePost(Range(0,2),Range(0,1)),iobs_cov);
            if(has_lock) distlog.push_back(mahal);
            vector<double> humoments;
            HuMoments(blobs[i].second,humoments);

            hu0log.push_back(humoments[0]);

            if(!(blobs[i].second.m00 > 16 && blobs[i].second.m00 < 160)) continue;
            if(humoments[0] > 0.3) continue;
            if(has_lock && mahal > 2) continue;
            mags.push_back(blobs[i]);

        }

        auto nearest = min_element(begin(mags),end(mags),[=](pair<Point2f,Moments> x,pair<Point2f,Moments> y) { return x.second.m00 < y.second.m00; });
        

        if(nearest != end(mags)) {
            KF.correct(Mat(nearest->first));
        }

        was_there.push_back(mags.empty() ? false : true);

        int hamm = circ_hamm_dist(was_there,expected_there);

        hammlog.push_back(hamm);

        if(hamm > 6) {
            ++track_lost;
            has_lock = false;
            Point2f estimated = Point2f(640/2,480/2);
            cross(image,estimated);
            cross(im,estimated);
        }
        else {
            has_lock = true;
            Point2f estimated = Point2f(KF.statePost(Range(0,2),Range(0,1)));;

            cross(image,estimated);
            cross(im,estimated);
        }



        imshow("out",image);
        waitKey(1);


        
    ++frames;


    }

    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;

    cout << "Lost beacon " << track_lost << " times \n";


    dump("distlog.txt",distlog);
    dump("masslog.txt",masslog);
    dump("hammlog.txt",hammlog);
    dump("hu0log.txt",hu0log);


    cout << "Kalman" << endl << KF.errorCovPost << endl << endl;


    return 0;
}

