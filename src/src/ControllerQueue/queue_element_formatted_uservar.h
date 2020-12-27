#ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H
#define CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/DeviceStruct.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"


struct EventStruct;

/*********************************************************************************************\
* For queueing task values already formatted according to the task settings
\*********************************************************************************************/
class queue_element_formatted_uservar {
public:

  queue_element_formatted_uservar();

  queue_element_formatted_uservar(struct EventStruct *event);

  size_t getSize() const;

  String txt[VARS_PER_TASK];
  int idx                          = 0;
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType          = Sensor_VType::SENSOR_TYPE_NONE;
  byte valueCount                  = 0;
};

#endif // CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H
