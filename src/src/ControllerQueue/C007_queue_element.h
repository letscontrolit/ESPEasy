#ifndef CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/DeviceStruct.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"


struct EventStruct;


#ifdef USES_C007

/*********************************************************************************************\
* C007_queue_element for queueing requests for C007 Emoncms
\*********************************************************************************************/
class C007_queue_element {
public:

  C007_queue_element();

  C007_queue_element(struct EventStruct *event);

  size_t getSize() const;

  String txt[VARS_PER_TASK];
  int idx                          = 0;
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType          = Sensor_VType::SENSOR_TYPE_NONE;
  byte valueCount                  = 0;
};

#endif //USES_C007

#endif // CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H
