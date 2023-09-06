#ifndef CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#ifdef USES_C018

# include "../ControllerQueue/Queue_element_base.h"
# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/UnitMessageCount.h"
# include "../Globals/CPlugins.h"


struct EventStruct;

/*********************************************************************************************\
* C018_queue_element for queueing requests for C018: TTN/RN2483
\*********************************************************************************************/


class C018_queue_element : public Queue_element_base {
public:

  C018_queue_element() = default;

# ifdef USE_SECOND_HEAP
  C018_queue_element(const C018_queue_element& other) = default;
# else // ifdef USE_SECOND_HEAP
  C018_queue_element(const C018_queue_element& other) = delete;
# endif // ifdef USE_SECOND_HEAP

  C018_queue_element(C018_queue_element&& other) = default;

  C018_queue_element(struct EventStruct *event,
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

#endif // USES_C018


#endif // CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
