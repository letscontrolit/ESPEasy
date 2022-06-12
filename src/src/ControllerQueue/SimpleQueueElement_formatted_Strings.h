#ifndef CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
#define CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H


#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;

/*********************************************************************************************\
* Base element class for keeping task value strings in a controller queue
* Can also be used for controllers only sending a single value at a time.
\*********************************************************************************************/
class SimpleQueueElement_formatted_Strings {
public:

  SimpleQueueElement_formatted_Strings() = default;

  // Constructor formatting the task values using the default formatter
  SimpleQueueElement_formatted_Strings(struct EventStruct *event);

  // Constructor not formatting the values
  SimpleQueueElement_formatted_Strings(const struct EventStruct *event,
                                  uint8_t                   value_count);


#ifdef USE_SECOND_HEAP
  SimpleQueueElement_formatted_Strings(const SimpleQueueElement_formatted_Strings& rval) = default;
#else
  SimpleQueueElement_formatted_Strings(const SimpleQueueElement_formatted_Strings& rval) = delete;
#endif
  
  SimpleQueueElement_formatted_Strings(SimpleQueueElement_formatted_Strings&& rval);

  SimpleQueueElement_formatted_Strings& operator=(SimpleQueueElement_formatted_Strings&& other);


  // For controllers that only send a single value per request and thus need to keep track of the number of values already sent.
  bool                             checkDone(bool succesfull) const;

  size_t                           getSize() const;

  bool                             isDuplicate(const SimpleQueueElement_formatted_Strings& other) const;

  const UnitMessageCount_t       * getUnitMessageCount() const {
    return nullptr;
  }

  String txt[VARS_PER_TASK];
  int idx                          = 0;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType          = Sensor_VType::SENSOR_TYPE_NONE;
  mutable uint8_t valuesSent          = 0; // Value must be set by const function checkDone()
  uint8_t valueCount                  = 0;
};


#endif // CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
