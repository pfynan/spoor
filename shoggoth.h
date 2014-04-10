#pragma once

#include <boost/shared_ptr.hpp>
#include "franken.h"
#include "vision.h"


void thriftThread(boost::shared_ptr<FrankenConnection> franken_conn,boost::shared_ptr<Vision> vision);

