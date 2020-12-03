#ifndef GLOBALS_C016_CONTROLLERCACHE_H
#define GLOBALS_C016_CONTROLLERCACHE_H

#include "../../ESPEasy_common.h"

#ifdef USES_C016

#include <Arduino.h>
#include "../DataStructs/ESPEasyControllerCache.h"
#include "../DataStructs/DeviceStruct.h"

extern ControllerCache_struct ControllerCache;

//********************************************************************************
// Helper functions used in the webserver to access the cache data
//********************************************************************************
bool C016_startCSVdump();

String C016_getCacheFileName(bool& islast);

bool C016_deleteOldestCacheBlock();

bool C016_getCSVline(
  unsigned long& timestamp,
  byte& controller_idx,
  byte& TaskIndex,
  Sensor_VType& sensorType,
  byte& valueCount,
  float& val1,
  float& val2,
  float& val3,
  float& val4);

#endif

#endif