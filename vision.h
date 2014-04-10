#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include "franken.h"
#include "ImLogger.h"

class Vision
{
public:
    Vision(boost::program_options::variables_map &vm, boost::shared_ptr<FrankenConnection> _franken_conn);
    void run();
    boost::optional<cv::Point2f> getCurPos() {
        boost::lock_guard<boost::mutex> lock(mtx);
        return cur_pos;
    }

private:
    boost::mutex mtx;
    boost::optional<cv::Point2f> cur_pos;
    boost::shared_ptr<FrankenConnection> franken_conn;
    cv::VideoCapture capture;
    boost::shared_ptr<ImLogger> log;
    cv::VideoWriter writer;
    cv::Size frame_size;
};
