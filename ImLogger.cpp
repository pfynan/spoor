#include <iostream>
#include "ImLogger.h"

using namespace std;
using namespace cv;

ImLogger::ImLogger(std::map<std::string,std::string> to_hook,double fps,cv::Size frame_size) : frame_size(frame_size) {
    for(auto x : to_hook) {
        VideoWriter wr;
        wr.open
            ( x.second
              , CV_FOURCC('F','M','P','4')
              , fps
              , frame_size);
        hooks.insert(make_pair(x.first,wr));
    }

}

void ImLogger::log(std::string hook,cv::Mat image) {
    auto iter = hooks.find(hook);

    if(iter != hooks.end() && iter->second.isOpened()) {
        Mat wr = image.clone();
        if(wr.channels() == 1) {
            if(wr.size() != frame_size)
                resize(wr,wr,frame_size);

            if(wr.depth() == CV_32F) {
                double minval,maxval;
                minMaxLoc(wr,&minval,&maxval);
                wr *= 255;
                wr.convertTo(wr,CV_8UC1);
                putText(wr,to_string(minval),Point(0,40),FONT_HERSHEY_SCRIPT_SIMPLEX,1,Scalar::all(255),1,8,false);
                putText(wr,to_string(maxval),Point(0,80),FONT_HERSHEY_SCRIPT_SIMPLEX,1,Scalar::all(255),1,8,false);
            }

            cvtColor(wr,wr,CV_GRAY2BGR);
        }

        iter->second << wr;
    }
}
