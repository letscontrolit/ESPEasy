#include "../Helpers/Networking.h"

#include "../Commands/ExecuteCommand.h"
#include "../CustomBuild/CompiletimeDefines.h"
#include "../DataStructs/NodeStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/EventValueSource.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasyEthEvent.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"

#ifdef USES_ESPEASY_NOW
#include "../Globals/ESPEasy_now_handler.h"
#endif

#include "../Globals/EventQueue.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Nodes.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"

#include "../../ESPEasy-Globals.h"

#include <IPAddress.h>
#include <base64.h>
#include <MD5Builder.h> // for getDigestAuth

#include <WiFiUdp.h>

#include <lwip/dns.h>

// Generic Networking routines

// Syslog
// UDP system messaging
// SSDP
//  #if LWIP_VERSION_MAJOR == 2
#define IPADDR2STR(addr) (uint8_t)((uint32_t)addr &  0xFF), (uint8_t)(((uint32_t)addr >> 8) &  0xFF), \
  (uint8_t)(((uint32_t)addr >> 16) &  0xFF), (uint8_t)(((uint32_t)addr >> 24) &  0xFF)

//  #endif

#include <lwip/netif.h>

#ifdef ESP8266
#include <lwip/opt.h>
#include <lwip/udp.h>
#include <lwip/igmp.h>
#include <include/UdpContext.h>
#endif

#ifdef SUPPORT_ARP
# include <lwip/etharp.h>

# ifdef ESP32
#  include <lwip/etharp.h>
#  include <lwip/tcpip.h>

void _etharp_gratuitous_func(struct netif *netif) {
  etharp_gratuitous(netif);
}

void etharp_gratuitous_r(struct netif *netif) {
  tcpip_callback_with_block((tcpip_callback_fn)_etharp_gratuitous_func, netif, 0);
}

# endif // ifdef ESP32

#endif  // ifdef SUPPORT_ARP

#if FEATURE_DOWNLOAD
# ifdef ESP8266
#  include <ESP8266HTTPClient.h>
# endif // ifdef ESP8266
# ifdef ESP32
#  include <HTTPClient.h>
#  include <Update.h>
# endif // ifdef ESP32
#endif  // if FEATURE_DOWNLOAD

#include <vector>

/*********************************************************************************************\
   Syslog client
\*********************************************************************************************/
void sendSyslog(uint8_t logLevel, const String& message)
{
  if ((Settings.Syslog_IP[0] != 0) && NetworkConnected())
  {
    IPAddress broadcastIP(Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);

    FeedSW_watchdog();

    if (portUDP.beginPacket(broadcastIP, Settings.SyslogPort) == 0) {
      // problem resolving the hostname or port
      return;
    }
    unsigned int prio = Settings.SyslogFacility * 8;

    if (logLevel == LOG_LEVEL_ERROR) {
      prio += 3; // syslog error
    }
    else if (logLevel == LOG_LEVEL_INFO) {
      prio += 5; // syslog notice
    }
    else {
      prio += 7;
    }

    // An RFC3164 compliant message must be formated like :  "<PRIO>[TimeStamp ]Hostname TaskName: Message"

    // Using Settings.Name as the Hostname (Hostname must NOT content space)
    {
      String header;
      header += '<';
      header += prio;
      header += '>';
      header += NetworkCreateRFCCompliantHostname(true);
      header += F(" EspEasy: ");
      header.trim();
      header.replace(' ', '_');
      
      #ifdef ESP8266
      portUDP.write(header.c_str(),                                    header.length());
      #endif // ifdef ESP8266
      #ifdef ESP32
      portUDP.write(reinterpret_cast<const uint8_t *>(header.c_str()), header.length());
      #endif // ifdef ESP32
    }

    #ifdef ESP8266
    portUDP.write(message.c_str(), message.length());
    #endif // ifdef ESP8266
    #ifdef ESP32
    portUDP.write(reinterpret_cast<const uint8_t *>(message.c_str()), message.length());
    #endif // ifdef ESP32

    portUDP.endPacket();
    FeedSW_watchdog();
    delay(0);
  }
}

#if FEATURE_ESPEASY_P2P

/*********************************************************************************************\
   Send event using UDP message
\*********************************************************************************************/
void SendUDPCommand(uint8_t destUnit, const char *data, uint8_t dataLength)
{
  if (!NetworkConnected(10)) {
    return;
  }

  if (destUnit != 0)
  {
    sendUDP(destUnit, (const uint8_t *)data, dataLength);
    delay(10);
  } else {
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        sendUDP(it->first, (const uint8_t *)data, dataLength);
        delay(10);
      }
    }
  }
  delay(50);
}

/*********************************************************************************************\
   Send UDP message to specific unit (unit 255=broadcast)
\*********************************************************************************************/
void sendUDP(uint8_t unit, const uint8_t *data, uint8_t size)
{
  if (!NetworkConnected(10)) {
    return;
  }

  IPAddress remoteNodeIP = getIPAddressForUnit(unit);

  if (remoteNodeIP[0] == 0) {
    return;
  }

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLogMove(LOG_LEVEL_DEBUG_MORE,  strformat(
      F("UDP  : Send UDP message to %d (%s)"), 
      unit,
      remoteNodeIP.toString().c_str()
      ));
  }
# endif // ifndef BUILD_NO_DEBUG

  statusLED(true);
  FeedSW_watchdog();
  portUDP.beginPacket(remoteNodeIP, Settings.UDPPort);
  portUDP.write(data, size);
  portUDP.endPacket();
  FeedSW_watchdog();
  delay(0);
}

/*********************************************************************************************\
   Update UDP port (ESPEasy propiertary protocol)
\*********************************************************************************************/
void updateUDPport(bool force)
{
  static uint16_t lastUsedUDPPort = 0;

  if (!force && Settings.UDPPort == lastUsedUDPPort) {
    return;
  }

  if (lastUsedUDPPort != 0) {
    portUDP.stop();
    lastUsedUDPPort = 0;
  }

  if (!NetworkConnected()) {
    return;
  }

  if (Settings.UDPPort != 0) {
    if (portUDP.begin(Settings.UDPPort) == 0) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, concat(F("UDP : Cannot bind to ESPEasy p2p UDP port "), Settings.UDPPort));
      }
    } else {
      lastUsedUDPPort = Settings.UDPPort;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("UDP : Start listening on port "), Settings.UDPPort));
      }
    }
  }
}

