#include "shoggoth.h"
#include "franken.h"

#include "gen-cpp/Tracking.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class TrackingHandler : virtual public TrackingIf {
    shared_ptr<FrankenConnection> franken_conn;
    shared_ptr<Vision> vision;
    public:
        TrackingHandler(shared_ptr<FrankenConnection> _franken_conn,shared_ptr<Vision> _vision) {
            franken_conn = _franken_conn;
            vision = _vision;
        }
        
  void setMode(const PointMode::type mode) {
    // Your implementation goes here
    switch(mode) {
        case PointMode::AUTOMATIC:
            vision->setEngaged(true);
            printf("setMode to AUTOMATIC\n");
            break;
        case PointMode::MANUAL:
            printf("setMode to MANUAL\n");
            vision->setEngaged(false);
            break;
    }
  }

  PointMode::type getMode() {
    // Your implementation goes here
    switch(vision->getEngaged()) {
        case true:
            printf("getMode AUTOMATIC\n");
            return PointMode::AUTOMATIC;
        case false:
            printf("getMode MANUAL\n");
            return PointMode::MANUAL;
    }
  }

  void setPos(const Coordinates& coord) {
    // Your implementation goes here
    printf("setPos to %d, %d\n", coord.x, coord.y);
    cv::Point2f pp;
    pp.x = coord.x;
    pp.y = coord.y;
    franken_conn->writeGoto(pp);
  }

  void setOnOff(const bool state) {
    // Your implementation goes here
    printf("setOnOff to %d\n",state);
    franken_conn->writeOnOff(state);
  }

  void halt() {
    // Your implementation goes here
    printf("halt\n");

    franken_conn->writeHalt();
  }

  void sleep() {
    // Your implementation goes here
    printf("sleep\n");
    franken_conn->writeSleep();
  }

  void wake() {
    // Your implementation goes here
    printf("wake\n");
    franken_conn->writeWake();
  }

  void setIntensity(const int8_t intens) {
    // Your implementation goes here
    printf("setIntensity to %hhd\n", intens);
    franken_conn->writeIntensity(intens);
  }

  void calibrate() {
    // Your implementation goes here
    printf("calibrate\n");
    franken_conn->writeCal();
  }

  LightStatus::type getLightStatus() {
    // Your implementation goes here
    printf("getLightStatus\n");
    FrankenConnection::Status status = franken_conn->getStatus();

    typedef FrankenConnection::Status::LightStatus type;

    switch(status.light) {
        case type::OVERHEAT:
            return LightStatus::OVERHEAT;
        case type::ON:
            return LightStatus::ON;
        case type::OFF:
            return LightStatus::OFF;
    }

    
  }

  int8_t getIntensity() {
    // Your implementation goes here
    printf("getIntensity\n");
    FrankenConnection::Status status = franken_conn->getStatus();
    return status.intensity;
  }

  MoveStatus::type getMoveStatus() {
    // Your implementation goes here
    printf("getMoveStatus\n");
    FrankenConnection::Status status = franken_conn->getStatus();

    typedef FrankenConnection::Status::MoveStatus type;
    switch(status.move) {
        case type::RUNNING:
            return MoveStatus::RUNNING;
        case type::UNCAL:
            return MoveStatus::UNCAL;
        case type::CALING:
            return MoveStatus::CALING;
        case type::SLEEPING:
            return MoveStatus::SLEEPING;
        case type::TILT_OVERCUR:
            return MoveStatus::TILT_OVERCUR;
        case type::TILT_FAULT:
            return MoveStatus::TILT_FAULT;
        case type::PAN_OVERCUR:
            return MoveStatus::PAN_OVERCUR;
        case type::PAN_FAULT:
            return MoveStatus::PAN_FAULT;
    }
  }

  void getActualPos(Coordinates& _return) {
    // Your implementation goes here
    printf("getActualPos\n");
    FrankenConnection::Status status = franken_conn->getStatus();
    _return.x = status.current_x;
    _return.y = status.current_y;
    
  }

};

void thriftThread(boost::shared_ptr<FrankenConnection> franken_conn,boost::shared_ptr<Vision> vision) {
  int port = 9090;
  shared_ptr<TrackingHandler> handler(new TrackingHandler(franken_conn,vision));
  shared_ptr<TProcessor> processor(new TrackingProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
}

