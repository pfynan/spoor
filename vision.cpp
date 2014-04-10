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

using namespace std;
using namespace cv;
using namespace boost;

namespace po = boost::program_options;

Vision::Vision(boost::program_options::variables_map &vm, boost::shared_ptr<FrankenConnection> _franken_conn) {
    franken_conn = _franken_conn;
    string outfile = "out.avi";


    capture.open(vm["in"].as<string>());

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

}

void Vision::run() {
    Mat image;
    namedWindow("Out",1);

    FeatureExtract fe(frame_size,log);
    Tracking tr;



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

        {
            lock_guard<mutex> lck(mtx);
            cur_pos = fp;
        }

        franken_conn->writeSetTargetAngle(pp);


        
        writer << disp;
        frames++;

    }

    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;

}

