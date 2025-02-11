
#include "franken.h"

#include <arpa/inet.h>


using namespace std;
using namespace cv;
using namespace boost;

using asio::ip::tcp;

timer::nanosecond_type const rate_limit = 50L * 1000000L;

FrankenConnection::FrankenConnection() : io_service(), resolver(io_service) {
    lock_guard<mutex> lock( mtx ); // Deadlock in constructor?

    last_message = 0;

    //resolved =  resolver.resolve({"127.0.0.1", "8080"});
    resolved =  resolver.resolve({"153.106.113.49", "80"});

}



void FrankenConnection::sendMessage(std::function<void(std::ostream&)> fn) {

    lock_guard<mutex> lock( mtx );



    asio::streambuf buf;
    ostream stream(&buf);

    tcp::socket s(io_service);
    asio::connect(s,resolved);

    fn(stream);

    const char header = buf.size();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
    buffers.push_back( buf.data() );

    boost::asio::async_write(s,buffers,[](boost::system::error_code ec, std::size_t length){});

    s.close();


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

    auto fn = ([=] (ostream &buf) {
        // Message type
        buf.put(static_cast<char>(MessageType::STATUS));

        for (int i = 0; i < 7; ++i)
        {
        buf.put(0);
        }
        });

    lock_guard<mutex> lock( mtx );
    asio::streambuf buf;
    ostream stream(&buf);

    tcp::socket s(io_service);
    asio::connect(s,resolved);

    fn(stream);

    const char header = buf.size();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
    buffers.push_back( buf.data() );

    boost::asio::write(s,buffers);


    char length;

    asio::read(s,asio::buffer(&length, sizeof(length)));

    asio::streambuf buf2;

    size_t n = asio::read(s,buf2.prepare(length));

    if(n != length)
        cerr << "Someone goofed..." << endl;

    buf2.commit(n);

    istream is(&buf2);

    Status status;

    is.read((char*)&status.light, sizeof(status.light));
    is.read((char*)&status.intensity, sizeof(status.intensity));
    is.read((char*)&status.move, sizeof(status.move));

    is.read((char*)&status.current_x, sizeof(status.current_x));
    is.read((char*)&status.current_y, sizeof(status.current_y));

    status.current_x = ntohs(status.current_x);
    status.current_y = ntohs(status.current_y);

    s.close();

    return status;


}


