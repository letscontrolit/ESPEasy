#include "ESPEasyWiFi_credentials.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"

// ********************************************************************************
// Manage WiFi credentials
// ********************************************************************************
const char* getLastWiFiSettingsSSID() {
  return RTC.lastWiFiSettingsIndex == 0 ? SecuritySettings.WifiSSID : SecuritySettings.WifiSSID2;
}

const char* getLastWiFiSettingsPassphrase() {
  return RTC.lastWiFiSettingsIndex == 0 ? SecuritySettings.WifiKey : SecuritySettings.WifiKey2;
}

bool selectNextWiFiSettings() {
  uint8_t tmp = RTC.lastWiFiSettingsIndex;

  RTC.lastWiFiSettingsIndex = (RTC.lastWiFiSettingsIndex + 1) % 2;

  if (!wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase())) {
    // other settings are not correct, switch back.
    RTC.lastWiFiSettingsIndex = tmp;
    return false; // Nothing changed.
  }
  return true;
}

bool selectValidWiFiSettings() {
  if (wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase())) {
    return true;
  }
  return selectNextWiFiSettings();
}

bool wifiSettingsValid(const char *ssid, const char *pass) {
  if ((ssid[0] == 0) || (strcasecmp(ssid, "ssid") == 0)) {
    return false;
  }

  //  if (pass[0] == 0) return false; // Allow for empty pass
  if (strlen(ssid) > 32) { return false; }

  if (strlen(pass) > 64) { return false; }
  return true;
}
