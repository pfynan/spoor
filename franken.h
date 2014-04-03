
#pragma once

#include <iostream>



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

enum class MessageType
    { SET_TARGET_ANGLE = 1
    , SET_LIGHT_INTENSITY = 2
    , SET_LIGHT_ONOFF = 3
    };

void writeSetTargetAngle(std::ostream &buf, cv::Point2f pp);
void writeLightIntensity(std::ostream &buf, ushort intens);
void writeLightOnOff(std::ostream &buf, bool onoff);

