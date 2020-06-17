#include "ESPEasyWiFi_credentials.h"
#include "src/Globals/RTC.h"
#include "src/Globals/SecuritySettings.h"

// ********************************************************************************
// Manage WiFi credentials
// ********************************************************************************

#include "src/Globals/ESPEasy_now_state.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Globals/RTC.h"
#include "src/Globals/ESPEasyWiFiEvent.h"

#ifdef USES_ESPEASY_NOW
#define ESPEASY_NOW_TMP_SSID       "ESPEASY_NOW"
#define ESPEASY_NOW_TMP_PASSPHRASE "random_passphrase"
#endif

bool validWiFiSettingsIndex(uint8_t index) {
  return index <= 2;
}

#ifdef USES_ESPEASY_NOW
bool isESPEasy_now_only() {
  return RTC.lastWiFiSettingsIndex == 2;
}
#endif

const char* getLastWiFiSettingsSSID() {
  switch (RTC.lastWiFiSettingsIndex) {
    case 0:
      return SecuritySettings.WifiSSID;
    case 1:
      return SecuritySettings.WifiSSID2;

#ifdef USES_ESPEASY_NOW
    case 2: 
      return ESPEASY_NOW_TMP_SSID;
#endif
    default:
      break;
  }
  return nullptr;
}

const char* getLastWiFiSettingsPassphrase() {
  switch (RTC.lastWiFiSettingsIndex) {
    case 0:
      return SecuritySettings.WifiKey;
    case 1:
      return SecuritySettings.WifiKey2;

#ifdef USES_ESPEASY_NOW
    case 2: 
      return ESPEASY_NOW_TMP_PASSPHRASE;
#endif
    default:
      break;
  }
  return nullptr;
}

bool selectNextWiFiSettings() {
  uint8_t tmp = RTC.lastWiFiSettingsIndex;
  RTC.lastWiFiSettingsIndex += 1;
  if (!validWiFiSettingsIndex(RTC.lastWiFiSettingsIndex)) {
    RTC.lastWiFiSettingsIndex = 0;
  }

  if (!wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase())) {
    // other settings are not correct, switch back.
    RTC.lastWiFiSettingsIndex = tmp;
    return false; // Nothing changed.
  }
#ifdef USES_ESPEASY_NOW
  if (espeasy_now_only != isESPEasy_now_only()) {
    if (!espeasy_now_only) {
      espeasy_now_only = true;
      ESPEasy_now_handler.end();
      addLog(LOG_LEVEL_INFO, F("ESPEasy-Now only mode"));
    }
    if (espeasy_now_only) {
      if (!ESPEasy_now_handler.begin()) {
        espeasy_now_only = false;
      }
    }
  }
  if (espeasy_now_only && use_EspEasy_now) {
    if (!ESPEasy_now_handler.active()) {
      espeasy_now_only = false;
    }
  }
#endif
  return true;
}

bool selectValidWiFiSettings() {
  if (wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase())) {
    return true;
  }
  return selectNextWiFiSettings();
}

bool wifiSettingsValid(const char *ssid, const char *pass) {
  if (ssid == nullptr || pass == nullptr) {
    return false;
  }
  if ((ssid[0] == 0) || (strcasecmp(ssid, "ssid") == 0)) {
    return false;
  }

  //  if (pass[0] == 0) return false; // Allow for empty pass
  if (strlen(ssid) > 32) { return false; }

  if (strlen(pass) > 64) { return false; }
  return true;
}
