#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI


# ifdef ESP8266

#  include "../DataStructs/NWPluginData_static_runtime.h"

#  include <IPAddress.h>

// ********************************************************************************

// Work-around for setting _useStaticIP
// See reported issue: https://github.com/esp8266/Arduino/issues/4114
// ********************************************************************************

#  include <ESP8266WiFi.h>
#  include <ESP8266WiFiSTA.h>

namespace ESPEasy {
namespace net {
namespace wifi {


class ESPEasyWiFi_STA_EventHandler
{
public:

  ESPEasyWiFi_STA_EventHandler(networkIndex_t networkIndex);
  ~ESPEasyWiFi_STA_EventHandler();

  static bool                  initialized();

  NWPluginData_static_runtime* getNWPluginData_static_runtime();

  WiFiDisconnectReason         getLastDisconnectReason() const;
  uint8_t                      getAuthMode() const;

  const __FlashStringHelper *  getWiFi_encryptionType() const;


  // ********************************************************************************
  // Functions called on events.
  // Make sure not to call anything in these functions that result in delay() or yield()
  // ********************************************************************************
  static void onConnected(const WiFiEventStationModeConnected& event);

  static void onDisconnect(const WiFiEventStationModeDisconnected& event);

  static void onGotIP(const WiFiEventStationModeGotIP& event);

  static void onDHCPTimeout();

  static void onConnectedAPmode(const WiFiEventSoftAPModeStationConnected& event);

  static void onDisconnectedAPmode(const WiFiEventSoftAPModeStationDisconnected& event);

  static void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged& event);

}; // class ESPEasyWiFi_STA_EventHandler

} // namespace wifi
} // namespace net
} // namespace ESPEasy

# endif // ifdef ESP8266

#endif // if FEATURE_WIFI
