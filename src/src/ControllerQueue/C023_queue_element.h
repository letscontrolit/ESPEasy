#ifndef CONTROLLERQUEUE_C023_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C023_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#ifdef USES_C023

# include "../ControllerQueue/Queue_element_base.h"
# include "../CustomBuild/ESPEasyLimits.h"
# include "../Globals/CPlugins.h"


struct EventStruct;
struct UnitMessageCount_t;

/*********************************************************************************************\
* C023_queue_element for queueing requests for C023: AT-command LoRaWAN
\*********************************************************************************************/


class C023_queue_element : public Queue_element_base {
public:

  C023_queue_element() = default;

  C023_queue_element(const C023_queue_element& other) = delete;

  C023_queue_element(C023_queue_element&& other) = default;

  C023_queue_element(struct EventStruct *event,
                     uint8_t             sampleSetCount);

  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return nullptr;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return nullptr;
  }

  String packed;
};

#endif // USES_C023


#endif // CONTROLLERQUEUE_C023_QUEUE_ELEMENT_H
