#ifndef GLOBALS_PROTOCOL_H
#define GLOBALS_PROTOCOL_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/ProtocolStruct.h"

#include "../CustomBuild/ESPEasyLimits.h"

extern ProtocolStruct Protocol[CPLUGIN_MAX];
extern int protocolCount;

#endif // GLOBALS_PROTOCOL_H