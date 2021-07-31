#ifndef WEBSERVER_WEBSERVER_SYSVARPAGE_H
#define WEBSERVER_WEBSERVER_SYSVARPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_SYSVARS


// ********************************************************************************
// Web Interface sysvars showing all system vars and their value.
// ********************************************************************************
void handle_sysvars();

void addSysVar_html(const __FlashStringHelper * input);
void addSysVar_html(const String& input);

#endif // WEBSERVER_SYSVARS




#endif