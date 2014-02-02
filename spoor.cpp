#include <iostream>

#include <cv.h>
#include <highgui.h>


#include "utility.h"
#include "feature.h"
#include "tracking.h"

using namespace std;
using namespace cv;
using namespace boost;

RNG rng(12345);



int main(int argc,char *argv[]) {

    bool interactive = false;

    int capture_id = 0;

    if(argc >= 2) {
        if(string(argv[1]) == "-i")
            interactive = true;
        if(argc >= 3)
            capture_id = atoi(argv[2]);
    }

    VideoCapture capture;
    
    if(interactive)
        capture.open(capture_id);
    else
        capture.open("test.mkv");

    if(!capture.isOpened()) {
        cerr << "Can't open input" << endl;
        exit(1);
    }


    
    

    VideoWriter writer;
        
    if(!interactive) {    
        writer.open
            ( "out.avi"
            , CV_FOURCC('F','M','P','4')
            , capture.get(CV_CAP_PROP_FPS)
            , Size
                ( capture.get(CV_CAP_PROP_FRAME_WIDTH)
                , capture.get(CV_CAP_PROP_FRAME_HEIGHT)));
    }

    Mat image;
    namedWindow("Out",1);


    FeatureExtract fe;
    Tracking tr;

    while(capture >> image, !image.empty()) {


        Mat disp;
        image.copyTo(disp);

       
        optional<Point2f> fp = fe(image);
        //Point2f pp = tr(fp);

        //cross( disp , pp );
        

            //imshow("Out",disp);
            //if(waitKey(1) == 27) break;

        cvtColor(image,image,CV_GRAY2BGR);
        
        writer << image;

        if(interactive) {
            imshow("Out",image);
            if(waitKey(1) == 27) break;
        }
    }


    return 0;
}


