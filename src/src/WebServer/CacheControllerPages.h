#ifndef WEBSERVER_WEBSERVER_CACHECONTROLLERPAGES_H
#define WEBSERVER_WEBSERVER_CACHECONTROLLERPAGES_H

#include "../WebServer/common.h"

#ifdef USES_C016

// ********************************************************************************
// URLs needed for C016_CacheController
// to help dump the content of the binary log files
// ********************************************************************************
void handle_dumpcache();

void handle_cache_json();

void handle_cache_csv();

#endif // ifdef USES_C016


#endif