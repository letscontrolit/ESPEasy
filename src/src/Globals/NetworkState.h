#ifndef GLOBALS_NETWORKSTATE_H
#define GLOBALS_NETWORKSTATE_H

#include <Arduino.h>

#include "../../ESPEasy-Globals.h"
#include "../DataStructs/ESPEasy_plugin_functions.h"
#include "../DataStructs/NetworkMedium.h"

// Ethernet Connectiopn status
extern NetworkMedium_t active_network_medium;
extern bool    eth_connected;

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
