#include "NetworkState.h"

// Ethernet Connectiopn status
#ifdef HAS_ETHERNET
NetworkMedium_t eth_wifi_mode = NetworkMedium_t::Ethernet;
bool eth_connected = false;
#endif // ifdef HAS_ETHERNET


bool webserverRunning(false);
bool webserver_init(false);


bool dnsServerActive = false;

// NTP status
bool statusNTPInitialized = false;


// Setup DNS, only used if the ESP has no valid WiFi config
const byte DNS_PORT = 53;
IPAddress  apIP(DEFAULT_AP_IP);
DNSServer  dnsServer;


// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;
