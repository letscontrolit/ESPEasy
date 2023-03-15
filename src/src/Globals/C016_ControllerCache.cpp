#include "../Globals/C016_ControllerCache.h"

#ifdef USES_C016


# include "../ControllerQueue/C016_queue_element.h"

ControllerCache_struct ControllerCache;

void C016_flush() {
  ControllerCache.flush();
}

bool C016_CacheInitialized() {
  return ControllerCache.isInitialized();
}

String C016_getCacheFileName(int& fileNr, bool& islast) {
  return ControllerCache.getNextCacheFileName(fileNr, islast);
}

bool C016_deleteOldestCacheBlock() {
  return ControllerCache.deleteOldestCacheBlock();
}

bool C016_deleteAllCacheBlocks() {
  return ControllerCache.deleteAllCacheBlocks();
}

bool C016_getTaskSample(C016_binary_element& element) {
  return ControllerCache.peek((uint8_t *)&element, sizeof(element));
}

struct EventStruct C016_getTaskSample(
  unsigned long& timestamp,
  uint8_t      & valueCount,
  float        & val1,
  float        & val2,
  float        & val3,
  float        & val4)
{
  C016_binary_element element;

  if (!ControllerCache.peek((uint8_t *)&element, sizeof(element))) {
    return EventStruct();
  }

  timestamp  = element.unixTime;
  valueCount = element.valueCount;
  val1       = element.values[0];
  val2       = element.values[1];
  val3       = element.values[2];
  val4       = element.values[3];

  EventStruct event(element.TaskIndex);

  // FIXME TD-er: Is this needed?
  //  event.ControllerIndex = element._controller_idx;
  event.sensorType = element.sensorType;

  return event;
}

#endif // ifdef USES_C016
