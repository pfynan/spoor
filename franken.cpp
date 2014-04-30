
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

void FrankenConnection::writeGoto(Point2f pp) {

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
    

   char length;

   asio::read(s,asio::buffer(&length, sizeof(length)));

    asio::streambuf buf;

    size_t n = asio::read(s,buf.prepare(length));

    if(n != length)
        cerr << "Someone goofed..." << endl;

    buf.commit(n);

    istream is(&buf);

    Status status;

    is.read((char*)&status.light, sizeof(status.light));
    is.read((char*)&status.intensity, sizeof(status.intensity));
    is.read((char*)&status.move, sizeof(status.move));

    is.read((char*)&status.current_x, sizeof(status.current_x));
    is.read((char*)&status.current_y, sizeof(status.current_y));

    status.current_x = ntohs(status.current_x);
    status.current_y = ntohs(status.current_y);

    return status;


}


