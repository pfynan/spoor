#include "franken.h"
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

typedef tokenizer<char_separator<char>>::iterator tok_it;

void parseGoto(tok_it cur_tok, FrankenConnection &s) {
    float x = lexical_cast<float>(*cur_tok);
    float y = lexical_cast<float>(*(++cur_tok));
    s.writeGoto(cv::Point2f(x,y));
}

void parseStatus(tok_it cur_tok, FrankenConnection &s) {
    FrankenConnection::Status status;

    status = s.getStatus();

    cout << (char)status.light << endl;
    cout << status.intensity << endl;
    cout << (char)status.move << endl;
    cout << status.current_x << endl;
    cout << status.current_y << endl;

}


void parseExit(tok_it cur_tok, FrankenConnection &s) {
    exit(0);
}


void parse(string &command, FrankenConnection &s) {
    char_separator<char> sep(" ");
    tokenizer<char_separator<char>> tokens(command, sep);

    tok_it cur_tok = tokens.begin();

    map<string,std::function<void(tok_it,FrankenConnection&)> > lut = 
    {{"goto", parseGoto}
    //,{"intens", parseSetIntens}
    //,{"on", parseOn}
    //,{"off", parseOff}
    ,{"status", parseStatus}
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

    FrankenConnection conn;

    while(true) {

        string command;

        cout << "> " << flush;

        if(!getline(cin,command))
            break;


        parse(command,conn);
        

    }


    return 0;
}
