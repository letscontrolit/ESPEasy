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

ChecksumType SecurityStruct::computeChecksum() const {
  constexpr size_t len_upto_md5 = offsetof(SecurityStruct, md5);
  return ChecksumType(
    reinterpret_cast<const uint8_t *>(this), 
    sizeof(SecurityStruct),
    len_upto_md5);
}

bool SecurityStruct::checksumMatch() const {
  return computeChecksum().matchChecksum(md5);
}

bool SecurityStruct::updateChecksum() {
  const ChecksumType checksum = computeChecksum();
  if (checksum.matchChecksum(md5)) {
    return false;
  }
  checksum.getChecksum(md5);
  return true;
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
  if (slot == SecurityStruct::WiFiCredentialsSlot::first) {
    ZERO_FILL(WifiSSID);
    ZERO_FILL(WifiKey);
  } else if (slot == SecurityStruct::WiFiCredentialsSlot::second) {
    ZERO_FILL(WifiSSID2);
    ZERO_FILL(WifiKey2);
  }
}

bool SecurityStruct::hasWiFiCredentials() const {
  return hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::first) ||
         hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::second);
}

bool SecurityStruct::hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot slot) const {
  if (slot == SecurityStruct::WiFiCredentialsSlot::first)
      return (WifiSSID[0] != 0 && !String(WifiSSID).equalsIgnoreCase(F("ssid")));
  if (slot == SecurityStruct::WiFiCredentialsSlot::second)
      return (WifiSSID2[0] != 0 && !String(WifiSSID2).equalsIgnoreCase(F("ssid")));

  return false;
}

String SecurityStruct::getPassword() const {
  String res;
  const size_t passLength = strnlen(Password, sizeof(Password));
  res.reserve(passLength);
  for (size_t i = 0; i < passLength; ++i) {
    res += Password[i];
  }
  return res;
}