/*********************************************************************************************\
   Check UDP messages (ESPEasy propiertary protocol)
\*********************************************************************************************/
boolean runningUPDCheck = false;
void checkUDP()
{
  if (Settings.UDPPort == 0) {
    return;
  }

  if (runningUPDCheck) {
    return;
  }

  runningUPDCheck = true;

  // UDP events
  int packetSize = portUDP.parsePacket();

  if (packetSize > 0 /*&& portUDP.remotePort() == Settings.UDPPort*/)
  {
    statusLED(true);

    IPAddress remoteIP = portUDP.remoteIP();

    if (portUDP.remotePort() == 123)
    {
      // unexpected NTP reply, drop for now...
      runningUPDCheck = false;
      return;
    }

    // UDP_PACKETSIZE_MAX should be as small as possible but still enough to hold all
    // data for PLUGIN_UDP_IN or CPLUGIN_UDP_IN calls
    // This node may also receive other UDP packets which may be quite large
    // and then crash due to memory allocation failures
    if ((packetSize >= 2) && (packetSize < UDP_PACKETSIZE_MAX)) {
      // Allocate buffer to process packet.
      std::vector<char> packetBuffer;
      packetBuffer.resize(packetSize + 1);

      if (packetBuffer.size() >= static_cast<size_t>(packetSize)) {
        memset(&packetBuffer[0], 0, packetSize + 1);
        int len = portUDP.read(&packetBuffer[0], packetSize);

        if (len >= 2) {
          if (static_cast<uint8_t>(packetBuffer[0]) != 255)
          {
            packetBuffer[len] = 0;
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLogMove(LOG_LEVEL_DEBUG,  
                strformat(F("UDP  : %s  Command: %s"), 
                  formatIP(remoteIP, true).c_str(), 
                  wrapWithQuotesIfContainsParameterSeparatorChar(String(&packetBuffer[0])).c_str()
                  ));
            }
            #endif
            ExecuteCommand_all({EventValueSource::Enum::VALUE_SOURCE_SYSTEM, &packetBuffer[0]}, true);
          }
          else
          {
            // binary data!
            switch (packetBuffer[1])
            {
              case 1: // sysinfo message
              {
                if (len < 13) {
                  break;
                }
                int copy_length = sizeof(NodeStruct);
                // Older versions sent 80 bytes, regardless of the size of NodeStruct
                // Make sure the extra data received is ignored as it was also not initialized
                if (len == 80) {
                  copy_length = 56;
                }

                if (copy_length > (len - 2)) {
                  copy_length = (len - 2);
                }
                NodeStruct received;
                memcpy(&received, &packetBuffer[2], copy_length);

                if (received.validate(remoteIP)) {
                  Nodes.addNode(received); // Create a new element when not present

# ifndef BUILD_NO_DEBUG

                  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
                    addLogMove(LOG_LEVEL_DEBUG_MORE,  
                      strformat(F("UDP  : %s (%d) %s,%s,%d"), 
                        formatIP(remoteIP).c_str(), 
                        received.unit,
                        received.STA_MAC().toString().c_str(), 
                        formatIP(received.IP(), true).c_str(), 
                        received.unit));
                  }

#endif // ifndef BUILD_NO_DEBUG
                }
                break;
              }

              default:
              {
                struct EventStruct TempEvent;
                TempEvent.Data = reinterpret_cast<uint8_t *>(&packetBuffer[0]);
                TempEvent.Par1 = remoteIP[3];
                TempEvent.Par2 = len;
                String dummy;
                // TD-er: Disabled the PLUGIN_UDP_IN call as we don't have any plugin using this.
                //PluginCall(PLUGIN_UDP_IN, &TempEvent, dummy);
                CPluginCall(CPlugin::Function::CPLUGIN_UDP_IN, &TempEvent);
                break;
              }
            }
          }
        }
      }
    }
  }

  // Flush any remaining content of the packet.
  while (portUDP.available()) {
    // Do not call portUDP.flush() as that's meant to sending the packet (on ESP8266)
    portUDP.read();
  }
  runningUPDCheck = false;
}

/*********************************************************************************************\
   Get formatted IP address for unit
   formatcodes: 0 = default toString(), 1 = empty string when invalid, 2 = 0 when invalid
\*********************************************************************************************/
String formatUnitToIPAddress(uint8_t unit, uint8_t formatCode) {
  IPAddress unitIPAddress = getIPAddressForUnit(unit);

  if (unitIPAddress[0] == 0) { // Invalid?
    switch (formatCode) {
      case 1:                  // Return empty string
      {
        return EMPTY_STRING;
      }
      case 2: // Return "0"
      {
        return String('0');
      }
    }
  }
  return formatIP(unitIPAddress);
}

/*********************************************************************************************\
   Get IP address for unit
\*********************************************************************************************/
IPAddress getIPAddressForUnit(uint8_t unit) {
  if (unit == 255) {
    const IPAddress ip(255, 255, 255, 255);
    return ip;
  }
  auto it = Nodes.find(unit);

  if (it == Nodes.end() || it->second.ip[0] == 0) {
    IPAddress ip;
    return ip;
  }
#if FEATURE_USE_IPV6
/*
  // FIXME TD-er: for now do not try to send to IPv6
  if (it->second.hasIPv6_mac_based_link_local) {
    return it->second.IPv6_link_local();
  }
  if (it->second.hasIPv6_mac_based_link_global) {
    return it->second.IPv6_global();
  }
*/
#endif
  return it->second.IP();
}


String getNameForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end() || it->second.getNodeName().isEmpty()) {
    return EMPTY_STRING;
  }
  return it->second.getNodeName();
}

long getAgeForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end()) {
    return -1000; // milliseconds, negative == unknown
  }
  return static_cast<long>(it->second.getAge());
}

uint16_t getBuildnrForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end() || it->second.build == 0) {
    return 0;
  }
  return it->second.build;
}

float getLoadForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end()) {
    return 0.0f;
  }
  return it->second.getLoad();
}

uint8_t getTypeForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end()) {
    return 0;
  }
  return it->second.nodeType;
}

const __FlashStringHelper* getTypeStringForUnit(uint8_t unit) {
  auto it = Nodes.find(unit);

  if (it == Nodes.end()) {
    return F("");
  }
  return it->second.getNodeTypeDisplayString();
}

/*********************************************************************************************\
   Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList()
{
  unsigned long max_age;
  const unsigned long max_age_allowed = 10 * 60 * 1000; // 10 minutes

  Nodes.refreshNodeList(max_age_allowed, max_age);

  #ifdef USES_ESPEASY_NOW
  #ifdef ESP8266
  // FIXME TD-er: Do not perform regular scans on ESP32 as long as we cannot scan per channel
  if (!Nodes.isEndpoint()) {
    WifiScan(true, Nodes.getESPEasyNOW_channel());
  }
  #endif
  #endif

  if (max_age > (0.75 * max_age_allowed)) {
    Scheduler.sendGratuitousARP_now();
  }
  sendSysInfoUDP(1);
  #ifdef USES_ESPEASY_NOW
  if (Nodes.recentlyBecameDistanceZero()) {
    // Send to all channels
    ESPEasy_now_handler.sendDiscoveryAnnounce(-1);
  } else {
    ESPEasy_now_handler.sendDiscoveryAnnounce();
  }
  ESPEasy_now_handler.sendNTPquery();
  ESPEasy_now_handler.sendTraceRoute();
  #endif // ifdef USES_ESPEASY_NOW
}

/*********************************************************************************************\
   Broadcast system info to other nodes. (to update node lists)
\*********************************************************************************************/
void sendSysInfoUDP(uint8_t repeats)
{
  if ((Settings.UDPPort == 0) || !NetworkConnected(10)) {
    return;
  }

  // 1 byte 'binary token 255'
  // 1 byte id '1'
  // NodeStruct object (packed data struct)

  // send my info to the world...
# ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, F("UDP  : Send Sysinfo message"));
# endif // ifndef BUILD_NO_DEBUG

  const NodeStruct *thisNode = Nodes.getThisNode();

  if (thisNode == nullptr) {
    // Should not happen
    return;
  }

  // Prepare UDP packet to send
  constexpr size_t data_size = sizeof(NodeStruct) + 2;
  uint8_t data[data_size] = {0};
  data[0] = 255;
  data[1] = 1;
  memcpy(&data[2], thisNode, sizeof(NodeStruct));

  for (uint8_t counter = 0; counter < repeats; counter++)
  {
    statusLED(true);

    IPAddress broadcastIP(255, 255, 255, 255);
    FeedSW_watchdog();
    portUDP.beginPacket(broadcastIP, Settings.UDPPort);
    portUDP.write(data, data_size);
    portUDP.endPacket();

    if (counter < (repeats - 1)) {
      // FIXME TD-er: Must use scheduler to send out messages, not using delay
      delay(100);
    }
  }
}

