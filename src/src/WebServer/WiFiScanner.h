#ifndef WEBSERVER_WEBSERVER_WIFISCANNER_H
#define WEBSERVER_WEBSERVER_WIFISCANNER_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface Wifi scanner
// ********************************************************************************
void handle_wifiscanner_json();

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_WIFI_SCANNER

void handle_wifiscanner();

#endif // ifdef WEBSERVER_WIFI_SCANNER

#endif