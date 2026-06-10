#include "../wifi/WiFi_STA_State.h"

#if FEATURE_WIFI


namespace ESPEasy {
namespace net {
namespace wifi {

const __FlashStringHelper* toString(WiFi_STA_State_e state)
{
  switch (state)
  {
    case ESPEasy::net::wifi::WiFi_STA_State_e::Disabled: return F("Disabled");
    case ESPEasy::net::wifi::WiFi_STA_State_e::TimeOut: return F("TimeOut");
    case ESPEasy::net::wifi::WiFi_STA_State_e::Idle: return F("Idle");
    case ESPEasy::net::wifi::WiFi_STA_State_e::IdleWaiting: return F("IdleWaiting");
    case ESPEasy::net::wifi::WiFi_STA_State_e::STA_Scanning: return F("STA_Scanning");
    case ESPEasy::net::wifi::WiFi_STA_State_e::STA_AP_Scanning: return F("STA_AP_Scanning");
    case ESPEasy::net::wifi::WiFi_STA_State_e::STA_Connecting: return F("STA_Connecting");
    case ESPEasy::net::wifi::WiFi_STA_State_e::STA_Reconnecting: return F("STA_Reconnecting");
    case ESPEasy::net::wifi::WiFi_STA_State_e::STA_Connected: return F("STA_Connected");
  }
  return F("");
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
