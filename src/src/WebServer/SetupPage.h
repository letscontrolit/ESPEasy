#ifndef WEBSERVER_WEBSERVER_SETUPPAGE_H
#define WEBSERVER_WEBSERVER_SETUPPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_SETUP

// ********************************************************************************
// Web Interface Setup Wizard
// ********************************************************************************

void handle_setup();

void handle_setup_scan_and_show(const String& ssid, const String& other, const String& password);

bool handle_setup_connectingStage(byte refreshCount);

#endif // ifdef WEBSERVER_SETUP





#endif