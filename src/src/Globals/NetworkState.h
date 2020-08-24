#ifndef GLOBALS_NETWORKSTATE_H
#define GLOBALS_NETWORKSTATE_H

#include <Arduino.h>

#include "ESPEasy-Globals.h"
#include "ESPEasy_plugindefs.h"
#include "../DataStructs/NetworkMedium.h"

// Ethernet Connectiopn status
#ifdef HAS_ETHERNET
extern NetworkMedium_t eth_wifi_mode;
extern bool    eth_connected;
#endif // ifdef HAS_ETHERNET


extern bool webserverRunning;
extern bool webserver_init;


extern bool dnsServerActive;

// NTP status
extern bool statusNTPInitialized;


// Setup DNS, only used if the ESP has no valid WiFi config
extern const byte DNS_PORT;
extern IPAddress  apIP;
extern DNSServer  dnsServer;

// udp protocol stuff (syslog, global sync, node info list, ntp time)
extern WiFiUDP portUDP;


#endif // GLOBALS_NETWORKSTATE_H
