
#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>


#ifdef NO_OPENCV

namespace cv {

    class Point2f {
        public:
            float x,y;
            Point2f(float _x, float _y) {
                x = _x;
                y = _y;
            }
    };
}

#else

#include <cv.h>

#endif


class FrankenConnection
{
public:
    FrankenConnection ();
    void writeSetTargetAngle(cv::Point2f pp);
    void writeLightIntensity(ushort intens);
    void writeLightOnOff(bool onoff);

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket s;
    boost::mutex mtx;
    void sendMessage(std::function<void(std::ostream&)> fn);
    
enum class MessageType
    { SET_TARGET_ANGLE = 1
    , SET_LIGHT_INTENSITY = 2
    , SET_LIGHT_ONOFF = 3
    };
};
