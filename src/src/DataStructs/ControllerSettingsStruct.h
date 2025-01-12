#ifndef DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
#define DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H

/*********************************************************************************************\
* ControllerSettingsStruct definition
\*********************************************************************************************/
#include "../../ESPEasy_common.h"

#include <memory> // For std::shared_ptr
#include <new>    // for std::nothrow

#include <IPAddress.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "../DataStructs/ChecksumType.h"

#if FEATURE_MQTT_TLS
#include "../DataTypes/TLS_types.h"
#endif

#include "../Globals/Plugins.h"

// Minimum delay between messages for a controller to send in msec.
#ifndef CONTROLLER_DELAY_QUEUE_DELAY_MAX
# define CONTROLLER_DELAY_QUEUE_DELAY_MAX   3600000
#endif // ifndef CONTROLLER_DELAY_QUEUE_DELAY_MAX
#ifndef CONTROLLER_DELAY_QUEUE_DELAY_DFLT
# define CONTROLLER_DELAY_QUEUE_DELAY_DFLT  100
#endif // ifndef CONTROLLER_DELAY_QUEUE_DELAY_DFLT

// Queue length for controller messages not yet sent.
#ifndef CONTROLLER_DELAY_QUEUE_DEPTH_MAX
# define CONTROLLER_DELAY_QUEUE_DEPTH_MAX   50
#endif // ifndef CONTROLLER_DELAY_QUEUE_DEPTH_MAX
#ifndef CONTROLLER_DELAY_QUEUE_DEPTH_DFLT
# define CONTROLLER_DELAY_QUEUE_DEPTH_DFLT  10
#endif // ifndef CONTROLLER_DELAY_QUEUE_DEPTH_DFLT

// Number of retries to send a message by a controller.
// N.B. Retries without a connection to wifi do not count as retry.
#ifndef CONTROLLER_DELAY_QUEUE_RETRY_MAX
# define CONTROLLER_DELAY_QUEUE_RETRY_MAX   10
#endif // ifndef CONTROLLER_DELAY_QUEUE_RETRY_MAX
#ifndef CONTROLLER_DELAY_QUEUE_RETRY_DFLT
# define CONTROLLER_DELAY_QUEUE_RETRY_DFLT  10
#endif // ifndef CONTROLLER_DELAY_QUEUE_RETRY_DFLT

// Timeout of the client in msec.
#ifndef CONTROLLER_CLIENTTIMEOUT_MAX
# define CONTROLLER_CLIENTTIMEOUT_MAX     4000 // Not sure if this may trigger SW watchdog.
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_MAX
#ifndef CONTROLLER_CLIENTTIMEOUT_DFLT
# define CONTROLLER_CLIENTTIMEOUT_DFLT     100
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_DFLT

// MQTT Keep Alive Timeout
#ifndef CONTROLLER_KEEP_ALIVE_TIME_MAX
# define CONTROLLER_KEEP_ALIVE_TIME_MAX    65535
#endif // ifndef CONTROLLER_KEEP_ALIVE_TIME_MAX
#ifndef CONTROLLER_KEEP_ALIVE_TIME_DFLT
# define CONTROLLER_KEEP_ALIVE_TIME_DFLT      60
#endif // ifndef CONTROLLER_KEEP_ALIVE_TIME_DFLT

#ifndef CONTROLLER_DEFAULT_CLIENTID
# define CONTROLLER_DEFAULT_CLIENTID  "%sysname%_%unit%"
#endif // ifndef CONTROLLER_DEFAULT_CLIENTID

