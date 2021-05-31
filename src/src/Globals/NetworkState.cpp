#include "../Globals/NetworkState.h"

#include "../../ESPEasy_common.h"


// Ethernet Connection status
NetworkMedium_t active_network_medium = DEFAULT_NETWORK_MEDIUM;

bool webserverRunning(false);
bool webserver_init(false);
bool mDNS_init(false);


// NTP status
bool statusNTPInitialized = false;


// Setup DNS, only used if the ESP has no valid WiFi config
const byte DNS_PORT = 53;
IPAddress  apIP(DEFAULT_AP_IP);



// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;
