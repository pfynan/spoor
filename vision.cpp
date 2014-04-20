#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>

#include <iostream>

#include <cv.h>
#include <highgui.h>

#include "vision.h"

#include "ImLogger.h"
#include "utility.h"
#include "feature.h"
#include "tracking.h"
#include "franken.h"

#include "cap_gstreamer.h"

using namespace std;
using namespace cv;
using namespace boost;

namespace po = boost::program_options;

Vision::Vision(boost::program_options::variables_map &vm, boost::shared_ptr<FrankenConnection> _franken_conn) {
    franken_conn = _franken_conn;
    string outfile = "out.avi";


/*    capture.open(vm["in"].as<string>());

    if(!capture.isOpened()) {
        cerr << "Can't open input" << endl;
        exit(1);
    }


    frame_size = Size( capture.get(CV_CAP_PROP_FRAME_WIDTH)
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
          
    log = boost::shared_ptr<ImLogger>(new ImLogger(logmap,capture.get(CV_CAP_PROP_FPS),frame_size));

    writer.open
        ( outfile
          , CV_FOURCC('F','M','P','4')
          , capture.get(CV_CAP_PROP_FPS)
          , frame_size);
    */
    log = boost::shared_ptr<ImLogger>(new ImLogger(std::map<string,string>(),25,Size(0,0)));

}

void Vision::run() {

    CvCapture_GStreamer cap;

    if(!cap.open(3,"video/low1.avi"))
        cerr << "Failed to open" << endl;

    CvVideoWriter_GStreamer out;

    if(!out.open("out.avi",CV_FOURCC('M','P','2','V'),25,Size(640,480),true))
        cerr << "Failed to open output" << endl;


    FeatureExtract fe(Size(640,480),log);
    Tracking tr;



    int frames = 0;

    timer::cpu_timer timer;
    Mat image;
    while(1) {
        if(!cap.grabFrame())
            break;

        image = Mat(cap.retrieveFrame(0));
        if(image.empty())
            break;
        

        Mat disp;
        image.copyTo(disp);
        optional<Point2f> fp = fe(image);
        Point2f pp = tr(fp);

        if(fp)
            cross(disp, *fp);

        cross( disp , pp );

        {
            lock_guard<mutex> lck(mtx);
            cur_pos = fp;
        }

        franken_conn->writeSetTargetAngle(pp);

        
        //writer << disp;
        IplImage oframe = IplImage(disp);
        out.writeFrame(&oframe);
        frames++;

    }


    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;

    //cap.close();
    //out.close();


}

