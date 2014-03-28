#include <iostream>

#include <cv.h>
#include <highgui.h>


#include "utility.h"
#include "feature.h"
#include "tracking.h"

#include "ImLogger.h"

//#include "cvplot.hpp"

#include <boost/asio.hpp>
#include <boost/timer/timer.hpp>

#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
using namespace boost;

namespace po = boost::program_options;


RNG rng(12345);


int main(int argc,char *argv[]) {

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("log", po::value<vector<string>>(), "log hook")
        ("log-file", po::value<vector<string>>(), "log file")
        ("in", po::value<string>()->default_value("test.mkv"), "input file")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }


    
    



    string outfile = "out.avi";


    VideoCapture capture;
    
    capture.open(vm["in"].as<string>());

    if(!capture.isOpened()) {
        cerr << "Can't open input" << endl;
        exit(1);
    }


    Size frame_size = Size( capture.get(CV_CAP_PROP_FRAME_WIDTH)
                          , capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    
    map<string,string> logmap;

    vector<string> hooks;
    vector<string> logfiles;

    if(vm.count("log"))
        hooks = vm["log"].as<vector<string>>();

    if(vm.count("log-file"))
        logfiles = vm["log-file"].as<vector<string>>();

    for (int i = 0; i < hooks.size(); ++i)
    {
        if(i < logfiles.size())
            logmap.insert(make_pair(hooks[i],logfiles[i]));
        else
            logmap.insert(make_pair(hooks[i],hooks[i] + ".avi"));
    }
          
    ImLogger log(logmap,capture.get(CV_CAP_PROP_FPS),frame_size);

    VideoWriter writer;
        
    writer.open
        ( outfile
          , CV_FOURCC('F','M','P','4')
          , capture.get(CV_CAP_PROP_FPS)
          , frame_size);

    Mat image;
    namedWindow("Out",1);

    FeatureExtract fe(frame_size,log);
    Tracking tr;

    /*
    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"127.0.0.1", "9090"}));
    */

    int frames = 0;

    //timer::cpu_timer

    while(capture >> image, !image.empty()) {
        

        Mat disp;
        image.copyTo(disp);
        optional<Point2f> fp = fe(image);
        Point2f pp = tr(fp);

        cross( disp , pp );
        

            //imshow("Out",disp);
            //if(waitKey(1) == 27) break;
        

/*
        char net_buffer[6];

        net_buffer[0] = 5;
        net_buffer[1] = 1;
        *((short*)(net_buffer + 2)) = htons((int)pp.x);
        *((short*)(net_buffer + 4)) = htons((int)pp.y);

        

        boost::asio::write(s,boost::asio::buffer(net_buffer,6));
        */

        
        //cvtColor(image,image,CV_GRAY2BGR);
        writer << disp;
        frames++;

    }


    return 0;
}


