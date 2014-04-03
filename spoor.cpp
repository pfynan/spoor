#include <iostream>

#include <cv.h>
#include <highgui.h>


#include "utility.h"
#include "feature.h"
#include "tracking.h"

#include "ImLogger.h"

//#include "cvplot.hpp"

#include "franken.h"

#include "thrift.h"

#include <boost/asio.hpp>
#include <boost/timer/timer.hpp>

#include <boost/program_options.hpp>

#include <boost/thread.hpp>

using namespace std;
using namespace cv;
using namespace boost;

namespace po = boost::program_options;


RNG rng(12345);

/*
 * Threads:
 *
 * Thrift  -|--| 
 * Franken -|--| // Next
 * Processing--|-|  // Wait
 * Gstreamer-----|  // Wait
 *
 * ...
 *
 * Jesus Christ almighty!
 *
 */





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

    
    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);

    system::error_code ec;
    boost::asio::connect(s, resolver.resolve({"127.0.0.1", "9090"}),ec);
    if(ec) {
        cerr << "Could not connect to light" << endl;
    }

    

    thread thrift_thread(thriftThread);




    int frames = 0;

    timer::cpu_timer timer;
    while(capture >> image, !image.empty()) {
        

        Mat disp;
        image.copyTo(disp);
        optional<Point2f> fp = fe(image);
        Point2f pp = tr(fp);

        if(fp)
            cross(disp, *fp);

        cross( disp , pp );
        

        asio::streambuf buf;
        ostream stream(&buf);


        writeSetTargetAngle(stream,pp);
        

        const char header = buf.size();

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
        buffers.push_back( buf.data() );
        boost::asio::write(s,buffers,ec);

        
        writer << disp;
        frames++;

    }

    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;


    return 0;
}


