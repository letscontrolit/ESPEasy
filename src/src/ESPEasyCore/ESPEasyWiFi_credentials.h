#ifndef ESPEASYWIFI_CREDENTIALS_H
#define ESPEASYWIFI_CREDENTIALS_H

#include "../../ESPEasy_common.h"

const char* getLastWiFiSettingsSSID();
const char* getLastWiFiSettingsPassphrase();
bool selectNextWiFiSettings();
bool selectValidWiFiSettings();
bool wifiSettingsValid(const char *ssid, const char *pass);

#ifdef USES_ESPEASY_NOW
bool isESPEasy_now_only();
#endif

#endif // ESPEASYWIFI_CREDENTIALS_H