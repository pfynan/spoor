#include "franken.h"
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

void parseSetAngle(tokenizer<char_separator<char>>::iterator cur_tok, ostream &os) {
    float x = lexical_cast<float>(*cur_tok);
    float y = lexical_cast<float>(*(++cur_tok));
    writeSetTargetAngle(os,cv::Point2f(x,y));
}

void parseExit(tokenizer<char_separator<char>>::iterator cur_tok, ostream &os) {
    exit(0);
}


void parse(tokenizer<char_separator<char>>::iterator cur_tok, ostream &os) {
    map<string,function<void(tokenizer<char_separator<char>>::iterator,ostream&)>> lut = 
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

    (c->second)(++cur_tok,os);
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

    system::error_code ec;
    boost::asio::connect(s, resolver.resolve({host, port}),ec);
    if(ec) {
        cerr << "Could not connect to light" << endl;
    }

    while(true) {

        string command;

        cout << "> " << flush;
        getline(cin,command);

        char_separator<char> sep(" ");
        tokenizer<char_separator<char>> tokens(command, sep);

        asio::streambuf buf;
        ostream stream(&buf);

        parse(tokens.begin(),stream);
        

        const char header = buf.size();

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
        buffers.push_back( buf.data() );
        boost::asio::write(s,buffers,ec);

    }


    return 0;
}
