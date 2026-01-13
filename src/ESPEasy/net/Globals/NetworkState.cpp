#include "../Globals/NetworkState.h"

#include "../../../ESPEasy_common.h"


// Ethernet Connection status
ESPEasy::net::NetworkMedium_t active_network_medium = ESPEasy::net::NetworkMedium_t::NotSet;

bool webserverRunning(false);
bool webserver_init(false);

#if FEATURE_MDNS
bool mDNS_init(false);
#endif


// NTP status
bool statusNTPInitialized = false;


// Setup DNS, only used if the ESP has no valid WiFi config
const uint8_t DNS_PORT = 53;
IPAddress     apIP(DEFAULT_AP_IP);


// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;
#ifdef ESP32
bool nonDefaultNetworkInterface_gotIP{};
#endif

