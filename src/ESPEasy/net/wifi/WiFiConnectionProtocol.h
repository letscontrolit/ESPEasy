#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI


namespace ESPEasy {
namespace net {
namespace wifi {


# ifdef ESP8266

enum class WiFiConnectionProtocol {
  Unknown,
  WiFi_Protocol_11b,
  WiFi_Protocol_11g,
  WiFi_Protocol_11n

};


# endif // ifdef ESP8266

# ifdef ESP32

enum class WiFiConnectionProtocol {
  Unknown,
  WiFi_Protocol_11b,
  WiFi_Protocol_11g,
  WiFi_Protocol_HT20,
  WiFi_Protocol_HT40,
#  if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, \
                                            2, \
                                            0)
  WiFi_Protocol_11a,   // traditional (old) 5 GHz
  WiFi_Protocol_VHT20, // 5 GHz WiFi 5, 802.11ac
#  endif // if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
  WiFi_Protocol_HE20,  // WiFi 6
  WiFi_Protocol_LR

};

# endif // ifdef ESP32

const __FlashStringHelper* toString(WiFiConnectionProtocol proto);


} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
