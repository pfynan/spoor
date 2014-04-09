
#include "franken.h"

#include <arpa/inet.h>


using namespace std;
using namespace cv;
using namespace boost;

FrankenConnection::FrankenConnection() : io_service(), s(io_service) {
    lock_guard<mutex> lock( mtx ); // Deadlock in constructor?

    using boost::asio::ip::tcp;

    tcp::resolver resolver(io_service);

    system::error_code ec;
    boost::asio::connect(s, resolver.resolve({"127.0.0.1", "9090"}),ec);
    if(ec) {
        cerr << "Could not connect to light" << endl;
    }
}


void FrankenConnection::sendMessage(std::function<void(std::ostream&)> fn) {

    lock_guard<mutex> lock( mtx );
    asio::streambuf buf;
    ostream stream(&buf);


    fn(stream);

    const char header = buf.size();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
    buffers.push_back( buf.data() );

    boost::asio::write(s,buffers);

}

void FrankenConnection::writeSetTargetAngle(Point2f pp) {

    sendMessage([=] (ostream &buf) {
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

    });

}

void FrankenConnection::writeLightIntensity(ushort intens) {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::SET_LIGHT_INTENSITY));

        ushort nbointens = htons(intens);
        buf.write((char*)&nbointens,sizeof(short));
        for (int i = 0; i < 5; ++i)
        {
            buf.put(0);
        }
    });

}

void FrankenConnection::writeLightOnOff(bool onoff) {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::SET_LIGHT_ONOFF));

        buf.put(onoff);

        for (int i = 0; i < 6; ++i)
        {
            buf.put(0);
        }
    });

}

