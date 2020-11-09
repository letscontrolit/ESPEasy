#ifndef HELPERS_NETWORKING_H
#define HELPERS_NETWORKING_H

#include "../../ESPEasy_common.h"


#include <WiFiClient.h>
#include <WiFiUdp.h>

/*********************************************************************************************\
   Syslog client
\*********************************************************************************************/
void syslog(byte logLevel, const char *message);


/*********************************************************************************************\
   Update UDP port (ESPEasy propiertary protocol)
\*********************************************************************************************/
void updateUDPport();


/*********************************************************************************************\
   Check UDP messages (ESPEasy propiertary protocol)
\*********************************************************************************************/
extern boolean runningUPDCheck;
void checkUDP();

/*********************************************************************************************\
   Send event using UDP message
\*********************************************************************************************/
void SendUDPCommand(byte destUnit, const char *data, byte dataLength);

/*********************************************************************************************\
   Get formatted IP address for unit
   formatcodes: 0 = default toString(), 1 = empty string when invalid, 2 = 0 when invalid
\*********************************************************************************************/
String formatUnitToIPAddress(byte unit, byte formatCode);

/*********************************************************************************************\
   Get IP address for unit
\*********************************************************************************************/
IPAddress getIPAddressForUnit(byte unit);

/*********************************************************************************************\
   Send UDP message (unit 255=broadcast)
\*********************************************************************************************/
void sendUDP(byte unit, const byte *data, byte size);

/*********************************************************************************************\
   Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList();

/*********************************************************************************************\
   Broadcast system info to other nodes. (to update node lists)
\*********************************************************************************************/
void sendSysInfoUDP(byte repeats);


#if defined(ESP8266)

# ifdef USES_SSDP

/********************************************************************************************\
   Respond to HTTP XML requests for SSDP information
 \*********************************************************************************************/
void SSDP_schema(WiFiClient& client);

/********************************************************************************************\
   Global SSDP stuff
 \*********************************************************************************************/
typedef enum {
  NONE,
  SEARCH,
  NOTIFY
} ssdp_method_t;

extern UdpContext *_server;

extern IPAddress _respondToAddr;
extern uint16_t  _respondToPort;

extern bool _pending;
extern unsigned short _delay;
extern unsigned long  _process_time;
extern unsigned long  _notify_time;

#  define SSDP_INTERVAL     1200
#  define SSDP_PORT         1900
#  define SSDP_METHOD_SIZE  10
#  define SSDP_URI_SIZE     2
#  define SSDP_BUFFER_SIZE  64
#  define SSDP_MULTICAST_TTL 2


/********************************************************************************************\
   Launch SSDP listener and send initial notify
 \*********************************************************************************************/
bool SSDP_begin();

/********************************************************************************************\
   Send SSDP messages (notify & responses)
 \*********************************************************************************************/
void SSDP_send(byte method);

/********************************************************************************************\
   SSDP message processing
 \*********************************************************************************************/
void SSDP_update();

# endif // ifdef USES_SSDP
#endif // if defined(ESP8266)


// ********************************************************************************
// Return subnet range of WiFi.
// ********************************************************************************
bool getSubnetRange(IPAddress& low, IPAddress& high);

// ********************************************************************************
// Functions to test and handle network/client connectivity.
// ********************************************************************************

#ifdef CORE_POST_2_5_0
# include <AddrList.h>
#endif // ifdef CORE_POST_2_5_0


bool hasIPaddr();

// Check connection. Maximum timeout 500 msec.
bool NetworkConnected(uint32_t timeout_ms);

bool hostReachable(const IPAddress& ip);

bool connectClient(WiFiClient& client, const char *hostname, uint16_t port);

bool connectClient(WiFiClient& client, IPAddress ip, uint16_t port);

bool resolveHostByName(const char *aHostname, IPAddress& aResult);

bool hostReachable(const String& hostname);

// Create a random port for the UDP connection.
// Return true when successful.
bool beginWiFiUDP_randomPort(WiFiUDP& udp);

void sendGratuitousARP();

bool splitHostPortString(const String& hostPortString, String& host, uint16_t& port);

// Split a full URL like "http://hostname:port/path/file.htm"
// Return value is everything after the hostname:port section (including /)
String splitURL(const String& fullURL, String& host, uint16_t& port, String& file);

#ifdef USE_SETTINGS_ARCHIVE

// Download a file from a given URL and save to a local file named "file_save"
// If the URL ends with a /, the file part will be assumed the same as file_save.
// If file_save is empty, the file part from the URL will be used as local file name.
// Return true when successful.
bool downloadFile(const String& url, String file_save);

bool downloadFile(const String& url, String file_save, const String& user, const String& pass, String& error);

#endif // USE_SETTINGS_ARCHIVE






#endif