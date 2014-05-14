#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>

#include <iostream>

#include <cv.h>
#include <highgui.h>

#include "vision.h"

#include "ImLogger.h"
#include "feature.h"
#include "franken.h"

#include "cap_gstreamer.h"

using namespace std;
using namespace cv;
using namespace boost;

namespace po = boost::program_options;

void cross(cv::Mat& img,cv::Point pt,int size=10) {
    using namespace cv;


    Scalar color = Scalar(0,0,255);
    int thick = 3;
    line(img,pt - Point(size/2,size/2),pt + Point(size/2,size/2),color,thick);
    line(img,pt - Point(-size/2,size/2),pt + Point(-size/2,size/2),color,thick);
}

//FIXME

Vision::Vision(boost::program_options::variables_map &vm, boost::shared_ptr<FrankenConnection> _franken_conn) : engaged(true) {
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

    if(!cap.open(3,
        "udpsrc port=4000 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)JPEG, payload=(int)96\" ! "
        "rtpjpegdepay ! jpegdec ! ffmpegcolorspace ! queue ! appsink"
                ))
        cerr << "Failed to open" << endl;

    cap.setProperty(CV_CAP_GSTREAMER_QUEUE_LENGTH,2);


    CvVideoWriter_GStreamer out;

    if(!out.open(
                "appsrc ! "
                "queue ! "
                "ffmpegcolorspace ! "
                "videorate ! "
                "video/x-raw-yuv,framerate=15/1 ! "
                "jpegenc ! "
                "rtpjpegpay ! "
                "udpsink host=127.0.0.1 port=5000"
                ,15,Size(640,480)))
        cerr << "Failed to open output" << endl;


    FeatureExtract fe(Size(640,480),log);



    int frames = 0;

    timer::cpu_timer timer;
    auto last_frame = timer.elapsed().wall;
    Mat image;
    while(1) {
        if(!cap.grabFrame())
            break;

        GstClockTime tstamp;

        IplImage *iplim = cap.retrieveFrame(0,&tstamp);
        if(!iplim)
            break;

        image = Mat(iplim);

        if(image.empty())
            break;
        
        timer::nanosecond_type const proc_start(timer.elapsed().wall);

        Mat disp;
        image.copyTo(disp);

        cvtColor(disp, disp, CV_BGR2GRAY);
        GaussianBlur(disp,disp,Size(5,5),0);
        cvtColor(disp, disp, CV_GRAY2BGR);
        
        optional<Point2f> fp = fe(image);

        if(fp) {
            Point2f pp = *fp;
            cross(disp, pp);

            Mat norm_pos =  Mat::diag(Mat(Point2f(1./640.,1./480.))) * Mat(pp);

            Point2f upleft(0x0090,0x1c10), botright(0x0140,0x1600);

            Mat spot_zero = Mat(upleft);

            Mat spot_scale = Mat::diag(Mat(botright - upleft));

            Mat spot_coords = spot_scale * norm_pos + spot_zero;

            {
		
                lock_guard<mutex> lck(mtx);
                cur_pos = fp;
                if(engaged)
                    franken_conn->writeGoto(Point2f(spot_coords));
                // NOTE: I smell a deadlock...
            }

        }
	
        timer::nanosecond_type const elapsed(timer.elapsed().wall);

        putText(disp,to_string(1e-6*(double)(elapsed-last_frame)),Point(550,440),FONT_HERSHEY_SIMPLEX,1,Scalar(0,200,0),1,8,false);
        putText(disp,to_string(1e-6*(double)(elapsed-proc_start)),Point(550,340),FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,200),1,8,false);
        last_frame = elapsed;
       
        //writer << disp;
        IplImage oframe = IplImage(disp);
        out.writeFrame(&oframe,tstamp);
        frames++;
    }


    timer::cpu_times const elapsed(timer.elapsed());
    timer::nanosecond_type const wall(elapsed.wall);

    double fps = (double) frames / ((double) wall * 1e-9);

    cout << fps << " fps" << endl;

    //cap.close();
    //out.close();


}

