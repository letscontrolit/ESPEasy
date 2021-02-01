#ifndef HELPERS_STRINGGENERATOR_WIFI_H
#define HELPERS_STRINGGENERATOR_WIFI_H

#include "../../ESPEasy_common.h"

String WiFi_encryptionType(byte encryptionType);

#ifndef ESP32
String SDKwifiStatusToString(uint8_t sdk_wifistatus);
#endif

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus);

String getLastDisconnectReason();

#endif