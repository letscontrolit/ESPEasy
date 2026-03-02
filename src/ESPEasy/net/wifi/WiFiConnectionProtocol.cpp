#include "../wifi/WiFiConnectionProtocol.h"

#if FEATURE_WIFI
namespace ESPEasy {
namespace net {
namespace wifi {


# ifdef ESP8266

const __FlashStringHelper* toString(WiFiConnectionProtocol proto) {
  switch (proto)
  {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      return F("802.11b");
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      return F("802.11g");
    case WiFiConnectionProtocol::WiFi_Protocol_11n:
      return F("802.11n");
    case WiFiConnectionProtocol::Unknown:
      break;
  }
  return F("-");
}

# endif // ifdef ESP8266

# ifdef ESP32

const __FlashStringHelper* toString(WiFiConnectionProtocol proto) {
  switch (proto)
  {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      return F("Wi-Fi 1: 802.11b");
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      return F("Wi-Fi 3: 802.11g");
    case WiFiConnectionProtocol::WiFi_Protocol_HT20:
      return F("Wi-Fi 4: 802.11n (HT20)");
    case WiFiConnectionProtocol::WiFi_Protocol_HT40:
      return F("Wi-Fi 4: 802.11n (HT40)");
  #  if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
    case WiFiConnectionProtocol::WiFi_Protocol_11a:
      return F("Wi-Fi 2: 802.11a");
    case WiFiConnectionProtocol::WiFi_Protocol_VHT20:
      return F("Wi-Fi 5: 802.11ac (VHT20)");
  #  endif // if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
    case WiFiConnectionProtocol::WiFi_Protocol_HE20:
      return F("Wi-Fi 6: 802.11ax (HE20)");
    case WiFiConnectionProtocol::WiFi_Protocol_LR:
      return F("802.11lr");

    case WiFiConnectionProtocol::Unknown:
      break;
  }
  return F("-");
}

# endif // ifdef ESP32
} // namespace wifi
} // namespace net
} // namespace ESPEasy
#endif // if FEATURE_WIFI
