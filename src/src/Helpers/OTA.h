#ifndef HELPERS_OTA_H
#define HELPERS_OTA_H

#include "../../ESPEasy_common.h"

bool OTA_possible(uint32_t& maxSketchSize,
                  bool    & use2step);

#ifdef FEATURE_ARDUINO_OTA

/********************************************************************************************\
   Allow updating via the Arduino OTA-protocol. (this allows you to upload directly from platformio)
 \*********************************************************************************************/

void ArduinoOTAInit();

#endif // ifdef FEATURE_ARDUINO_OTA


#endif