#endif // FEATURE_ESPEASY_P2P

#if defined(ESP8266)

# if FEATURE_SSDP

/********************************************************************************************\
   Respond to HTTP XML requests for SSDP information
 \*********************************************************************************************/
void SSDP_schema() {
  if (!NetworkConnected(10)) {
    return;
  }

  const IPAddress ip     = NetworkLocalIP();
  const uint32_t  chipId = ESP.getChipId();
  char uuid[64];

  sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
            (uint16_t)((chipId >> 16) & 0xff),
            (uint16_t)((chipId >>  8) & 0xff),
            (uint16_t)chipId        & 0xff);

  web_server.client().print(F(
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/xml\r\n"
                 "Connection: close\r\n"
                 "Access-Control-Allow-Origin: *\r\n"
                 "\r\n"
                 "<?xml version=\"1.0\"?>"
                 "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
                 "<specVersion>"
                 "<major>1</major>"
                 "<minor>0</minor>"
                 "</specVersion>"
                 "<URLBase>http://"));

  web_server.client().print(formatIP(ip));
  web_server.client().print(F(":80/</URLBase>"
                 "<device>"
                 "<deviceType>urn:schemas-upnp-org:device:BinaryLight:1</deviceType>"
                 "<friendlyName>"));
  web_server.client().print(Settings.getName());
  web_server.client().print(F("</friendlyName>"
                 "<presentationURL>/</presentationURL>"
                 "<serialNumber>"));
  web_server.client().print(String(ESP.getChipId()));
  web_server.client().print(F("</serialNumber>"
                 "<modelName>ESP Easy</modelName>"
                 "<modelNumber>"));
  web_server.client().print(getValue(LabelType::GIT_BUILD));
  web_server.client().print(F("</modelNumber>"
                 "<modelURL>http://www.letscontrolit.com</modelURL>"
                 "<manufacturer>http://www.letscontrolit.com</manufacturer>"
                 "<manufacturerURL>http://www.letscontrolit.com</manufacturerURL>"
                 "<UDN>uuid:"));
  web_server.client().print(String(uuid));
  web_server.client().print(F("</UDN></device>"
                 "</root>\r\n"
                 "\r\n"));
}

/********************************************************************************************\
   Global SSDP stuff
 \*********************************************************************************************/

UdpContext *_server;

IPAddress _respondToAddr;
uint16_t  _respondToPort;

bool _pending;
unsigned short _delay;
unsigned long  _process_time;
unsigned long  _notify_time;

#  define SSDP_INTERVAL     1200
#  define SSDP_PORT         1900
#  define SSDP_METHOD_SIZE  10
#  define SSDP_URI_SIZE     2
#  define SSDP_BUFFER_SIZE  64
#  define SSDP_MULTICAST_TTL 2

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);


/********************************************************************************************\
   Launch SSDP listener and send initial notify
 \*********************************************************************************************/
