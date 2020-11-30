#include "C016_ControllerCache.h"

#ifdef USES_C016


#include "../ControllerQueue/C016_queue_element.h"

ControllerCache_struct ControllerCache;

bool C016_startCSVdump() {
  ControllerCache.resetpeek();
  return ControllerCache.isInitialized();
}

String C016_getCacheFileName(bool& islast) {
  return ControllerCache.getPeekCacheFileName(islast);
}

bool C016_deleteOldestCacheBlock() {
  return ControllerCache.deleteOldestCacheBlock();
}

bool C016_getCSVline(
  unsigned long& timestamp,
  byte& controller_idx,
  byte& TaskIndex,
  Sensor_VType& sensorType,
  byte& valueCount,
  float& val1,
  float& val2,
  float& val3,
  float& val4)
{
  C016_queue_element element;
  bool result = ControllerCache.peek((uint8_t*)&element, sizeof(element));
  timestamp = element.timestamp;
  controller_idx = element.controller_idx;
  TaskIndex = element.TaskIndex;
  sensorType = element.sensorType;
  valueCount = element.valueCount;
  val1 = element.values[0];
  val2 = element.values[1];
  val3 = element.values[2];
  val4 = element.values[3];
  return result;
}

#endif