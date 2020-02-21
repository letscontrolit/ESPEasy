#ifndef DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
#define DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H

/*********************************************************************************************\
* ControllerSettingsStruct definition
\*********************************************************************************************/
#include <Arduino.h>
#include <memory> // For std::shared_ptr

#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

class IPAddress;
class WiFiClient;
class WiFiUDP;

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
# define CONTROLLER_CLIENTTIMEOUT_MAX     1000
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_MAX
#ifndef CONTROLLER_CLIENTTIMEOUT_DFLT
# define CONTROLLER_CLIENTTIMEOUT_DFLT     100
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_DFLT


// ********************************************************************************
//   IDs of controller settings, used to generate web forms
// ********************************************************************************

#define CONTROLLER_USE_DNS                  1
#define CONTROLLER_HOSTNAME                 2
#define CONTROLLER_IP                       3
#define CONTROLLER_PORT                     4
#define CONTROLLER_USER                     5
#define CONTROLLER_PASS                     6
#define CONTROLLER_MIN_SEND_INTERVAL        7
#define CONTROLLER_MAX_QUEUE_DEPTH          8
#define CONTROLLER_MAX_RETRIES              9
#define CONTROLLER_FULL_QUEUE_ACTION        10
#define CONTROLLER_CHECK_REPLY              12
#define CONTROLLER_SUBSCRIBE                13
#define CONTROLLER_PUBLISH                  14
#define CONTROLLER_LWT_TOPIC                15
#define CONTROLLER_LWT_CONNECT_MESSAGE      16
#define CONTROLLER_LWT_DISCONNECT_MESSAGE   17
#define CONTROLLER_SEND_LWT                 18
#define CONTROLLER_WILL_RETAIN              19
#define CONTROLLER_CLEAN_SESSION            20
#define CONTROLLER_TIMEOUT                  21
#define CONTROLLER_SAMPLE_SET_INITIATOR     22
#define CONTROLLER_ENABLED                  23 // Keep this as last, is used to loop over all parameters

struct ControllerSettingsStruct
{
  ControllerSettingsStruct();

  void      reset();

  void      validate();

  IPAddress getIP() const;

  String    getHost() const;

  void      setHostname(const String& controllerhostname);

  boolean   checkHostReachable(bool quick);

  boolean   connectToHost(WiFiClient& client);

  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  int       beginPacket(WiFiUDP& client);

  String    getHostPortString() const;

  // MQTT_flags defaults to 0, keep in mind when adding bit lookups.
  bool mqtt_cleanSession() const;
  void mqtt_cleanSession(bool value);

  bool mqtt_sendLWT() const;
  void mqtt_sendLWT(bool value);

  bool mqtt_willRetain() const;
  void mqtt_willRetain(bool value);

  boolean      UseDNS;
  byte         IP[4];
  unsigned int Port;
  char         HostName[65];
  char         Publish[129];
  char         Subscribe[129];
  char         MQTTLwtTopic[129];
  char         LWTMessageConnect[129];
  char         LWTMessageDisconnect[129];
  unsigned int MinimalTimeBetweenMessages;
  unsigned int MaxQueueDepth;
  unsigned int MaxRetry;
  boolean      DeleteOldest;       // Action to perform when buffer full, delete oldest, or ignore newest.
  unsigned int ClientTimeout;
  boolean      MustCheckReply;     // When set to false, a sent message is considered always successful.
  taskIndex_t  SampleSetInitiator; // The first task to start a sample set.
  uint32_t     MQTT_flags;         // Various flags for MQTT controllers

private:

  bool ipSet();

  bool updateIPcache();
};

typedef std::shared_ptr<ControllerSettingsStruct> ControllerSettingsStruct_ptr_type;
#define MakeControllerSettings(T) ControllerSettingsStruct_ptr_type ControllerSettingsStruct_ptr(new ControllerSettingsStruct()); \
  ControllerSettingsStruct& (T) = *ControllerSettingsStruct_ptr;

#endif // DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
