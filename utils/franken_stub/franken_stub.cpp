#include "franken.h"
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

typedef tokenizer<char_separator<char>>::iterator tok_it;
typedef boost::asio::ip::tcp::socket sock;

void parseSetAngle(tok_it cur_tok, sock &s) {
    using namespace std::placeholders;
    float x = lexical_cast<float>(*cur_tok);
    float y = lexical_cast<float>(*(++cur_tok));
    sendMessage(bind(writeSetTargetAngle,_1,cv::Point2f(x,y)),s);
}

void parseExit(tok_it cur_tok, sock &s) {
    exit(0);
}


void parse(string &command, sock &s) {
    char_separator<char> sep(" ");
    tokenizer<char_separator<char>> tokens(command, sep);

    tok_it cur_tok = tokens.begin();

    map<string,function<void(tok_it,sock&)>> lut = 
    {{"angle", parseSetAngle}
    //,{"intens", parseSetIntens}
    //,{"on", parseOn}
    //,{"off", parseOff}
    ,{"exit", parseExit}
    };

    string cmd = *cur_tok;
    auto c = lut.find(cmd);
    if(c == lut.end()) {
        throw std::runtime_error("Unknown command");
    }

    (c->second)(++cur_tok,s);
}



int main(int argc, char *argv[])
{

    string host = "127.0.0.1";
    string port = "9090";

    if(argc == 3) {
        host = argv[1];
        port = argv[2];
    }

    cout << "Connecting to " << host << ":" << port << endl;
    
    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(tcp::v4(), host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    system::error_code ec;
    s.connect(*iterator,ec);
    //boost::asio::connect(s, resolver.resolve({host, port}),ec);
    if(ec) {
        cerr << "Could not connect to light" << endl;
    }

    while(true) {

        string command;

        cout << "> " << flush;

        if(!getline(cin,command))
            break;


        parse(command,s);
        

    }


    return 0;
}
