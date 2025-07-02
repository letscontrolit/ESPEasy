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


  bool useCredentials() const {
    return usesAccount || usesPassword;
  }

  bool useExtendedCredentials() const {
    return usesExtCreds && useCredentials();
  }

  uint16_t defaultPort{};
  struct {
    uint32_t usesMQTT             : 1;
    uint32_t usesAccount          : 1;
    uint32_t usesPassword         : 1;
    uint32_t usesTemplate         : 1; // When set, the protocol will pre-load some templates like default MQTT topics
    uint32_t usesID               : 1; // Whether a controller supports sending an IDX value sent along with plugin data
    uint32_t Custom               : 1; // When set, the controller has to define all parameters on the controller setup page
    uint32_t usesHost             : 1;
    uint32_t usesPort             : 1;
    uint32_t usesQueue            : 1;
    uint32_t usesCheckReply       : 1;
    uint32_t usesTimeout          : 1;
    uint32_t usesSampleSets       : 1;
    uint32_t usesExtCreds         : 1;
    uint32_t needsNetwork         : 1;
    uint32_t allowsExpire         : 1;
    uint32_t allowLocalSystemTime : 1;
#if FEATURE_MQTT_TLS
    uint32_t usesTLS              : 1; // May offer TLS related settings and options
#else
    uint32_t dontUseBit16         : 1;
#endif
#if FEATURE_MQTT_DISCOVER
    uint32_t mqttAutoDiscover     : 1; // Enable MQTT Auto Discovery (MQTT server-specific)
#else
    uint32_t dontUseBit17         : 1;
#endif
#if FEATURE_STRING_VARIABLES
    uint32_t allowSendDerived     : 1;
#else
    uint32_t dontUseBit18         : 1;
#endif
    uint32_t dummy19              : 1;
    uint32_t dummy20              : 1;
    uint32_t dummy21              : 1;
    uint32_t dummy22              : 1;
    uint32_t dummy23              : 1;
    uint32_t dummy24              : 1;
    uint32_t dummy25              : 1;
    uint32_t dummy26              : 1;
    uint32_t dummy27              : 1;
    uint32_t dummy28              : 1;
    uint32_t dummy29              : 1;
    uint32_t dummy30              : 1;
    uint32_t dummy31              : 1; // These dummy flags can be used when needed, though should get a useful name
  };

//  uint8_t Number{};
};


#endif // DATASTRUCTS_PROTOCOLSTRUCT_H
