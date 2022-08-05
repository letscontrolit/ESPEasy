#ifndef WEBSERVER_WEBSERVER_ADVANCEDCONFIGPAGE_H
#define WEBSERVER_WEBSERVER_ADVANCEDCONFIGPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_ADVANCED

#include "../DataTypes/TimeSource.h"


// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_advanced();

void addFormDstSelect(bool isStart, uint16_t choice);

void addFormExtTimeSourceSelect(const __FlashStringHelper * label, const __FlashStringHelper * id, ExtTimeSource_e choice);

void addFormLogLevelSelect(LabelType::Enum label, int choice);

void addFormLogFacilitySelect(const __FlashStringHelper * label, const __FlashStringHelper * id, int choice);


#endif // ifdef WEBSERVER_ADVANCED




#endif