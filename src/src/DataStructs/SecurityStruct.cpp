#include "../DataStructs/SecurityStruct.h"

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/CPlugins.h"

SecurityStruct::SecurityStruct() {
  ZERO_FILL(WifiSSID);
  ZERO_FILL(WifiKey);
  ZERO_FILL(WifiSSID2);
  ZERO_FILL(WifiKey2);
  ZERO_FILL(WifiAPKey);

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    ZERO_FILL(ControllerUser[i]);
    ZERO_FILL(ControllerPassword[i]);
  }
  ZERO_FILL(Password);
}

void SecurityStruct::validate() {
  ZERO_TERMINATE(WifiSSID);
  ZERO_TERMINATE(WifiKey);
  ZERO_TERMINATE(WifiSSID2);
  ZERO_TERMINATE(WifiKey2);
  ZERO_TERMINATE(WifiAPKey);

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    ZERO_TERMINATE(ControllerUser[i]);
    ZERO_TERMINATE(ControllerPassword[i]);
  }
  ZERO_TERMINATE(Password);
}

void SecurityStruct::clearWiFiCredentials() {
  ZERO_FILL(WifiSSID);
  ZERO_FILL(WifiKey);
  ZERO_FILL(WifiSSID2);
  ZERO_FILL(WifiKey2);
  addLog(LOG_LEVEL_INFO, F("WiFi : Clear WiFi credentials from settings"));
}

void SecurityStruct::clearWiFiCredentials(SecurityStruct::WiFiCredentialsSlot slot) {
  switch (slot) {
    case SecurityStruct::WiFiCredentialsSlot::first:
      ZERO_FILL(WifiSSID);
      ZERO_FILL(WifiKey);
      break;
    case SecurityStruct::WiFiCredentialsSlot::second:
      ZERO_FILL(WifiSSID2);
      ZERO_FILL(WifiKey2);
      break;
  }
}

bool SecurityStruct::hasWiFiCredentials() const {
  return hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::first) || 
         hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::second);
}

bool SecurityStruct::hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot slot) const {
  switch (slot) {
    case SecurityStruct::WiFiCredentialsSlot::first:
      return (WifiSSID[0] != 0 && !String(WifiSSID).equalsIgnoreCase(F("ssid")));
    case SecurityStruct::WiFiCredentialsSlot::second:
      return (WifiSSID2[0] != 0 && !String(WifiSSID2).equalsIgnoreCase(F("ssid")));
  }
  return false;
}
