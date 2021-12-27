#ifndef WEBSERVER_WEBSERVER_CUSTOMPAGE_H
#define WEBSERVER_WEBSERVER_CUSTOMPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_CUSTOM
// ********************************************************************************
// Web Interface custom page handler
// ********************************************************************************
bool handle_custom(const String& path);
#endif

#endif