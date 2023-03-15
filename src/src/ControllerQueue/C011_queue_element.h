#ifndef CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../ControllerQueue/Queue_element_base.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;


#ifdef USES_C011

/*********************************************************************************************\
* C011_queue_element for queueing requests for C011: Generic HTTP Advanced.
\*********************************************************************************************/
class C011_queue_element : public Queue_element_base {
public:

  C011_queue_element() = default;

  C011_queue_element(C011_queue_element&& other) = default;

# ifdef USE_SECOND_HEAP
  C011_queue_element(const C011_queue_element& other) = default;
# else // ifdef USE_SECOND_HEAP
  C011_queue_element(const C011_queue_element& other) = delete;
# endif // ifdef USE_SECOND_HEAP

  C011_queue_element(const struct EventStruct *event);

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return nullptr;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return nullptr;
  }

  size_t getSize() const;

  String uri;
  String HttpMethod;
  String header;
  String postStr;
  int idx                 = 0;
  Sensor_VType sensorType = Sensor_VType::SENSOR_TYPE_NONE;
};

#endif // USES_C011


#endif // CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H
