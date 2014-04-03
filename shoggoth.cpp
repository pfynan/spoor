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
 public:
  TrackingHandler() {
    // Your initialization goes here
  }

  void setMode(const PointMode::type mode) {
    // Your implementation goes here
    printf("setMode\n");
  }

  PointMode::type getMode() {
    // Your implementation goes here
    printf("getMode\n");
  }

  void setPos(const int16_t target) {
    // Your implementation goes here
    printf("setPos\n");
  }

  void getActualPos(Coordinates& _return) {
    // Your implementation goes here
    printf("getActualPos\n");
  }

  void setOnOff(const bool state) {
    // Your implementation goes here
    printf("setOnOff\n");
  }

};

void thriftThread() {
  int port = 9091;
  shared_ptr<TrackingHandler> handler(new TrackingHandler());
  shared_ptr<TProcessor> processor(new TrackingProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
}

