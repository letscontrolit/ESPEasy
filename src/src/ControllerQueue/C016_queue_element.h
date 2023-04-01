#ifndef CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#ifdef USES_C016


#include "../ControllerQueue/Queue_element_base.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/ControllerIndex.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/Plugins.h"

struct EventStruct;


// The binary format to store the samples using the Cache Controller
// Do NOT change order of members!
struct C016_binary_element {
  float             values[VARS_PER_TASK]{};
  unsigned long     unixTime{};
  taskIndex_t       TaskIndex{INVALID_TASK_INDEX};
  pluginID_t        pluginID{INVALID_PLUGIN_ID};
  Sensor_VType      sensorType{Sensor_VType::SENSOR_TYPE_NONE};
  uint8_t           valueCount{};
};


/*********************************************************************************************\
* C016_queue_element for queueing requests for C016: Cached HTTP.
\*********************************************************************************************/

// TD-er: This one has a fixed uint8_t order and is stored.
// This also means the order of members should not be changed!
class C016_queue_element : public Queue_element_base {
public:

  C016_queue_element();

  C016_queue_element(const C016_queue_element& other) = delete;

  C016_queue_element(C016_queue_element&& other);

  C016_queue_element(const struct EventStruct *event,
                     uint8_t                   value_count);

  C016_queue_element      & operator=(C016_queue_element&& other);


  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return nullptr;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return nullptr;
  }

  C016_binary_element getBinary() const;

  float values[VARS_PER_TASK]{};
  unsigned long unixTime = 0;
  Sensor_VType sensorType{Sensor_VType::SENSOR_TYPE_NONE};
  uint8_t valueCount{};
};

#endif // USES_C016


#endif // CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
