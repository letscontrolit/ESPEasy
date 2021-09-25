#ifndef DATASTRUCTS_PROTOCOLSTRUCT_H
#define DATASTRUCTS_PROTOCOLSTRUCT_H

#include "../../ESPEasy_common.h"
#include <vector>

/*********************************************************************************************\
* ProtocolStruct
\*********************************************************************************************/
struct ProtocolStruct
{
  ProtocolStruct();

  bool useCredentials() const;

  bool useExtendedCredentials() const;

  uint16_t defaultPort;
  uint8_t  Number;
  bool     usesMQTT             : 1; // Indicating whether it is a MQTT controller
  bool     usesAccount          : 1; // Offer to enter credentials
  bool     usesPassword         : 1;
  bool     usesTemplate         : 1; // When set, the protocol will pre-load some templates like default MQTT topics
  bool     usesID               : 1; // Whether a controller supports sending an IDX value sent along with plugin data
  bool     Custom               : 1; // When set, the controller has to define all parameters on the controller setup page
  bool     usesHost             : 1; // Offer either DNS hostname or IP
  bool     usesPort             : 1; // Offer to set a port nr. This can be network port, but depending on controller this may be a
                                     // different type of port. See LoRaWAN for example.
  bool     usesQueue            : 1; // Allow to queue messages
  bool     usesCheckReply       : 1; // Allow optional wait for reply
  bool     usesTimeout          : 1; // Offer to set a timeout.
  bool     usesSampleSets       : 1; // A sample set is an extra counter which is incremented as soon as a new value of set task is seen.
                                     // (to keep track of bursts of messages where some may be lost)
  bool     usesExtCreds         : 1; // Offer to store longer credentials
  bool     needsNetwork         : 1; // Whether it needs a network connection to work
  bool     allowsExpire         : 1; // Whether queued messages may be removed from the queue after some time
  bool     allowLocalSystemTime : 1; // Allow switching between Unix time and local time (including timezone and DST)
  bool     usesTLS              : 1; // May offer TLS related settings and options
};

typedef std::vector<ProtocolStruct> ProtocolVector;


#endif // DATASTRUCTS_PROTOCOLSTRUCT_H
