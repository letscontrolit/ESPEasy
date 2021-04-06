#ifndef WEBSERVER_WEBSERVER_CUSTOMPAGE_H
#define WEBSERVER_WEBSERVER_CUSTOMPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_CUSTOM
// ********************************************************************************
// Web Interface custom page handler
// ********************************************************************************
boolean handle_custom(String path);
#endif

#endif