struct ControllerSettingsStruct
{
  // ********************************************************************************
  //   IDs of controller settings, used to generate web forms
  // ********************************************************************************
  enum VarType {
    CONTROLLER_USE_DNS                  = 0, // PLace this before HOSTNAME/IP
    CONTROLLER_USE_EXTENDED_CREDENTIALS = 1, // Place this before USER/PASS
    CONTROLLER_HOSTNAME,
    CONTROLLER_IP,
    CONTROLLER_PORT,
#if FEATURE_MQTT_TLS
    CONTROLLER_MQTT_TLS_TYPE,
    CONTROLLER_MQTT_TLS_STORE_FINGERPRINT,
    CONTROLLER_MQTT_TLS_STORE_CERT,
    CONTROLLER_MQTT_TLS_STORE_CACERT,
#endif
    CONTROLLER_USER,
    CONTROLLER_PASS,
    CONTROLLER_MIN_SEND_INTERVAL,
    CONTROLLER_MAX_QUEUE_DEPTH,
    CONTROLLER_MAX_RETRIES,
    CONTROLLER_FULL_QUEUE_ACTION,
    CONTROLLER_ALLOW_EXPIRE,
    CONTROLLER_DEDUPLICATE,
    CONTROLLER_USE_LOCAL_SYSTEM_TIME,
    CONTROLLER_CHECK_REPLY,
    CONTROLLER_CLIENT_ID,
#if FEATURE_MQTT
    CONTROLLER_UNIQUE_CLIENT_ID_RECONNECT,
    CONTROLLER_RETAINFLAG,
#endif
    CONTROLLER_SUBSCRIBE,
    CONTROLLER_PUBLISH,
#if FEATURE_MQTT
    CONTROLLER_LWT_TOPIC,
    CONTROLLER_LWT_CONNECT_MESSAGE,
    CONTROLLER_LWT_DISCONNECT_MESSAGE,
    CONTROLLER_SEND_LWT,
    CONTROLLER_WILL_RETAIN,
    CONTROLLER_CLEAN_SESSION,
    CONTROLLER_KEEP_ALIVE_TIME,
#endif
    CONTROLLER_TIMEOUT,
    CONTROLLER_SAMPLE_SET_INITIATOR,
    CONTROLLER_SEND_BINARY,

    // Keep this as last, is used to loop over all parameters
    CONTROLLER_ENABLED
  };


  ControllerSettingsStruct();

  void         reset();

  bool         isSet() const;

  void         validate();

  ChecksumType computeChecksum() const {
    return ChecksumType(reinterpret_cast<const uint8_t *>(this), sizeof(ControllerSettingsStruct));
  }

  IPAddress    getIP() const {
    return IPAddress(IP[0], IP[1], IP[2], IP[3]);
  }

  String       getHost() const;

  void         setHostname(const String& controllerhostname);

  bool         checkHostReachable(bool quick);

  #if FEATURE_HTTP_CLIENT
  bool         connectToHost(WiFiClient& client);
  #endif // FEATURE_HTTP_CLIENT

  bool         beginPacket(WiFiUDP& client);

  String       getHostPortString() const;

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  bool         mqtt_cleanSession() const { return VariousBits1.mqtt_cleanSession; }
  void         mqtt_cleanSession(bool value) { VariousBits1.mqtt_cleanSession = value; }

  bool         mqtt_sendLWT() const { return !VariousBits1.mqtt_not_sendLWT; }
  void         mqtt_sendLWT(bool value) { VariousBits1.mqtt_not_sendLWT = !value; }

  bool         mqtt_willRetain() const { return !VariousBits1.mqtt_not_willRetain; }
  void         mqtt_willRetain(bool value) { VariousBits1.mqtt_not_willRetain = !value; }

  bool         mqtt_uniqueMQTTclientIdReconnect() const { return VariousBits1.mqtt_uniqueMQTTclientIdReconnect; }
  void         mqtt_uniqueMQTTclientIdReconnect(bool value) { VariousBits1.mqtt_uniqueMQTTclientIdReconnect = value; }

  bool         mqtt_retainFlag() const { return VariousBits1.mqtt_retainFlag; }
  void         mqtt_retainFlag(bool value) { VariousBits1.mqtt_retainFlag = value; }

  bool         useExtendedCredentials() const { return VariousBits1.useExtendedCredentials; }
  void         useExtendedCredentials(bool value) { VariousBits1.useExtendedCredentials = value; }

  bool         sendBinary() const { return VariousBits1.sendBinary; }
  void         sendBinary(bool value) { VariousBits1.sendBinary = value; }

  bool         allowExpire() const { return VariousBits1.allowExpire; }
  void         allowExpire(bool value) { VariousBits1.allowExpire = value; }

  bool         deduplicate() const { return VariousBits1.deduplicate; }
  void         deduplicate(bool value) { VariousBits1.deduplicate = value; }

