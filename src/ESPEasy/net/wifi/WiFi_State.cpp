#include "../wifi/WiFi_State.h"

#if FEATURE_WIFI


namespace ESPEasy {
namespace net {
namespace wifi {

const __FlashStringHelper* toString(WiFiState_e state)
{
  switch (state)
  {
    case ESPEasy::net::wifi::WiFiState_e::Disabled: return F("Disabled");
    case ESPEasy::net::wifi::WiFiState_e::WiFiOFF: return F("OFF");
    case ESPEasy::net::wifi::WiFiState_e::AP_only: return F("AP_only");
    case ESPEasy::net::wifi::WiFiState_e::AP_Fallback: return F("AP_Fallback");
    case ESPEasy::net::wifi::WiFiState_e::IdleWaiting: return F("IdleWaiting");
    case ESPEasy::net::wifi::WiFiState_e::STA_Scanning: return F("STA_Scanning");
    case ESPEasy::net::wifi::WiFiState_e::STA_AP_Scanning: return F("STA_AP_Scanning");
    case ESPEasy::net::wifi::WiFiState_e::STA_Connecting: return F("STA_Connecting");
    case ESPEasy::net::wifi::WiFiState_e::STA_Reconnecting: return F("STA_Reconnecting");
    case ESPEasy::net::wifi::WiFiState_e::STA_Connected: return F("STA_Connected");
  }
  return F("");
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
