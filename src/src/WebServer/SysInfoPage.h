#pragma once

#include "../WebServer/common.h"


#if SHOW_SYSINFO_JSON

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json();

#endif // if SHOW_SYSINFO_JSON

#ifdef WEBSERVER_SYSINFO

void handle_sysinfo();


void handle_sysinfo_Network();

# ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_NetworkServices();
# endif

#endif    // ifdef WEBSERVER_SYSINFO
