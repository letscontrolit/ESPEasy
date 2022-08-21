#ifndef HELPERS_STRINGGENERATOR_SYSTEM_H
#define HELPERS_STRINGGENERATOR_SYSTEM_H


#include "../../ESPEasy_common.h"
#include "../Globals/Statistics.h"
#include "../Helpers/Hardware.h"

/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/

#if FEATURE_MQTT
const __FlashStringHelper * getMQTT_state();
#endif // if FEATURE_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
const __FlashStringHelper * getLastBootCauseString();

#ifdef ESP32

// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
String  getResetReasonString(uint8_t icore);
#endif // ifdef ESP32

String  getResetReasonString();

String  getSystemBuildString();

String  formatSystemBuildNr(uint16_t buildNr);

String  getPluginDescriptionString();

String  getSystemLibraryString();

#ifdef ESP8266
String  getLWIPversion();
#endif // ifdef ESP8266


#endif