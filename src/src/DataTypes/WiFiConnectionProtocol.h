#ifndef DATATYPES_WIFICONNECTIONPROTOCOL_H
#define DATATYPES_WIFICONNECTIONPROTOCOL_H

#include <Arduino.h>

enum class WiFiConnectionProtocol {
    Unknown,
    WiFi_Protocol_11b,
    WiFi_Protocol_11g,
    WiFi_Protocol_11n
};

String toString(WiFiConnectionProtocol proto);



#endif