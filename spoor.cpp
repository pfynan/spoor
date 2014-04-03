#include <iostream>





//#include "cvplot.hpp"

#include "thrift.h"
#include "vision.h"


#include <boost/program_options.hpp>

#include <boost/thread.hpp>

using namespace std;
using namespace boost;

namespace po = boost::program_options;



/*
 * Threads:
 *
 * Thrift  -|--| 
 * Franken -|--| // Next
 * Processing--|-|  // Wait
 * Gstreamer-----|  // Wait
 *
 * ...
 *
 * Jesus Christ almighty!
 *
 */





int main(int argc,char *argv[]) {

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("log", po::value<vector<string>>(), "log hook")
        ("log-file", po::value<vector<string>>(), "log file")
        ("in", po::value<string>()->default_value("test.mkv"), "input file")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }


    thread thrift_thread(thriftThread);
    thread vision_thread(visionThread,vm);
    
    vision_thread.join();


    return 0;
}


