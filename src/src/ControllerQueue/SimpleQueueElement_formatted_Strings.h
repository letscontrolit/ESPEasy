#ifndef CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
#define CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H


#include "../../ESPEasy_common.h"
#include "../ControllerQueue/Queue_element_base.h"
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
class SimpleQueueElement_formatted_Strings : public Queue_element_base {
public:

  SimpleQueueElement_formatted_Strings() = default;

  // Constructor formatting the task values using the default formatter
  SimpleQueueElement_formatted_Strings(struct EventStruct *event);

  // Constructor not formatting the values
  SimpleQueueElement_formatted_Strings(const struct EventStruct *event,
                                       uint8_t                   value_count);


#ifdef USE_SECOND_HEAP
  SimpleQueueElement_formatted_Strings(const SimpleQueueElement_formatted_Strings& rval) = default;
#else // ifdef USE_SECOND_HEAP
  SimpleQueueElement_formatted_Strings(const SimpleQueueElement_formatted_Strings& rval) = delete;
#endif // ifdef USE_SECOND_HEAP

  SimpleQueueElement_formatted_Strings(SimpleQueueElement_formatted_Strings&& rval);

  SimpleQueueElement_formatted_Strings& operator=(SimpleQueueElement_formatted_Strings&& other);


  // For controllers that only send a single value per request and thus need to keep track of the number of values already sent.
  bool                      checkDone(bool succesfull) const;

  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return nullptr;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return nullptr;
  }

  String txt[VARS_PER_TASK]  = {};
  int idx                    = 0;
  Sensor_VType sensorType    = Sensor_VType::SENSOR_TYPE_NONE;
  mutable uint8_t valuesSent = 0; // Value must be set by const function checkDone()
  uint8_t valueCount         = 0;
};


#endif // CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
