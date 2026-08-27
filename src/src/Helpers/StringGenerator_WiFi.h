#ifndef HELPERS_STRINGGENERATOR_WIFI_H
#define HELPERS_STRINGGENERATOR_WIFI_H

#include "../../ESPEasy_common.h"

#include "../../ESPEasy/net/wifi/WiFiDisconnectReason.h"

const __FlashStringHelper * WiFi_encryptionType(uint8_t encryptionType);

#ifdef ESP8266
#ifdef LIMIT_BUILD_SIZE
String SDKwifiStatusToString(uint8_t sdk_wifistatus);
#else
const __FlashStringHelper * SDKwifiStatusToString(uint8_t sdk_wifistatus);
#endif
#endif

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus);

WiFiDisconnectReason getWiFi_disconnectReason();

String getWiFi_disconnectReason_str();

const __FlashStringHelper*   getWiFi_encryptionType();

#endif