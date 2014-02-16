#include <iostream>

#include <cv.h>
#include <highgui.h>


#include "utility.h"
#include "feature.h"
#include "tracking.h"

#include "cvplot.hpp"

#include <boost/asio.hpp>

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


    Size frame_size = Size( capture.get(CV_CAP_PROP_FRAME_WIDTH)
                          , capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    

    VideoWriter writer;
        
    if(!interactive) {    
        writer.open
            ( "out.avi"
            , CV_FOURCC('F','M','P','4')
            , capture.get(CV_CAP_PROP_FPS)
            , frame_size);
    }

    Mat image;
    namedWindow("Out",1);

    FeatureExtract fe(frame_size);
    Tracking tr;

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"127.0.0.1", "9090"}));


    while(capture >> image, !image.empty()) {
        

        Mat disp;
        image.copyTo(disp);
       
        optional<Point2f> fp = fe(image);
        Point2f pp = tr(fp);

        cross( disp , pp );
        

            //imshow("Out",disp);
            //if(waitKey(1) == 27) break;
        


        char net_buffer[6];

        net_buffer[0] = 5;
        net_buffer[1] = 1;
        *((short*)(net_buffer + 2)) = htons((int)pp.x);
        *((short*)(net_buffer + 4)) = htons((int)pp.y);

        

        boost::asio::write(s,boost::asio::buffer(net_buffer,6));

        writer << disp;

        if(interactive) {
            imshow("Out",disp);
            if(waitKey(1) == 27) break;
        }
    }


    return 0;
}


