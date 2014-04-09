#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include "franken.h"

void visionThread(boost::program_options::variables_map &vm, boost::shared_ptr<FrankenConnection> franken_conn);

/*
class Vision
{
public:
    Vision(po::variables_map &vm, std::shared_ptr<FrankenConnection> franken_conn);
    virtual ~Vision ();

private:
    std::shared_ptr<FrankenConnection> franken_conn;
};
*/
