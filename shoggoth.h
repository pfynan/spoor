#pragma once

#include <boost/shared_ptr.hpp>
#include "franken.h"

void thriftThread(boost::shared_ptr<FrankenConnection> franken_conn);

