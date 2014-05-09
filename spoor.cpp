#include <iostream>





//#include "cvplot.hpp"

#include "shoggoth.h"
#include "vision.h"
#include "franken.h"


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


    boost::shared_ptr<FrankenConnection> franken_conn(new FrankenConnection);

    boost::shared_ptr<Vision> vision(new Vision(vm,franken_conn));

    thread thrift_thread(thriftThread,franken_conn,vision);

    franken_conn->getStatus();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    franken_conn->writeWake();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    franken_conn->writeCal();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10000));


    thread vision_thread(&Vision::run,vision);

    vision_thread.join();


    return 0;
}


