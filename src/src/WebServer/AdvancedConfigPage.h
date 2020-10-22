#ifndef WEBSERVER_WEBSERVER_ADVANCEDCONFIGPAGE_H
#define WEBSERVER_WEBSERVER_ADVANCEDCONFIGPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_ADVANCED

// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_advanced();

void addFormDstSelect(bool isStart, uint16_t choice);

void addFormLogLevelSelect(const String& label, const String& id, int choice);

void addLogLevelSelect(const String& name, int choice);

void addFormLogFacilitySelect(const String& label, const String& id, int choice);

void addLogFacilitySelect(const String& name, int choice);

#endif // ifdef WEBSERVER_ADVANCED




#endif