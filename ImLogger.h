#pragma once

#include <string>
#include <map>

#include <cv.h>
#include <highgui.h>

class ImLogger {
    public:
        ImLogger(std::map<std::string,std::string> hooks,double fps,cv::Size frame_size);

        void log(std::string hook,cv::Mat image);
    private:
        std::map<std::string,cv::VideoWriter> hooks;
        cv::Size frame_size;
};

