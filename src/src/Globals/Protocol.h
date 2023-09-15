#ifndef GLOBALS_PROTOCOL_H
#define GLOBALS_PROTOCOL_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/ProtocolStruct.h"
#include "../DataTypes/ProtocolIndex.h"

/*
struct ProtocolVector {

  ProtocolStruct& operator[](protocolIndex_t protocolIndex);
  const ProtocolStruct& operator[](protocolIndex_t protocolIndex) const;
};


// Array of ProtocolStruct where CPlugins will store their info during CPLUGIN_ADD
extern ProtocolVector Protocol;
*/
extern int protocolCount;

#endif // GLOBALS_PROTOCOL_H