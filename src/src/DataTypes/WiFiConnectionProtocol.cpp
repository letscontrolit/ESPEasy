#include "../DataTypes/WiFiConnectionProtocol.h"

const __FlashStringHelper * toString(WiFiConnectionProtocol proto) {
  switch (proto) {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      return F("802.11b");
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      return F("802.11g");
#ifdef ESP8266
    case WiFiConnectionProtocol::WiFi_Protocol_11n:
      return F("802.11n");
#endif
#ifdef ESP32
    case WiFiConnectionProtocol::WiFi_Protocol_HT20:
      return F("802.11n (HT20)");
    case WiFiConnectionProtocol::WiFi_Protocol_HT40:
      return F("802.11n (HT40)");
    case WiFiConnectionProtocol::WiFi_Protocol_HE20:
      return F("802.11ax (HE20)");
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
    case WiFiConnectionProtocol::WiFi_Protocol_11a:
      return F("802.11a");
    case WiFiConnectionProtocol::WiFi_Protocol_VHT20:
      return F("802.11ac (VHT20)");
#endif
    case WiFiConnectionProtocol::WiFi_Protocol_LR:
      return F("802.11lr");

#endif
    case WiFiConnectionProtocol::Unknown:
      break;;
  }
  return F("-");
}