  bool         useLocalSystemTime() const { return VariousBits1.useLocalSystemTime; }
  void         useLocalSystemTime(bool value) { VariousBits1.useLocalSystemTime = value; }

#if FEATURE_MQTT_TLS
  TLS_types TLStype() const { return static_cast<TLS_types>(VariousBits1.TLStype); }
  void      TLStype(TLS_types tls_type) { VariousBits1.TLStype = static_cast<uint8_t>(tls_type); }

  String    getCertificateFilename() const;
  String    getCertificateFilename(TLS_types tls_type) const;
#endif
  

  bool         UseDNS;
  uint8_t      IP[4];
  uint8_t      UNUSED_1[3];
  unsigned int Port;
  char         HostName[65];
  char         Publish[129];
  char         Subscribe[129];
  char         MQTTLwtTopic[129];
  char         LWTMessageConnect[129];
  char         LWTMessageDisconnect[129];
  uint8_t      UNUSED_2[2];
  unsigned int MinimalTimeBetweenMessages;
  unsigned int MaxQueueDepth;
  unsigned int MaxRetry;
  bool         DeleteOldest;       // Action to perform when buffer full, delete oldest, or ignore newest.
  uint8_t      UNUSED_3[3];
  unsigned int ClientTimeout;
  bool         MustCheckReply;     // When set to false, a sent message is considered always successful.
  taskIndex_t  SampleSetInitiator; // The first task to start a sample set.
  uint16_t     KeepAliveTime;      // The configured Keep Alive time in seconds, can be 0 (disabled) to 65535 (18+ hours)

  struct {
    uint32_t unused_00                        : 1; // Bit 00
    uint32_t mqtt_cleanSession                : 1; // Bit 01
    uint32_t mqtt_not_sendLWT                 : 1; // Bit 02, !value, default enabled
    uint32_t mqtt_not_willRetain              : 1; // Bit 03, !value, default enabled
    uint32_t mqtt_uniqueMQTTclientIdReconnect : 1; // Bit 04
    uint32_t mqtt_retainFlag                  : 1; // Bit 05
    uint32_t useExtendedCredentials           : 1; // Bit 06
    uint32_t sendBinary                       : 1; // Bit 07
    uint32_t unused_08                        : 1; // Bit 08
    uint32_t allowExpire                      : 1; // Bit 09
    uint32_t deduplicate                      : 1; // Bit 10
    uint32_t useLocalSystemTime               : 1; // Bit 11
    uint32_t TLStype                          : 4; // Bit 12...15: TLS type
    uint32_t unused_16                        : 1; // Bit 16
    uint32_t unused_17                        : 1; // Bit 17
    uint32_t unused_18                        : 1; // Bit 18
    uint32_t unused_19                        : 1; // Bit 19
    uint32_t unused_20                        : 1; // Bit 20
    uint32_t unused_21                        : 1; // Bit 21
    uint32_t unused_22                        : 1; // Bit 22
    uint32_t unused_23                        : 1; // Bit 23
    uint32_t unused_24                        : 1; // Bit 24
    uint32_t unused_25                        : 1; // Bit 25
    uint32_t unused_26                        : 1; // Bit 26
    uint32_t unused_27                        : 1; // Bit 27
    uint32_t unused_28                        : 1; // Bit 28
    uint32_t unused_29                        : 1; // Bit 29
    uint32_t unused_30                        : 1; // Bit 30
    uint32_t unused_31                        : 1; // Bit 31
  }    VariousBits1;
  char ClientID[65];                                 // Used to define the Client ID used by the controller

private:

  bool ipSet() const;

  bool updateIPcache();
};

#include "../Helpers/Memory.h"

typedef std::shared_ptr<ControllerSettingsStruct> ControllerSettingsStruct_ptr_type;
/*
# ifdef USE_SECOND_HEAP
#define MakeControllerSettings(T) HeapSelectIram ephemeral; ControllerSettingsStruct_ptr_type T(new (std::nothrow)  ControllerSettingsStruct());
#else
*/
#define MakeControllerSettings(T) void * calloc_ptr = special_calloc(1,sizeof(ControllerSettingsStruct)); ControllerSettingsStruct_ptr_type T(new (calloc_ptr)  ControllerSettingsStruct());
//#endif

// Check to see if MakeControllerSettings was successful
#define AllocatedControllerSettings() (ControllerSettings.get() != nullptr)

#endif // DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
