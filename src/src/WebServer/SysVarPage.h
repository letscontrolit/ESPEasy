#ifndef WEBSERVER_WEBSERVER_SYSVARPAGE_H
#define WEBSERVER_WEBSERVER_SYSVARPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_SYSVARS


// ********************************************************************************
// Web Interface sysvars showing all system vars and their value.
// ********************************************************************************
void handle_sysvars();

void addSysVar_html(const __FlashStringHelper * input);
void addSysVar_html_specialChar(const __FlashStringHelper * input);
void addSysVar_html(const String& input);
void addSysVar_html(String input, bool isSpecialChar);

#endif // WEBSERVER_SYSVARS




#endif