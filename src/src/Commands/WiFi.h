#ifndef COMMAND_WIFI_H
#define COMMAND_WIFI_H

#include "../../ESPEasy_common.h"

#if FEATURE_WIFI
#include <WString.h>

String                     Command_Wifi_SSID(struct EventStruct *event,
                                             const char         *Line);
String                     Command_Wifi_Key(struct EventStruct *event,
                                            const char         *Line);
String                     Command_Wifi_SSID2(struct EventStruct *event,
                                              const char         *Line);
String                     Command_Wifi_Key2(struct EventStruct *event,
                                             const char         *Line);
String                     Command_Wifi_HiddenSSID(struct EventStruct *event,
                                                   const char         *Line);
const __FlashStringHelper* Command_Wifi_Scan(struct EventStruct *event,
                                             const char         *Line);
const __FlashStringHelper* Command_Wifi_Connect(struct EventStruct *event,
                                                const char         *Line);
const __FlashStringHelper* Command_Wifi_Disconnect(struct EventStruct *event,
                                                   const char         *Line);
const __FlashStringHelper* Command_Wifi_APMode(struct EventStruct *event,
                                               const char         *Line);
const __FlashStringHelper* Command_Wifi_STAMode(struct EventStruct *event,
                                                const char         *Line);
String                     Command_Wifi_Mode(struct EventStruct *event,
                                             const char         *Line);
const __FlashStringHelper* Command_Wifi_AllowAP(struct EventStruct *event,
                                                const char         *Line);

// FIXME: TD-er This is not an erase, but actually storing the current settings
// in the wifi settings of the core library
const __FlashStringHelper* Command_WiFi_Erase(struct EventStruct *event,
                                              const char         *Line);

#if FEATURE_OTA_FW_UPDATE_ESP_HOSTED_MCU
// Perform OTA update on the esp-hosted-mcu firmware of the external WiFi module (typically ESP32-C6)
String                     Command_Wifi_OTA_hosted_mcu(struct EventStruct *event,
                                             const char         *Line);

#endif

#endif
#endif // COMMAND_WIFI_H
