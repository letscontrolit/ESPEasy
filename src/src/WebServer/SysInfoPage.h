#ifndef WEBSERVER_WEBSERVER_SYSINFOPAGE_H
#define WEBSERVER_WEBSERVER_SYSINFOPAGE_H

#include "../WebServer/common.h"


#if SHOW_SYSINFO_JSON

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json();

#endif

#ifdef WEBSERVER_SYSINFO

void handle_sysinfo();

void handle_sysinfo_basicInfo();

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_memory();
#endif

#if FEATURE_ETHERNET
void handle_sysinfo_Ethernet();
#endif

void handle_sysinfo_Network();

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_WiFiSettings();
#endif

void handle_sysinfo_Firmware();

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_SystemStatus();

void handle_sysinfo_NetworkServices();

void handle_sysinfo_ESP_Board();

void handle_sysinfo_Storage();
#endif

#endif    // ifdef WEBSERVER_SYSINFO



#endif