#include <string>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/Tracking.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


typedef tokenizer<char_separator<char>>::iterator tok_it;

void parseExit(tok_it cur_tok, TrackingClient&) {
    exit(0);
}

void parse_get_pos(tokenizer<char_separator<char>>::iterator cur_tok, TrackingClient& client) {
    Coordinates coord;
    client.getActualPos(coord);
    cout << u8"φ = " << coord.phi << endl;
    cout << u8"θ = " << coord.theta << endl;
}



void parse(string &command, TrackingClient &client) {
    char_separator<char> sep(" ");
    tokenizer<char_separator<char>> tokens(command, sep);

    tok_it cur_tok = tokens.begin();

    map<string,function<void(tok_it,TrackingClient&)>> lut = 
    {{"get_pos", parse_get_pos}
    ,{"exit", parseExit}
    };

    string cmd = *cur_tok;
    auto c = lut.find(cmd);
    if(c == lut.end()) {
        throw std::runtime_error("Unknown command");
    }

    (c->second)(++cur_tok,client);
}



int main(int argc, char *argv[])
{
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9091));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  TrackingClient client(protocol);

  transport->open();

    while(true) {

        string command;

        cout << "> " << flush;

        if(!getline(cin,command))
            break;


        parse(command,client);
        

    }

    transport->close();


    return 0;
}
