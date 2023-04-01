#ifndef HELPERS_NETWORKING_H
#define HELPERS_NETWORKING_H

#include "../../ESPEasy_common.h"

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#if FEATURE_HTTP_CLIENT
#ifdef ESP8266
# include <ESP8266HTTPClient.h>
#endif // ifdef ESP8266
#ifdef ESP32
# include <HTTPClient.h>
#endif // ifdef ESP32
#endif


/*********************************************************************************************\
   Syslog client
\*********************************************************************************************/
void sendSyslog(uint8_t logLevel, const String& message);


#if FEATURE_ESPEASY_P2P

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
   Send event using UDP message to specific unit
\*********************************************************************************************/
void SendUDPCommand(uint8_t destUnit, const char *data, uint8_t dataLength);

/*********************************************************************************************\
   Get formatted IP address for unit
   formatcodes: 0 = default toString(), 1 = empty string when invalid, 2 = 0 when invalid
\*********************************************************************************************/
String formatUnitToIPAddress(uint8_t unit, uint8_t formatCode);

/*********************************************************************************************\
   Get IP address for specific unit
\*********************************************************************************************/
IPAddress getIPAddressForUnit(uint8_t unit);

/*********************************************************************************************\
   Send UDP message to specific unit (unit 255=broadcast)
\*********************************************************************************************/
void sendUDP(uint8_t unit, const uint8_t *data, uint8_t size);

/*********************************************************************************************\
   Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList();

/*********************************************************************************************\
   Broadcast system info to other nodes. (to update node lists)
\*********************************************************************************************/
void sendSysInfoUDP(uint8_t repeats);
#endif //FEATURE_ESPEASY_P2P


#if defined(ESP8266)

# if FEATURE_SSDP

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
void SSDP_send(uint8_t method);

/********************************************************************************************\
   SSDP message processing
 \*********************************************************************************************/
void SSDP_update();

# endif // if FEATURE_SSDP
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

bool useStaticIP();

// Check connection. Maximum timeout 500 msec.
bool NetworkConnected(uint32_t timeout_ms);

bool hostReachable(const IPAddress& ip);

#if FEATURE_HTTP_CLIENT
bool connectClient(WiFiClient& client, const char *hostname, uint16_t port, uint32_t timeout_ms = 100);

bool connectClient(WiFiClient& client, IPAddress ip, uint16_t port, uint32_t timeout_ms = 100);
#endif // FEATURE_HTTP_CLIENT

void scrubDNS();

bool valid_DNS_address(const IPAddress& dns);

bool setDNS(int index, const IPAddress& dns);

bool resolveHostByName(const char *aHostname, IPAddress& aResult, uint32_t timeout_ms = 1000);

bool hostReachable(const String& hostname);

// Create a random port for the UDP connection.
// Return true when successful.
bool beginWiFiUDP_randomPort(WiFiUDP& udp);

void sendGratuitousARP();


bool splitHostPortString(const String& hostPortString, String& host, uint16_t& port);

// Split the username and password from a string like this:
// username:password@hostname:portnr
// @param  hostPortString  The string to parse
// @param  user The found username (if any)
// @param  pass The found password (if any)
// @param  hostname The hostname stripped from any of the other possible parameters
// @param  port The found portname (defaults to 80 when not specified)
// @retval Whether supplied hostPortString was valid.
bool splitUserPass_HostPortString(const String& hostPortString, String& user, String& pass, String& host, uint16_t& port);

// Split a full URL like "http://hostname:port/path/file.htm"
// Return value is everything after the hostname:port section (including /)
String splitURL(const String& fullURL, String& user, String& pass, String& host, uint16_t& port, String& file);


#if FEATURE_HTTP_CLIENT
// Initiate the HTTP connection.
// Also try to authenticate using either Basic auth or Digest.
// @retval HTTP return code.
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
                      bool          must_check_reply);


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
                     bool          must_check_reply);
#endif // FEATURE_HTTP_CLIENT

#if FEATURE_DOWNLOAD

// Download a file from a given URL and save to a local file named "file_save"
// If the URL ends with a /, the file part will be assumed the same as file_save.
// If file_save is empty, the file part from the URL will be used as local file name.
// Return true when successful.
bool downloadFile(String file_save, String error);

bool downloadFile(const String& url, String file_save, const String& user, const String& pass, String& error);

bool downloadFirmware(String filename, String& error);
bool downloadFirmware(const String& url, String& file_save, String& user, String& pass, String& error);

// Return the full url including filename
String joinUrlFilename(const String& url, String& filename);

#endif // if FEATURE_DOWNLOAD






#endif