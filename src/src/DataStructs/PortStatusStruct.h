#ifndef DATASTRUCTS_PORTSTATUSSTRUCT_H
#define DATASTRUCTS_PORTSTATUSSTRUCT_H

#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"
#include <map>

struct portStatusStruct {
  portStatusStruct() : state(-1), output(-1), command(0), init(0), not_used(0), mode(0), task(0), monitor(0), forceMonitor(0), forceEvent(0), previousTask(
      -1), x(INVALID_DEVICE_INDEX) {}

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
