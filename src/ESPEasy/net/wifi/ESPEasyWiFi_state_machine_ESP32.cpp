#include "../wifi/ESPEasyWiFi_state_machine.h"

#ifdef ESP32
# if FEATURE_WIFI

#  include <WiFiGeneric.h>
#  include <esp_wifi.h> // Needed to call ESP-IDF functions like esp_wifi_....

#  ifndef ESP32P4
#   include <esp_phy_init.h>
#  endif

#  include <WiFi.h>
#  include <WiFiSTA.h>
#  include <WiFiType.h>

#  include "../wifi/ESPEasyWifi_abstracted.h"

#  include "../../../src/Globals/Settings.h"


namespace ESPEasy {
namespace net {
namespace wifi {

IPAddress ESPEasyWiFi_t::getIP() const
{
  if (WiFi.STA.hasIP()) {
    return WiFi.STA.localIP();
  }

  /*
     if (WiFi.AP.hasIP()) {
      return WiFi.AP.localIP();
     }
   */
  return IPAddress();
}

STA_connected_state ESPEasyWiFi_t::getSTA_connected_state() const
{
  if (WiFi.STA.connected()) {
    if (WiFi.STA.hasIP()) {
      return STA_connected_state::Connected;
    }
    return STA_connected_state::Connecting;
  }


  switch (WiFi.status())
  {
    case WL_CONNECTED:
      break; // return STA_connected_state::Connected;

    case WL_NO_SSID_AVAIL:
      return STA_connected_state::Error_Not_Found;

    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
      return STA_connected_state::Error_Connect_Failed;

    case WL_IDLE_STATUS:
      break; // return STA_connected_state::Connecting;


    case WL_NO_SHIELD:
    case WL_STOPPED:
    case WL_SCAN_COMPLETED:
    case WL_DISCONNECTED:
      // TODO: what to do here? Do we need an extra connected state?
      break;
  }

  // TODO: Keep track of whether connection is in progress
  // The status() function does not return a reply stating "connecting"

  /*   if (_sta_connecting) {
      return STA_connected_state::Connecting;
     }
   */
  return STA_connected_state::Idle;
}


} // namespace wifi
} // namespace net
} // namespace ESPEasy
# endif // if FEATURE_WIFI
#endif // ifdef ESP32
