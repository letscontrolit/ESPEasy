#ifndef CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../ControllerQueue/Queue_element_base.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;

#ifdef USES_C015

/*********************************************************************************************\
* C015_queue_element for queueing requests for 015: Blynk
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/

class C015_queue_element : public Queue_element_base {
public:

  C015_queue_element() = default;

# ifdef USE_SECOND_HEAP
  C015_queue_element(const C015_queue_element& other) = default;
# else // ifdef USE_SECOND_HEAP
  C015_queue_element(const C015_queue_element& other) = delete;
# endif // ifdef USE_SECOND_HEAP

  C015_queue_element(C015_queue_element&& other);

  C015_queue_element(const struct EventStruct *event,
                     uint8_t                   value_count);

  C015_queue_element      & operator=(C015_queue_element&& other);

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
  int vPin[VARS_PER_TASK]    = { 0 };
  int idx                    = 0;
  mutable uint8_t valuesSent = 0; // Value must be set by const function checkDone()
  uint8_t valueCount         = 0;
};

#endif // USES_C015


#endif // CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H
