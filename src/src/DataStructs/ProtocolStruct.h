#ifndef DATASTRUCTS_PROTOCOLSTRUCT_H
#define DATASTRUCTS_PROTOCOLSTRUCT_H

#include "../../ESPEasy_common.h"
#include <vector>

/*********************************************************************************************\
* ProtocolStruct
\*********************************************************************************************/
struct ProtocolStruct
{
  ProtocolStruct() :
    defaultPort(0), Number(0), usesMQTT(false), usesAccount(false), usesPassword(false),
    usesTemplate(false), usesID(false), Custom(false), usesHost(true), usesPort(true),
    usesQueue(true), usesSampleSets(false) {}

  uint16_t defaultPort;
  byte     Number;
  bool     usesMQTT       : 1;
  bool     usesAccount    : 1;
  bool     usesPassword   : 1;
  bool     usesTemplate   : 1; // When set, the protocol will pre-load some templates like default MQTT topics
  bool     usesID         : 1; // Whether a controller supports sending an IDX value sent along with plugin data
  bool     Custom         : 1; // When set, the controller has to define all parameters on the controller setup page
  bool     usesHost       : 1;
  bool     usesPort       : 1;
  bool     usesQueue      : 1;
  bool     usesSampleSets : 1;
};

typedef std::vector<ProtocolStruct> ProtocolVector;


#endif // DATASTRUCTS_PROTOCOLSTRUCT_H
