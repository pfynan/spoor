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
    printf("setMode\n");
  }

  PointMode::type getMode() {
    // Your implementation goes here
    printf("getMode\n");
  }

  void setPos(const Coordinates& coord) {
    // Your implementation goes here
    printf("setPos\n");
  }

  void setOnOff(const bool state) {
    // Your implementation goes here
    printf("setOnOff\n");
  }

  void halt() {
    // Your implementation goes here
    printf("halt\n");
  }

  void sleep() {
    // Your implementation goes here
    printf("sleep\n");
  }

  void wake() {
    // Your implementation goes here
    printf("wake\n");
  }

  void setIntensity(const int8_t intens) {
    // Your implementation goes here
    printf("setIntensity\n");
  }

  void calibrate() {
    // Your implementation goes here
    printf("calibrate\n");
  }

  LightStatus::type getLightStatus() {
    // Your implementation goes here
    printf("getLightStatus\n");
  }

  int8_t getIntensity() {
    // Your implementation goes here
    printf("getIntensity\n");
  }

  MoveStatus::type getMoveStatus() {
    // Your implementation goes here
    printf("getMoveStatus\n");
  }

  void getActualPos(Coordinates& _return) {
    // Your implementation goes here
    printf("getActualPos\n");
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