bool SSDP_begin() {
  _pending = false;

  if (_server != nullptr) {
    _server->unref();

    // FIXME TD-er: Shouldn't this also call delete _server ?

    _server = nullptr;
  }

  _server = new (std::nothrow) UdpContext;

  if (_server == nullptr) {
    return false;
  }
  _server->ref();

  ip_addr_t ifaddr;

  ifaddr.addr = NetworkLocalIP();
  ip_addr_t multicast_addr;

  multicast_addr.addr = (uint32_t)SSDP_MULTICAST_ADDR;

  if (igmp_joingroup(&ifaddr, &multicast_addr) != ERR_OK) {
    return false;
  }

#  ifdef CORE_POST_2_5_0

  // Core 2.5.0 changed the signature of some UdpContext function.
  if (!_server->listen(IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }

  _server->setMulticastInterface(&ifaddr);
  _server->setMulticastTTL(SSDP_MULTICAST_TTL);
  _server->onRx(&SSDP_update);

  if (!_server->connect(&multicast_addr, SSDP_PORT)) {
    return false;
  }
#  else // ifdef CORE_POST_2_5_0

  if (!_server->listen(*IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }

  _server->setMulticastInterface(ifaddr);
  _server->setMulticastTTL(SSDP_MULTICAST_TTL);
  _server->onRx(&SSDP_update);

  if (!_server->connect(multicast_addr, SSDP_PORT)) {
    return false;
  }
#  endif // ifdef CORE_POST_2_5_0

  SSDP_update();

  return true;
}

/********************************************************************************************\
   Send SSDP messages (notify & responses)
 \*********************************************************************************************/
void SSDP_send(uint8_t method) {
  uint32_t ip = NetworkLocalIP();

  // FIXME TD-er: Why create String objects of these flashstrings?
  String _ssdp_response_template = F(
    "HTTP/1.1 200 OK\r\n"
    "EXT:\r\n"
    "ST: upnp:rootdevice\r\n");

  String _ssdp_notify_template = F(
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "NT: upnp:rootdevice\r\n"
    "NTS: ssdp:alive\r\n");

  String _ssdp_packet_template = F(
    "%s"                                           // _ssdp_response_template / _ssdp_notify_template
    "CACHE-CONTROL: max-age=%u\r\n"                // SSDP_INTERVAL
    "SERVER: Arduino/1.0 UPNP/1.1 ESPEasy/%u\r\n"  // _modelNumber
    "USN: uuid:%s\r\n"                             // _uuid
    "LOCATION: http://%u.%u.%u.%u:80/ssdp.xml\r\n" // NetworkLocalIP(),
    "\r\n");
  {
    char uuid[64]   = { 0 };
    uint32_t chipId = ESP.getChipId();
    sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
              (uint16_t)((chipId >> 16) & 0xff),
              (uint16_t)((chipId >>  8) & 0xff),
              (uint16_t)chipId        & 0xff);

    char *buffer = new (std::nothrow) char[1460]();

    if (buffer == nullptr) { return; }
    int len = snprintf(buffer, 1460,
                       _ssdp_packet_template.c_str(),
                       (method == 0) ? _ssdp_response_template.c_str() : _ssdp_notify_template.c_str(),
                       SSDP_INTERVAL,
                       Settings.Build,
                       uuid,
                       IPADDR2STR(&ip)
                       );

    _server->append(buffer, len);
    delete[] buffer;
  }

  ip_addr_t remoteAddr;
  uint16_t  remotePort;

  if (method == 0) {
    remoteAddr.addr = _respondToAddr;
    remotePort      = _respondToPort;
  } else {
    remoteAddr.addr = SSDP_MULTICAST_ADDR;
    remotePort      = SSDP_PORT;
  }
  _server->send(&remoteAddr, remotePort);
  statusLED(true);
}

/********************************************************************************************\
   SSDP message processing
 \*********************************************************************************************/
void SSDP_update() {
  if (!_pending && _server->next()) {
    ssdp_method_t method = NONE;

    _respondToAddr = _server->getRemoteAddress();
    _respondToPort = _server->getRemotePort();

    typedef enum { METHOD, URI, PROTO, KEY, VALUE, ABORT } states;
    states state = METHOD;

    typedef enum { START, MAN, ST, MX } headers;
    headers header = START;

    uint8_t cursor = 0;
    uint8_t cr     = 0;

    char buffer[SSDP_BUFFER_SIZE] = { 0 };

    while (_server->getSize() > 0) {
      char c = _server->read();

      (c == '\r' || c == '\n') ? cr++ : cr = 0;

      switch (state) {
        case METHOD:

          if (c == ' ') {
            if (strcmp_P(buffer, PSTR("M-SEARCH")) == 0) { method = SEARCH; }
            else if (strcmp_P(buffer, PSTR("NOTIFY")) == 0) { method = NOTIFY; }

            if (method == NONE) { state = ABORT; }
            else { state = URI; }
            cursor = 0;
          } else if (cursor < SSDP_METHOD_SIZE - 1) {
            buffer[cursor++] = c;
            buffer[cursor]   = '\0';
          }
          break;
        case URI:

          if (c == ' ') {
            if (strcmp(buffer, "*")) { state = ABORT; }
            else { state = PROTO; }
            cursor = 0;
          } else if (cursor < SSDP_URI_SIZE - 1) {
            buffer[cursor++] = c;
            buffer[cursor]   = '\0';
          }
          break;
        case PROTO:

          if (cr == 2) {
            state  = KEY;
            cursor = 0;
          }
          break;
        case KEY:

          if (cr == 4) {
            _pending      = true;
            _process_time = millis();
          }
          else if (c == ' ') {
            cursor = 0;
            state  = VALUE;
          }
          else if ((c != '\r') && (c != '\n') && (c != ':') && (cursor < SSDP_BUFFER_SIZE - 1)) {
            buffer[cursor++] = c;
            buffer[cursor]   = '\0';
          }
          break;
        case VALUE:

          if (cr == 2) {
            switch (header) {
              case START:
                break;
              case MAN:
                break;
              case ST:

                if (strcmp_P(buffer, PSTR("ssdp:all"))) {
                  state = ABORT;
                }

                // if the search type matches our type, we should respond instead of ABORT
                if (strcmp_P(buffer, PSTR("urn:schemas-upnp-org:device:BinaryLight:1")) == 0) {
                  _pending      = true;
                  _process_time = millis();
                  state         = KEY;
                }
                break;
              case MX:
                _delay = HwRandom(0, atoi(buffer)) * 1000L;
                break;
            }

            if (state != ABORT) {
              state  = KEY;
              header = START;
              cursor = 0;
            }
          } else if ((c != '\r') && (c != '\n')) {
            if (header == START) {
              if (strncmp(buffer, "MA", 2) == 0) { header = MAN; }
              else if (strcmp(buffer, "ST") == 0) { header = ST; }
              else if (strcmp(buffer, "MX") == 0) { header = MX; }
            }

            if (cursor < SSDP_BUFFER_SIZE - 1) {
              buffer[cursor++] = c;
              buffer[cursor]   = '\0';
            }
          }
          break;
        case ABORT:
          _pending = false; _delay = 0;
          break;
      }
    }
  }

  if (_pending && timeOutReached(_process_time + _delay)) {
    _pending = false; _delay = 0;
    SSDP_send(NONE);
  } else if ((_notify_time == 0) || timeOutReached(_notify_time + (SSDP_INTERVAL * 1000L))) {
    _notify_time = millis();
    SSDP_send(NOTIFY);
  }

  if (_pending) {
    while (_server->next()) {
      _server->flush();
    }
  }
}

# endif // if FEATURE_SSDP
#endif  // if defined(ESP8266)


// ********************************************************************************
// Return subnet range of WiFi.
// ********************************************************************************
bool getSubnetRange(IPAddress& low, IPAddress& high)
{
  if (!WiFiEventData.WiFiGotIP()) {
    return false;
  }

  const IPAddress ip     = NetworkLocalIP();
  const IPAddress subnet = NetworkSubnetMask();

  low  = ip;
  high = ip;

  // Compute subnet range.
  for (uint8_t i = 0; i < 4; ++i) {
    if (subnet[i] != 255) {
      low[i]  = low[i] & subnet[i];
      high[i] = high[i] | ~subnet[i];
    }
  }
  return true;
}

// ********************************************************************************
// Functions to test and handle network/client connectivity.
// ********************************************************************************

#ifdef CORE_POST_2_5_0
# include <AddrList.h>
#endif // ifdef CORE_POST_2_5_0


bool hasIPaddr() {
  if (useStaticIP()) { return true; }

#ifdef CORE_POST_2_5_0
  bool configured = false;

  for (auto addr : addrList) {
    if ((configured = (!addr.isLocal() && (addr.ifnumber() == STATION_IF)))) {
      /*
         ESPEASY_SERIAL_CONSOLE_PORT.printf("STA: IF='%s' hostname='%s' addr= %s\n",
                    addr.ifname().c_str(),
                    addr.ifhostname(),
                    addr.toString().c_str());
       */
      break;
    }
  }
  return configured;
#else // ifdef CORE_POST_2_5_0
  return WiFi.isConnected();
#endif // ifdef CORE_POST_2_5_0
}

bool useStaticIP() {
  #if FEATURE_ETHERNET
  if (active_network_medium == NetworkMedium_t::Ethernet) {
    return ethUseStaticIP();
  }
  #endif
  return WiFiUseStaticIP();
}

// Check connection. Maximum timeout 500 msec.
bool NetworkConnected(uint32_t timeout_ms) {

#ifdef USES_ESPEASY_NOW
  if (isESPEasy_now_only()) {
    return false;
  }
#endif

  if (timeout_ms > 500) {
    timeout_ms = 500;
  }

  uint32_t timer     = millis() + timeout_ms;
  uint32_t min_delay = timeout_ms / 20;

  if (min_delay < 10) {
    delay(0); // Allow at least once time for backgroundtasks
    min_delay = 10;
  }

  // Apparently something needs network, perform check to see if it is ready now.
  while (!NetworkConnected()) {
    if (timeOutReached(timer)) {
      return false;
    }
    delay(min_delay); // Allow the backgroundtasks to continue procesing.
  }
  return true;
}

bool hostReachable(const IPAddress& ip) {
  if (!NetworkConnected()) { return false; }

  return true; // Disabled ping as requested here:
  // https://github.com/letscontrolit/ESPEasy/issues/1494#issuecomment-397872538

  /*
     // Only do 1 ping at a time to return early
     uint8_t retry = 3;
     while (retry > 0) {
   #if defined(ESP8266)
      if (Ping.ping(ip, 1)) return true;
   #endif
   #if defined(ESP32)
     if (ping_start(ip, 4, 0, 0, 5)) return true;
   #endif
      delay(50);
      --retry;
     }
     if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("Host unreachable: ");
      log += formatIP(ip);
      addLog(LOG_LEVEL_ERROR, log);
     }
     if (ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
      // Work-around to fix connected but not able to communicate.
      addLog(LOG_LEVEL_ERROR, F("WiFi : Detected strange behavior, reconnect wifi."));
      WifiDisconnect();
     }
     logConnectionStatus();
     return false;
   */
}

#if FEATURE_HTTP_CLIENT
bool connectClient(WiFiClient& client, const char *hostname, uint16_t port, uint32_t timeout_ms) {
  IPAddress ip;

  if (resolveHostByName(hostname, ip, timeout_ms)) {
    return connectClient(client, ip, port, timeout_ms);
  }
  return false;
}

bool connectClient(WiFiClient& client, IPAddress ip, uint16_t port, uint32_t timeout_ms)
{
  START_TIMER;

  if (!NetworkConnected()) {
    client.stop();
    return false;
  }

  // In case of domain name resolution error result can be negative.
  // https://github.com/esp8266/Arduino/blob/18f643c7e2d6a0da9d26ff2b14c94e6536ab78c1/libraries/Ethernet/src/Dns.cpp#L44
  // Thus must match the result with 1.
  bool connected = (client.connect(ip, port) == 1);

  delay(0);

  if (!connected) {
    Scheduler.sendGratuitousARP_now();
    client.stop(); // Make sure to start over without some stale connection
  }
  STOP_TIMER(CONNECT_CLIENT_STATS);
#if defined(ESP32) || defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0)
#else

  if (connected) {
    client.keepAlive(); // Use default keep alive values
  }
#endif // if defined(ESP32) || defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0)
  return connected;
}
#endif // FEATURE_HTTP_CLIENT

void scrubDNS() {
  #if FEATURE_ETHERNET
  if (active_network_medium == NetworkMedium_t::Ethernet) {
    if (EthEventData.EthServicesInitialized()) {
      setDNS(0, EthEventData.dns0_cache);
      setDNS(1, EthEventData.dns1_cache);
    }
    return;
  }
  #endif
  if (WiFiEventData.WiFiServicesInitialized()) {
    setDNS(0, WiFiEventData.dns0_cache);
    setDNS(1, WiFiEventData.dns1_cache);
  }
}

bool valid_DNS_address(const IPAddress& dns) {
  return (/*dns.v4() != (uint32_t)0x00000000 && */
          dns != IPAddress((uint32_t)0xFD000000) && 
#ifdef ESP32
          // Bug where IPv6 global prefix is set as DNS
          // Global IPv6 prefixes currently start with 2xxx::
          (dns[0] & 0xF0) != 0x20 && 
#endif
          dns != INADDR_NONE);
}

bool setDNS(int index, const IPAddress& dns) {
  if (index >= 2) return false;
  #ifdef ESP8266
  if(dns.isSet() && dns != WiFi.dnsIP(index)) {
    dns_setserver(index, dns);
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("IP   : Set DNS: "),  formatIP(dns)));
    }
    #endif
    return true;
  }
  #endif
  #ifdef ESP32
  ip_addr_t d;
  d.type = IPADDR_TYPE_V4;

  if (valid_DNS_address(dns) || dns == INADDR_NONE) {
    // Set DNS0-Server
    d.u_addr.ip4.addr = static_cast<uint32_t>(dns);
    const ip_addr_t* cur_dns = dns_getserver(index);
    if (cur_dns != nullptr && cur_dns->u_addr.ip4.addr == d.u_addr.ip4.addr) {
      // Still the same as before
      return false;
    }
    dns_setserver(index, &d);
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("IP   : Set DNS: "),  formatIP(dns)));
    }
    return true;
  }
  #endif
  return false;
}

