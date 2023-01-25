#ifndef GLOBALS_C016_CONTROLLERCACHE_H
#define GLOBALS_C016_CONTROLLERCACHE_H

#include "../../ESPEasy_common.h"

#ifdef USES_C016

# include <Arduino.h>
# include "../DataStructs/ESPEasyControllerCache.h"
# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../DataStructs/DeviceStruct.h"
# include "../ControllerQueue/C016_queue_element.h"

extern ControllerCache_struct ControllerCache;

// ********************************************************************************
// Helper functions used in the webserver to access the cache data
// ********************************************************************************
void   C016_flush();

bool   C016_CacheInitialized();

String C016_getCacheFileName(int & fileNr,
                             bool& islast);

bool   C016_deleteOldestCacheBlock();

bool   C016_deleteAllCacheBlocks();

bool   C016_getTaskSample(C016_binary_element& element);

struct EventStruct C016_getTaskSample(
  unsigned long& timestamp,
  uint8_t      & valueCount,
  float        & val1,
  float        & val2,
  float        & val3,
  float        & val4);

#endif // ifdef USES_C016

#endif // ifndef GLOBALS_C016_CONTROLLERCACHE_H
