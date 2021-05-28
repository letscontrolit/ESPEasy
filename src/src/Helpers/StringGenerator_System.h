#ifndef HELPERS_STRINGGENERATOR_SYSTEM_H
#define HELPERS_STRINGGENERATOR_SYSTEM_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"
#include "../Globals/Statistics.h"
#include "../Helpers/Hardware.h"

/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/

#ifdef USES_MQTT
const __FlashStringHelper * getMQTT_state();
#endif // USES_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
const __FlashStringHelper * getLastBootCauseString();

#ifdef ESP32

// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
String  getResetReasonString(byte icore);
#endif // ifdef ESP32

String  getResetReasonString();

String  getSystemBuildString();

String  getPluginDescriptionString();

String  getSystemLibraryString();

#ifdef ESP8266
String  getLWIPversion();
#endif // ifdef ESP8266


#endif