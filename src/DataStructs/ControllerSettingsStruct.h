#ifndef DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
#define DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H

/*********************************************************************************************\
 * ControllerSettingsStruct definition
\*********************************************************************************************/
#include <Arduino.h>
class IPAddress;
class WiFiClient;
class WiFiUDP;

// Minimum delay between messages for a controller to send in msec.
#define CONTROLLER_DELAY_QUEUE_DELAY_MAX   3600000
#define CONTROLLER_DELAY_QUEUE_DELAY_DFLT  100
// Queue length for controller messages not yet sent.
#define CONTROLLER_DELAY_QUEUE_DEPTH_MAX   50
#define CONTROLLER_DELAY_QUEUE_DEPTH_DFLT  10
// Number of retries to send a message by a controller.
// N.B. Retries without a connection to wifi do not count as retry.
#define CONTROLLER_DELAY_QUEUE_RETRY_MAX   10
#define CONTROLLER_DELAY_QUEUE_RETRY_DFLT  10
// Timeout of the client in msec.
#define CONTROLLER_CLIENTTIMEOUT_MAX     1000
#define CONTROLLER_CLIENTTIMEOUT_DFLT     100


struct ControllerSettingsStruct
{
  ControllerSettingsStruct();

  void reset();

  void validate();

  IPAddress getIP() const;

  String getHost() const;

  void setHostname(const String& controllerhostname);

  boolean checkHostReachable(bool quick);

  boolean connectToHost(WiFiClient &client);

  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  int beginPacket(WiFiUDP &client);

  String getHostPortString() const;

  boolean       UseDNS;
  byte          IP[4];
  unsigned int  Port;
  char          HostName[65];
  char          Publish[129];
  char          Subscribe[129];
  char          MQTTLwtTopic[129];
  char          LWTMessageConnect[129];
  char          LWTMessageDisconnect[129];
  unsigned int  MinimalTimeBetweenMessages;
  unsigned int  MaxQueueDepth;
  unsigned int  MaxRetry;
  boolean       DeleteOldest; // Action to perform when buffer full, delete oldest, or ignore newest.
  unsigned int  ClientTimeout;
  boolean       MustCheckReply; // When set to false, a sent message is considered always successful.
  uint8_t       SampleSetInitiator; // The first plugin to start a sample set.

private:

  bool ipSet();

  bool updateIPcache();
};

typedef std::shared_ptr<ControllerSettingsStruct> ControllerSettingsStruct_ptr_type;
#define MakeControllerSettings(T) ControllerSettingsStruct_ptr_type ControllerSettingsStruct_ptr(new ControllerSettingsStruct());\
                                    ControllerSettingsStruct& T = *ControllerSettingsStruct_ptr;
                                    
#endif // DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H