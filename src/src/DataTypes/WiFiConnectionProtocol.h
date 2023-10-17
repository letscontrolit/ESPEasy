#ifndef DATATYPES_WIFICONNECTIONPROTOCOL_H
#define DATATYPES_WIFICONNECTIONPROTOCOL_H

#include "../../ESPEasy_common.h"

enum class WiFiConnectionProtocol {
    Unknown,
    WiFi_Protocol_11b,
    WiFi_Protocol_11g,
#ifdef ESP8266
    WiFi_Protocol_11n
#endif
#ifdef ESP32
    WiFi_Protocol_HT20,
    WiFi_Protocol_HT40,
    WiFi_Protocol_HE20,   // WiFi 6
    WiFi_Protocol_LR
#endif
};

const __FlashStringHelper * toString(WiFiConnectionProtocol proto);



#endif