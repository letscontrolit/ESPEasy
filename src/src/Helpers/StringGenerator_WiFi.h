#ifndef HELPERS_STRINGGENERATOR_WIFI_H
#define HELPERS_STRINGGENERATOR_WIFI_H

#include "../../ESPEasy_common.h"

const __FlashStringHelper * WiFi_encryptionType(byte encryptionType);

#ifdef ESP8266
#ifdef LIMIT_BUILD_SIZE
String SDKwifiStatusToString(uint8_t sdk_wifistatus);
#else
const __FlashStringHelper * SDKwifiStatusToString(uint8_t sdk_wifistatus);
#endif
#endif

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus);

String getLastDisconnectReason();

#endif