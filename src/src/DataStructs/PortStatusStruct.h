#ifndef DATASTRUCTS_PORTSTATUSSTRUCT_H
#define DATASTRUCTS_PORTSTATUSSTRUCT_H

#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"
#include <map>

struct portStatusStruct {
  portStatusStruct();

  uint16_t getDutyCycle() const;

  int16_t getValue() const;

  int16_t dutyCycle = 0;

  int8_t state          : 2;       // -1,0,1
  int8_t output         : 2;       // -1,0,1
  int8_t command        : 2;       // 0,1
  int8_t init           : 1;       // 0,1
  int8_t not_used       : 1;       // 0,1

  uint8_t mode          : 3; // 6 current values (max. 8)
  uint8_t task          : 2; // 0-3 (max. 4)
  uint8_t monitor       : 1; // 0,1
  uint8_t forceMonitor  : 1; // 0,1
  uint8_t forceEvent    : 1; // 0,1
 
  int8_t previousTask : 8;

  deviceIndex_t x; // used to synchronize the Plugin_prt vector index (x) with the PLUGIN_ID
};

typedef std::map<uint32_t, portStatusStruct> MapPortStatus;

//giig1967g: TODO: remove std::map []operator and use at(), insert(), find()
//https://devblogs.microsoft.com/oldnewthing/20190227-00/?p=101072
#endif // DATASTRUCTS_PORTSTATUSSTRUCT_H
