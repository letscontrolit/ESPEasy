#ifndef CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../Globals/Plugins.h"

struct EventStruct;


// #ifdef USES_C016

/*********************************************************************************************\
* C016_queue_element for queueing requests for C016: Cached HTTP.
\*********************************************************************************************/
class C016_queue_element {
public:

  C016_queue_element();

  C016_queue_element(const struct EventStruct *event,
                     byte                      value_count,
                     unsigned long             unixTime);

  size_t getSize() const;

  float values[VARS_PER_TASK] = { 0 };
  unsigned long timestamp     = 0; // Unix timestamp
  taskIndex_t TaskIndex       = INVALID_TASK_INDEX;
  byte controller_idx         = 0;
  Sensor_VType sensorType     = Sensor_VType::SENSOR_TYPE_NONE;
  byte valueCount             = 0;
};

// #endif //USES_C016


#endif // CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
