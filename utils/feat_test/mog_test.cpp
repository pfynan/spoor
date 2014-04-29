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

    vector<float> sizes;
    
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


        auto blobs = getBlobs(i2);


        vector<pair<Point2f,float>> distance;
        transform(begin(blobs),end(blobs),back_inserter(distance),[&](pair<Point2f,float> x) { return make_pair(x.first,abs(x.second - 80)); } );

        auto nearest = min_element(begin(distance),end(distance),[=](pair<Point2f,float> x,pair<Point2f,float> y) { return x.second < y.second; });

        transform(begin(blobs),end(blobs),back_inserter(sizes),[] (pair<Point2f,float> x) {return x.second;} );
        

        if(nearest != end(distance) && nearest->second < sqrt(230) * 2) {
            //sizes.push_back(nearest->second);
            cross(image,nearest->first);
            cross(im,nearest->first);
        }


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

    Mat tr;

    transpose(sizes,tr);
    em.train(tr);

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