bool resolveHostByName(const char *aHostname, IPAddress& aResult, uint32_t timeout_ms) {
  START_TIMER;

  if (!NetworkConnected()) {
    return false;
  }

  FeedSW_watchdog();

  // FIXME TD-er: Must try to restore DNS server entries.
  scrubDNS();

#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  bool resolvedIP = WiFi.hostByName(aHostname, aResult) == 1;
#else // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  bool resolvedIP = WiFi.hostByName(aHostname, aResult, timeout_ms) == 1;
#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  delay(0);
  FeedSW_watchdog();

  if (!resolvedIP) {
    Scheduler.sendGratuitousARP_now();
  }
  STOP_TIMER(HOST_BY_NAME_STATS);
  return resolvedIP;
}

bool hostReachable(const String& hostname) {
  IPAddress remote_addr;

  if (resolveHostByName(hostname.c_str(), remote_addr)) {
    return hostReachable(remote_addr);
  }

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    addLogMove(LOG_LEVEL_ERROR,  concat(F("Hostname cannot be resolved: "), hostname));
  }
  return false;
}

// Create a random port for the UDP connection.
// Return true when successful.
bool beginWiFiUDP_randomPort(WiFiUDP& udp) {
  if (!NetworkConnected()) {
    return false;
  }
  unsigned int attempts = 3;

  while (attempts > 0) {
    --attempts;
    long port = HwRandom(1025, 65535);

    if (udp.begin(port) != 0) {
      return true;
    }
  }
  return false;
}

void sendGratuitousARP() {
  if (!NetworkConnected()) {
    return;
  }
#ifdef SUPPORT_ARP

  // See https://github.com/letscontrolit/ESPEasy/issues/2374
  START_TIMER;
  netif *n = netif_list;

  while (n) {
    if ((n->hwaddr_len == ETH_HWADDR_LEN) &&
        (n->flags & NETIF_FLAG_ETHARP) &&
        ((n->flags & NETIF_FLAG_LINK_UP) && (n->flags & NETIF_FLAG_UP))) {
      # ifdef ESP32
      etharp_gratuitous_r(n);
      # else // ifdef ESP32
      etharp_gratuitous(n);
      # endif // ifdef ESP32
    }
    n = n->next;
  }
  STOP_TIMER(GRAT_ARP_STATS);
#endif // ifdef SUPPORT_ARP
}

bool splitHostPortString(const String& hostPortString, String& host, uint16_t& port) {
  port = 80; // Some default
  int index_colon = hostPortString.indexOf(':');

  if (index_colon >= 0) {
    int32_t port_tmp;

    if (!validIntFromString(hostPortString.substring(index_colon + 1), port_tmp)) {
      return false;
    }

    if ((port_tmp < 0) || (port_tmp > 65535)) { return false; }
    port = port_tmp;
    host = hostPortString.substring(0, index_colon);
  } else {
    // No port nr defined.
    host = hostPortString;
  }
  return true;
}

bool splitUserPass_HostPortString(const String& hostPortString, String& user, String& pass, String& host, uint16_t& port)
{
  const int pos_at = hostPortString.indexOf('@');

  if (pos_at != -1) {
    user = hostPortString.substring(0, pos_at);
    const int pos_colon = user.indexOf(':');

    if (pos_colon != -1) {
      pass = user.substring(pos_colon + 1);
      user = user.substring(0, pos_colon);
    }
    return splitHostPortString(hostPortString.substring(pos_at + 1), host, port);
  }
  return splitHostPortString(hostPortString, host, port);
}

// Split a full URL like "http://hostname:port/path/file.htm"
// Return value is everything after the hostname:port section (including /)
String splitURL(const String& fullURL, String& user, String& pass, String& host, uint16_t& port, String& file) {
  int starthost = fullURL.indexOf(F("://"));

  if (starthost == -1) {
    starthost = 0;
  } else {
    starthost += 3;
  }
  const int endhost = fullURL.indexOf('/', starthost);
  splitUserPass_HostPortString(fullURL.substring(starthost, endhost), user, pass, host, port);

  if (endhost == -1) {
    return EMPTY_STRING;
  }

  int startfile = fullURL.lastIndexOf('/');

  if (startfile >= 0) {
    file = fullURL.substring(startfile);
  }
  return fullURL.substring(endhost);
}

String get_user_agent_string() {
  static unsigned int agent_size = 20;
  String userAgent;

  userAgent.reserve(agent_size);
  userAgent += F("ESP Easy/");
  userAgent += get_build_nr();
  userAgent += '/';
  userAgent += get_build_date();
  userAgent += ' ';
  userAgent += get_build_time();
  agent_size = userAgent.length();
  return userAgent;
}

