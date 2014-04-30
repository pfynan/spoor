
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
    void writeGoto(cv::Point2f pp);
    void writeIntensity(ushort intens);
    void writeOnOff(bool onoff);
    void writeHalt();
    void writeSleep();
    void writeWake();
    void writeCal();

    struct Status {
        enum class LightStatus : char {
            OVERHEAT = 'H',
            ON = 'O',
            OFF = 'F'
        };

        LightStatus light;

        char intensity;

        enum class MoveStatus : char {
            RUNNING = 'R',
            UNCAL = 'U',
            CALING = 'C',
            SLEEPING = 'S',
            TILT_OVERCUR = 'O',
            TILT_FAULT = 'F',
            PAN_OVERCUR = 'o',
            PAN_FAULT = 'f'
        };

        MoveStatus move;

        short current_x;
        short current_y;

    };

    Status getStatus();

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket s;
    boost::mutex mtx;
    void sendMessage(std::function<void(std::ostream&)> fn);
    
enum class MessageType : char
    { GOTO = 'G'
    , ONOFF = 'O'
    , HALT = 'H'
    , SLEEP = 'S'
    , WAKE = 'W'
    , INTENSITY = 'I'
    , STATUS = 'T'
    , CALIBRATE = 'C'
    };
};
