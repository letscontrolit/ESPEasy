#include "../wifi/ESPEasyWiFi_state_machine.h"

#ifdef ESP8266
# if FEATURE_WIFI

#  include "../wifi/ESPEasyWifi_abstracted.h"

namespace ESPEasy {
namespace net {
namespace wifi {

IPAddress ESPEasyWiFi_t::getIP() const
{
  IPAddress ip = WiFi.localIP();

  if (ip.isSet()) {
    return ip;
  }
/*
  ip = WiFi.softAPIP();

  if (ip.isSet()) {
    return ip;
  }
*/
  return IPAddress();
}

STA_connected_state ESPEasyWiFi_t::getSTA_connected_state() const
{
  // Perform check on SDK function, see: https://github.com/esp8266/Arduino/issues/7432
  station_status_t status = wifi_station_get_connect_status();

  switch (status)
  {
    case STATION_GOT_IP:
      return STA_connected_state::Connected;
    case STATION_NO_AP_FOUND:
      return STA_connected_state::Error_Not_Found;
    case STATION_CONNECT_FAIL:
    case STATION_WRONG_PASSWORD:
      return STA_connected_state::Error_Connect_Failed;
    case STATION_CONNECTING:
      return STA_connected_state::Connecting;
    case STATION_IDLE:
      break;

    default:
      break;
  }
  return STA_connected_state::Idle;
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy
# endif // if FEATURE_WIFI
#endif // ifdef ESP8266