bool splitHeaders(int& strpos, const String& multiHeaders, String& name, String& value) {
  if (strpos < 0) {
    return false;
  }
  int colonPos = multiHeaders.indexOf(':', strpos);

  if (colonPos < 0) {
    return false;
  }
  name = multiHeaders.substring(strpos, colonPos);
  int valueEndPos = multiHeaders.indexOf('\n', colonPos + 1);

  if (valueEndPos < 0) {
    value  = multiHeaders.substring(colonPos + 1);
    strpos = -1;
  } else {
    value  = multiHeaders.substring(colonPos + 1, valueEndPos);
    strpos = valueEndPos + 1;
  }
  value.replace('\r', ' ');
  value.trim();
  return true;
}

String extractParam(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param);

  if (_begin == -1) { return EMPTY_STRING; }
  return authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
}

#if FEATURE_HTTP_CLIENT
String getCNonce(const int len) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  String s;

  for (int i = 0; i < len; ++i) {
    // FIXME TD-er: Is this "-1" correct? The mod operator makes sure we never reach the sizeof index
    s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return s;
}

String getDigestAuth(const String& authReq,
                     const String& username,
                     const String& password,
                     const String& method,
                     const String& uri,
                     unsigned int  counter) {
  // extracting required parameters for RFC 2069 simpler Digest
  const String realm  = extractParam(authReq, F("realm=\""), '"');
  const String nonce  = extractParam(authReq, F("nonce=\""), '"');
  const String cNonce = getCNonce(8);

  char nc[9];

  snprintf(nc, sizeof(nc), "%08x", counter);

  // parameters for the RFC 2617 newer Digest
  MD5Builder md5;

  md5.begin();
  md5.add(username + ':' + realm + ':' + password); // md5 of the user:realm:user
  md5.calculate();
  const String h1 = md5.toString();

  md5.begin();
  md5.add(method + ':' + uri);
  md5.calculate();
  const String h2 = md5.toString();

  md5.begin();
  md5.add(h1 + ':' + nonce + ':' + String(nc) + ':' + cNonce + F(":auth:") + h2);
  md5.calculate();

  // return authorization
  return strformat(
    F("Digest username=\"%s\""
    ", realm=\"%s\""
    ", nonce=\"%s\""
    ", uri=\"%s\""
    ", algorithm=\"MD5\", qop=auth, nc=%s, cnonce=\"%s\""
    ", response=\"%s\""),
    username.c_str(),
    realm.c_str(),
    nonce.c_str(),
    uri.c_str(),
    nc,
    cNonce.c_str(),
    md5.toString().c_str()); // response
}

#ifndef BUILD_NO_DEBUG
void log_http_result(const HTTPClient& http,
                     const String    & logIdentifier,
                     const String    & host,
                     const String    & HttpMethod,
                     int               httpCode,
                     const String    & response)
{
  uint8_t loglevel = LOG_LEVEL_ERROR;
  bool    success  = false;

  // HTTP codes:
  // 1xx Informational response
  // 2xx Success
  if ((httpCode >= 100) && (httpCode < 300)) {
    loglevel = LOG_LEVEL_INFO;
    success  = true;
  }

  if (loglevelActiveFor(loglevel)) {
    String log = strformat(F("HTTP : %s %s %s"), 
                          logIdentifier.c_str(), host.c_str(), HttpMethod.c_str());

    if (!success) {
      log += F("failed ");
    }
    log += concat(F("HTTP code: "), httpCode);

    if (!success) {
      log += ' ';
      log += http.errorToString(httpCode);
    }

    if (response.length() > 0) {
      log += concat(F(" Received reply: "), response.substring(0, 100)); // Returned string may be huge, so only log the first part.
    }
    addLogMove(loglevel, log);
  }
}
#endif

int http_authenticate(const String& logIdentifier,
                      WiFiClient  & client,
                      HTTPClient  & http,
                      uint16_t      timeout,
                      const String& user,
                      const String& pass,
                      const String& host,
                      uint16_t      port,
                      const String& uri,
                      const String& HttpMethod,
                      const String& header,
                      const String& postStr,
                      bool          must_check_reply)
{
  if (!uri.startsWith(F("/"))) {
    return http_authenticate(
      logIdentifier,
      client,
      http,
      timeout,
      user,
      pass,
      host,
      port,
      concat(F("/"), uri),
      HttpMethod,
      header,
      postStr,
      must_check_reply);
  }
  int httpCode = 0;
  const bool hasCredentials = !user.isEmpty() && !pass.isEmpty();

  if (hasCredentials) {
    must_check_reply = true;
    http.setAuthorization(user.c_str(), pass.c_str());
  } else {
    http.setAuthorization(""); // Clear Basic authorization
#ifdef ESP32
    http.setAuthorizationType(""); // Default type is "Basic"
#endif
  }
  http.setTimeout(timeout);
  http.setUserAgent(get_user_agent_string());

  if (Settings.SendToHTTP_follow_redirects()) {
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setRedirectLimit(2);
  }

  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((timeout + 500) / 1000); // in seconds!!!!
  Client *pClient = &client;
  pClient->setTimeout(timeout);
  #else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  client.setTimeout(timeout);                // in msec as it should be!
  #endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // Add request header as fall back.
  // When adding another "accept" header, it may be interpreted as:
  // "if you have XXX, send it; or failing that, just give me what you've got."
  http.addHeader(F("Accept"), F("*/*;q=0.1"));

  // Add client IP
  http.addHeader(F("X-Forwarded-For"), formatIP(NetworkLocalIP()));

  delay(0);
  scrubDNS();
#if defined(CORE_POST_2_6_0) || defined(ESP32)
  http.begin(client, host, port, uri, false); // HTTP
#else // if defined(CORE_POST_2_6_0) || defined(ESP32)
  http.begin(client, host, port, uri);
#endif // if defined(CORE_POST_2_6_0) || defined(ESP32)

  const char *keys[] = { "WWW-Authenticate" };
  http.collectHeaders(keys, 1);

  {
    int headerpos = 0;
    String name, value;

    while (splitHeaders(headerpos, header, name, value)) {
      // Disabled the check to exclude "Authorization", due to: 
      //   https://github.com/letscontrolit/ESPEasy/issues/4364
      // Check was added for: https://github.com/letscontrolit/ESPEasy/issues/4355
      // However, I doubt this was the actual bug. More likely the supplied credential strings were not entirely empty for whatever reason.
      //
      // Work-around to not add Authorization header since the HTTPClient code
      // only ignores this when base64Authorication is set.
      
//      if (!name.equalsIgnoreCase(F("Authorization"))) {
        http.addHeader(name, value);
//      }
    }
  }

  // start connection and send HTTP header (and body)
  if (equals(HttpMethod, F("HEAD")) || equals(HttpMethod, F("GET"))) {
    httpCode = http.sendRequest(HttpMethod.c_str());
  } else {
    httpCode = http.sendRequest(HttpMethod.c_str(), postStr);
  }

  // Check to see if we need to try digest auth
  if ((httpCode == 401) && must_check_reply) {
    const String authReq = http.header(String(F("WWW-Authenticate")).c_str());

    if (authReq.indexOf(F("Digest")) != -1) {
      // Use Digest authorization
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("HTTP : Start Digest Authorization for "), host));
      }

      http.setAuthorization(""); // Clear Basic authorization
#ifdef ESP32
      http.setAuthorizationType(""); // Default type is "Basic" and "Digest" is already part of the string generated by getDigestAuth()
#endif
      const String authorization = getDigestAuth(authReq, user, pass, F("GET"), uri, 1);

      http.end();
