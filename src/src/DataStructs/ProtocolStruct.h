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
  union {
    struct {
      uint16_t usesMQTT             : 1;
      uint16_t usesAccount          : 1;
      uint16_t usesPassword         : 1;
      uint16_t usesTemplate         : 1; // When set, the protocol will pre-load some templates like default MQTT topics
      uint16_t usesID               : 1; // Whether a controller supports sending an IDX value sent along with plugin data
      uint16_t Custom               : 1; // When set, the controller has to define all parameters on the controller setup page
      uint16_t usesHost             : 1;
      uint16_t usesPort             : 1;
      uint16_t usesQueue            : 1;
      uint16_t usesCheckReply       : 1;
      uint16_t usesTimeout          : 1;
      uint16_t usesSampleSets       : 1;
      uint16_t usesExtCreds         : 1;
      uint16_t needsNetwork         : 1;
      uint16_t allowsExpire         : 1;
      uint16_t allowLocalSystemTime : 1;
    };
    uint16_t bits{};
  };

//  uint8_t Number{};
};


#endif // DATASTRUCTS_PROTOCOLSTRUCT_H
