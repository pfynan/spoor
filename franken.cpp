
#include "franken.h"

#include <arpa/inet.h>


using namespace std;
using namespace cv;
using namespace boost;

using asio::ip::udp;

timer::nanosecond_type const rate_limit = 50L * 1000000L;
timer::nanosecond_type const timeout = 100L * 1000000L;

FrankenConnection::FrankenConnection() 
    : io_service()
    , s(io_service, udp::endpoint(udp::v4(), 1337))
    , resolver(io_service)
    //, endpoint(*resolver.resolve({udp::v4(), "127.0.0.1", "1337"}))
    , endpoint(*resolver.resolve({udp::v4(), "153.106.113.52", "1337"}))
{

    last_message = 0;

}


void FrankenConnection::sendMessage(std::function<void(std::ostream&)> fn) {


    asio::streambuf buf;
    ostream stream(&buf);

    fn(stream);

    const char header = buf.size();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
    buffers.push_back( buf.data() );

    s.send_to(buffers,endpoint);

}

void FrankenConnection::writeGoto(Point2f pp) {

    if(message_timer.elapsed().wall - last_message < rate_limit)
        return;

    last_message = message_timer.elapsed().wall;

    sendMessage([=] (ostream &buf) {
            // Message type
            buf.put(static_cast<char>(MessageType::GOTO));

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

void FrankenConnection::writeIntensity(ushort intens) {
    
    if(message_timer.elapsed().wall - last_message < rate_limit)
        return;

    last_message = message_timer.elapsed().wall;

    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::INTENSITY));

        ushort nbointens = htons(intens);
        buf.write((char*)&nbointens,sizeof(short));
        for (int i = 0; i < 5; ++i)
        {
            buf.put(0);
        }
    });

}

void FrankenConnection::writeOnOff(bool onoff) {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::ONOFF));

        buf.put(onoff);

        for (int i = 0; i < 6; ++i)
        {
            buf.put(0);
        }
    });

}


void FrankenConnection::writeHalt() {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::HALT));

        for (int i = 0; i < 7; ++i)
        {
            buf.put(0);
        }
    });

}

void FrankenConnection::writeSleep() {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::SLEEP));

        for (int i = 0; i < 7; ++i)
        {
            buf.put(0);
        }
    });

}


void FrankenConnection::writeWake() {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::WAKE));

        for (int i = 0; i < 7; ++i)
        {
            buf.put(0);
        }
    });

}

void FrankenConnection::writeCal() {
    
    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::CALIBRATE));

        for (int i = 0; i < 7; ++i)
        {
            buf.put(0);
        }
    });

}

FrankenConnection::Status FrankenConnection::getStatus() {

    sendMessage([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::STATUS));

        for (int i = 0; i < 7; ++i)
        {
        buf.put(0);
        }
        });

    asio::streambuf buf;

    timer::nanosecond_type sent_time = message_timer.elapsed().wall;

    Status status = {Status::LightStatus::OVERHEAT,0,Status::MoveStatus::PAN_FAULT,-1,-1};

    while(s.available() == 0)
        if(message_timer.elapsed().wall - sent_time > timeout) {
            cerr << "Packet timeout" << endl;
            return status;
        }

    const size_t max_buffer = 512;

    size_t n = s.receive_from(buf.prepare(max_buffer),sender_endpoint);


    buf.commit(n);

    istream is(&buf);


    is.read((char*)&status.light, sizeof(status.light));
    is.read((char*)&status.intensity, sizeof(status.intensity));
    is.read((char*)&status.move, sizeof(status.move));

    is.read((char*)&status.current_x, sizeof(status.current_x));
    is.read((char*)&status.current_y, sizeof(status.current_y));

    status.current_x = ntohs(status.current_x);
    status.current_y = ntohs(status.current_y);

    return status;


}