#if defined(CORE_POST_2_6_0) || defined(ESP32)
      http.begin(client, host, port, uri, false); // HTTP, not HTTPS
#else // if defined(CORE_POST_2_6_0) || defined(ESP32)
      http.begin(client, host, port, uri);
#endif // if defined(CORE_POST_2_6_0) || defined(ESP32)

      http.addHeader(F("Authorization"), authorization);

      // start connection and send HTTP header (and body)
      if (equals(HttpMethod, F("HEAD")) || equals(HttpMethod, F("GET"))) {
        httpCode = http.sendRequest(HttpMethod.c_str());
      } else {
        httpCode = http.sendRequest(HttpMethod.c_str(), postStr);
      }
    }
  }

  if (!must_check_reply) {
    // There are services which do not send an ack.
    // So if the return code matches a read timeout, we change it into HTTP code 200
    if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
      httpCode = 200;
    }
  }

  if (Settings.UseRules) {
    // Generate event with the HTTP return code
    // e.g. http#hostname=401
    eventQueue.addMove(strformat(F("http#%s=%d"), host.c_str(), httpCode));
    
    #if FEATURE_THINGSPEAK_EVENT
      // Generate event with the response of a 
      // thingspeak request (https://de.mathworks.com/help/thingspeak/readlastfieldentry.html &
      // https://de.mathworks.com/help/thingspeak/readdata.html)
      // e.g. command for a specific field: "sendToHTTP,api.thingspeak.com,80,/channels/1637928/fields/5/last.csv"
      // command for all fields: "sendToHTTP,api.thingspeak.com,80,/channels/1637928/feeds/last.csv"
      // where first eventvalue is the channel number and the second to the nineth event values 
      // are the field values
      // Example of the event: "EVENT: ThingspeakReply=1637928,5,24.2,12,900,..."
      //                                                  ^    ^ └------┬------┘
      //                                   channel number ┘    |        └ received values
      //                                                   field number (only available for a "single-value-event")
      // In rules you can grep the reply by "On ThingspeakReply Do ..."
      //-----------------------------------------------------------------------------------------------------------------------------
      // 2024-02-05 - Added the option to get a single value of a field or all values of a channel at a certain time (not only the last entry)
      // Examples:
      // Single channel: "sendtohttp,api.thingspeak.com,80,channels/1637928/fields/1.csv?end=2024-01-01%2023:59:00&results=1"
      // => gets the value of field 1 at (or the last entry before) 23:59:00 of the channel 1637928
      // All channels: "sendtohttp,api.thingspeak.com,80,channels/1637928/feeds.csv?end=2024-01-01%2023:59:00&results=1"
      // => gets the value of each field of the channel 1637928 at (or the last entry before) 23:59:00 
      //-----------------------------------------------------------------------------------------------------------------------------

    if (httpCode == 200 && equals(host, F("api.thingspeak.com")) && (uri.endsWith(F("/last.csv")) || (uri.indexOf(F("results=1")) >= 0 && uri.indexOf(F(".csv")) >= 0))){
      String result = http.getString();
      result.replace(' ', '_'); // if using a single field with a certain time, the result contains a space and would break the code
      const int posTimestamp = result.lastIndexOf(':');
      if (posTimestamp >= 0){
        result = parseStringToEndKeepCase(result.substring(posTimestamp), 3);
        if (uri.indexOf(F("fields")) >= 0){                                                                           // when there is a single field call add the field number before the value
          result = parseStringKeepCase(uri, 4, '/').substring(0, 1) + "," + result; // since the field number is always the fourth part of the url and is always a single digit, we can use this to extact the fieldnumber
        }
        eventQueue.addMove(strformat(
            F("ThingspeakReply=%s,%s"),
            parseStringKeepCase(uri, 2, '/').c_str(),
            result.c_str()));
      }
    }
    #endif
  }

#ifndef BUILD_NO_DEBUG
  log_http_result(http, logIdentifier, host + ':' + port, HttpMethod, httpCode, EMPTY_STRING);
#endif
  return httpCode;
}

String send_via_http(const String& logIdentifier,
                     uint16_t      timeout,
                     const String& user,
                     const String& pass,
                     const String& host,
                     uint16_t      port,
                     const String& uri,
                     const String& HttpMethod,
                     const String& header,
                     const String& postStr,
                     int         & httpCode,
                     bool          must_check_reply) {
  WiFiClient client;
  HTTPClient http;
  http.setReuse(false);

  httpCode = http_authenticate(
    logIdentifier,
    client,
    http,
    timeout,
    user,
    pass,
    host,
    port,
    uri,
    HttpMethod,
    header,
    postStr,
    must_check_reply);

  String response;

  if ((httpCode > 0) && must_check_reply) {
    response = http.getString();
#ifndef BUILD_NO_DEBUG
    if (!response.isEmpty()) {
      log_http_result(http, logIdentifier, host, HttpMethod, httpCode, response);
    }
#endif
  }
  http.end();
  // http.end() does not call client.stop() if it is no longer connected.
  // However the client may still keep its internal state which may prevent 
  // future connections to the same host until there has been a connection to another host inbetween.
  client.stop(); 
  return response;
}
#endif // FEATURE_HTTP_CLIENT

#if FEATURE_DOWNLOAD

// FIXME TD-er: Must set the timeout somewhere
# ifndef DOWNLOAD_FILE_TIMEOUT
  #  define DOWNLOAD_FILE_TIMEOUT 2000
# endif // ifndef DOWNLOAD_FILE_TIMEOUT

// Download a file from a given URL and save to a local file named "file_save"
// If the URL ends with a /, the file part will be assumed the same as file_save.
// If file_save is empty, the file part from the URL will be used as local file name.
// Return true when successful.
bool downloadFile(const String& url, String file_save) {
  String error;

  return downloadFile(url, file_save, EMPTY_STRING, EMPTY_STRING, error);
}

// User and Pass may be updated if they occur in the hostname part.
// Thus have to be copied instead of const reference.
bool start_downloadFile(WiFiClient  & client,
                        HTTPClient  & http,
                        const String& url,
                        String      & file_save,
                        String        user,
                        String        pass,
                        String      & error) {
  String   host, file;
  uint16_t port;
  String   uri = splitURL(url, user, pass, host, port, file);

  if (file_save.isEmpty()) {
    file_save = file;
  } else if ((file.isEmpty()) && uri.endsWith("/")) {
    // file = file_save;
    uri += file_save;
  }
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("downloadFile: URL: %s decoded: %s:%d%s"), 
                                          url.c_str(), host.c_str(), port, uri.c_str()));
  }
# endif // ifndef BUILD_NO_DEBUG

  if (file_save.isEmpty()) {
    error = F("Empty filename");
    addLog(LOG_LEVEL_ERROR, error);
    return false;
  }

  const int httpCode = http_authenticate(
    F("DownloadFile"),
    client,
    http,
    DOWNLOAD_FILE_TIMEOUT,
    user,
    pass,
    host,
    port,
    uri,
    F("GET"),
    EMPTY_STRING, // header
    EMPTY_STRING, // postStr
    true          // must_check_reply
    );

  if (httpCode != HTTP_CODE_OK) {
    error  = strformat(F("HTTP code: %d %s"), httpCode, url.c_str());

    addLog(LOG_LEVEL_ERROR, error);
    http.end();
    client.stop();
    return false;
  }
  return true;
}

