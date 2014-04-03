
#include "franken.h"

#include <arpa/inet.h>

using namespace std;
using namespace cv;

void writeSetTargetAngle(ostream &buf, Point2f pp) {
    
        // Message type
        buf.put(static_cast<char>(MessageType::SET_TARGET_ANGLE));

        // X and Y coordinates
        short x = htons((int)pp.x);
        short y = htons((int)pp.y);
        buf.write((char*)&x,sizeof(short));
        buf.write((char*)&y,sizeof(short));

        for (int i = 0; i < 2; ++i)
        {
            buf.put(0);
        }

}

void writeLightIntensity(ostream &buf, ushort intens) {
    
        // Message type
        buf.put(static_cast<char>(MessageType::SET_LIGHT_INTENSITY));

        ushort nbointens = htons(intens);
        buf.write((char*)&nbointens,sizeof(short));
        for (int i = 0; i < 5; ++i)
        {
            buf.put(0);
        }

}

void writeLightOnOff(ostream &buf, bool onoff) {
    
        // Message type
        buf.put(static_cast<char>(MessageType::SET_LIGHT_ONOFF));

        buf.put(onoff);

        for (int i = 0; i < 6; ++i)
        {
            buf.put(0);
        }

}

