#ifndef WEBSERVER_WEBSERVER_TIMINGSTATS_H
#define WEBSERVER_WEBSERVER_TIMINGSTATS_H

#include "../WebServer/common.h"

#if defined(WEBSERVER_TIMINGSTATS) && FEATURE_TIMING_STATS

#include "../DataStructs/TimingStats.h"

void handle_timingstats();

#endif 


#endif