bool downloadFile(const String& url, String file_save, const String& user, const String& pass, String& error) {
  WiFiClient client;
  HTTPClient http;
  http.setReuse(false);

  if (!start_downloadFile(client, http, url, file_save, user, pass, error)) {
    return false;
  }

  if (fileExists(file_save)) {
    error  = concat(F("File exists: "), file_save);
    addLog(LOG_LEVEL_ERROR, error);
    http.end();
    client.stop();
    return false;
  }

  long len   = http.getSize();
  fs::File f = tryOpenFile(file_save, "w");

  if (f) {
    const size_t downloadBuffSize = 256;
    uint8_t buff[downloadBuffSize];
    size_t  bytesWritten  = 0;
    unsigned long timeout = millis() + DOWNLOAD_FILE_TIMEOUT;

    // get tcp stream
    WiFiClient *stream = &client;

    // read all data from server
    while (http.connected() && (len > 0 || len == -1)) {
      // read up to downloadBuffSize at a time.
      size_t bytes_to_read = downloadBuffSize;

      if ((len > 0) && (len < static_cast<int>(bytes_to_read))) {
        bytes_to_read = len;
      }
      const size_t c = stream->readBytes(buff, bytes_to_read);

      if (c > 0) {
        timeout = millis() + DOWNLOAD_FILE_TIMEOUT;

        if (f.write(buff, c) != c) {
          error  = strformat(F("Error saving file: %s %d Bytes written"), file_save.c_str(), bytesWritten);
          addLog(LOG_LEVEL_ERROR, error);
          http.end();
          client.stop();
          return false;
        }
        bytesWritten += c;

        if (len > 0) { len -= c; }
      }

      if (timeOutReached(timeout)) {
        error  = concat(F("Timeout: "), file_save);
        addLog(LOG_LEVEL_ERROR, error);
        delay(0);
        http.end();
        client.stop();
        return false;
      }
      delay(0);
    }
    f.close();
    http.end();
    client.stop();

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("downloadFile: %s Success"), file_save.c_str()));
    }
    return true;
  }
  http.end();
  client.stop();
  error  = concat(F("Failed to open file for writing: "), file_save);
  addLog(LOG_LEVEL_ERROR, error);
  return false;
}

bool downloadFirmware(String filename, String& error)
{
  String baseurl, user, pass;
# if FEATURE_CUSTOM_PROVISIONING
  MakeProvisioningSettings(ProvisioningSettings);

  if (ProvisioningSettings.get()) {
    loadProvisioningSettings(*ProvisioningSettings);
    if (!ProvisioningSettings->allowedFlags.allowFetchFirmware) {
      error = F("Not Allowed");
      return false;
    }
    baseurl = ProvisioningSettings->url;
    user = ProvisioningSettings->user;
    pass = ProvisioningSettings->pass;
  }
# endif // if FEATURE_CUSTOM_PROVISIONING

  const String fullUrl = joinUrlFilename(baseurl, filename);

  return downloadFirmware(fullUrl, filename, user, pass, error);
}

bool downloadFirmware(const String& url, String& file_save, String& user, String& pass, String& error)
{
  WiFiClient client;
  HTTPClient http;
  error.clear();

  if (!start_downloadFile(client, http, url, file_save, user, pass, error)) {
    return false;
  }

  int len = http.getSize();

  if (Update.begin(len, U_FLASH, Settings.Pin_status_led, Settings.Pin_status_led_Inversed ? LOW : HIGH)) {
    const size_t downloadBuffSize = 256;
    uint8_t buff[downloadBuffSize];
    size_t  bytesWritten  = 0;
    unsigned long timeout = millis() + DOWNLOAD_FILE_TIMEOUT;

    // get tcp stream
    WiFiClient *stream = &client;

    while (error.isEmpty() && http.connected() && (len > 0 || len == -1)) {
      // read up to downloadBuffSize at a time.
      size_t bytes_to_read = downloadBuffSize;

      if ((len > 0) && (len < static_cast<int>(bytes_to_read))) {
        bytes_to_read = len;
      }
      const size_t c = stream->readBytes(buff, bytes_to_read);

      if (c > 0) {
        timeout = millis() + DOWNLOAD_FILE_TIMEOUT;

        if (Update.write(buff, c) != c) {
          error  = strformat(F("Error saving firmware update: %s %d Bytes written"),
                             file_save.c_str(), bytesWritten);
          break;
        }
        bytesWritten += c;

        if (len > 0) { len -= c; }
      }

      if (timeOutReached(timeout)) {
        error  = concat(F("Timeout: "), file_save);
        break;
      }

      if (!UseRTOSMultitasking) {
        // On ESP32 the schedule is executed on the 2nd core.
        Scheduler.handle_schedule();
      }
      backgroundtasks();
    }
  }
  http.end();
  client.stop();

  if (error.isEmpty() && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("downloadFile: %s Success"), file_save.c_str()));
  }

  uint8_t errorcode = 0;
  if (!Update.end()) {
    errorcode = Update.getError();
#ifdef ESP32
    const __FlashStringHelper * err_fstr = F("Unknown");
    switch (errorcode) {
      case UPDATE_ERROR_OK:                  err_fstr = F("OK");           break;
      case UPDATE_ERROR_WRITE:               err_fstr = F("WRITE");        break;
      case UPDATE_ERROR_ERASE:               err_fstr = F("ERASE");        break;
      case UPDATE_ERROR_READ:                err_fstr = F("READ");         break;
      case UPDATE_ERROR_SPACE:               err_fstr = F("SPACE");        break;
      case UPDATE_ERROR_SIZE:                err_fstr = F("SIZE");         break;
      case UPDATE_ERROR_STREAM:              err_fstr = F("STREAM");       break;
      case UPDATE_ERROR_MD5:                 err_fstr = F("MD5");          break;
      case UPDATE_ERROR_MAGIC_BYTE:          err_fstr = F("MAGIC_BYTE");   break;
      case UPDATE_ERROR_ACTIVATE:            err_fstr = F("ACTIVATE");     break;
      case UPDATE_ERROR_NO_PARTITION:        err_fstr = F("NO_PARTITION"); break;
      case UPDATE_ERROR_BAD_ARGUMENT:        err_fstr = F("BAD_ARGUMENT"); break;
      case UPDATE_ERROR_ABORT:               err_fstr = F("ABORT");        break;
    }
    error += concat(F(" Error: "), err_fstr);
#else
    error += concat(F(" Error: "), errorcode);
#endif
  } else {
    if (Settings.UseRules) {
      eventQueue.addMove(concat(F("ProvisionFirmware#success="), file_save));
    }
    return true;
  }

  backgroundtasks();
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    addLog(LOG_LEVEL_ERROR, concat(F("Failed update firmware: "), error));
  }

  if (Settings.UseRules) {
    eventQueue.addMove(concat(F("ProvisionFirmware#failed="), file_save));
  }
  return false;
}

String joinUrlFilename(const String& url, String& filename)
{
  String fullUrl;

  fullUrl.reserve(url.length() + filename.length() + 1); // May need to add an extra slash
  fullUrl = url;
  fullUrl = parseTemplate(fullUrl, true);                // URL encode

  // URLEncode may also encode the '/' into "%2f"
  // FIXME TD-er: Can this really occur?
  fullUrl.replace(F("%2f"), F("/"));

  while (filename.startsWith(F("/"))) {
    filename = filename.substring(1);
  }

  if (!fullUrl.endsWith(F("/"))) {
    fullUrl += F("/");
  }
  fullUrl += filename;
  return fullUrl;
}

#endif // if FEATURE_DOWNLOAD

