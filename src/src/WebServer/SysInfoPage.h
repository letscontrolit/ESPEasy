#ifndef WEBSERVER_WEBSERVER_SYSINFOPAGE_H
#define WEBSERVER_WEBSERVER_SYSINFOPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json();

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_SYSINFO

void handle_sysinfo();

void handle_sysinfo_basicInfo();

void handle_sysinfo_memory();

#ifdef HAS_ETHERNET
void handle_sysinfo_Ethernet();
#endif

void handle_sysinfo_Network();

void handle_sysinfo_WiFiSettings();

void handle_sysinfo_Firmware();

void handle_sysinfo_SystemStatus();

void handle_sysinfo_ESP_Board();

void handle_sysinfo_Storage();

#endif    // ifdef WEBSERVER_SYSINFO



#endif