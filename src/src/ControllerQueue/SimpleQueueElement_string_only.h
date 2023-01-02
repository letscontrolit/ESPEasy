#ifndef CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
#define CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H

#include "../../ESPEasy_common.h"
#include "../ControllerQueue/Queue_element_base.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
* Simple queue element, only storing controller index and some String
\*********************************************************************************************/
class simple_queue_element_string_only : public Queue_element_base {
public:

  simple_queue_element_string_only() = default;

  #ifdef USE_SECOND_HEAP
  simple_queue_element_string_only(const simple_queue_element_string_only& other) = default;
  #else // ifdef USE_SECOND_HEAP
  simple_queue_element_string_only(const simple_queue_element_string_only& other) = delete;
  #endif // ifdef USE_SECOND_HEAP

  simple_queue_element_string_only(simple_queue_element_string_only&& other) = default;

  explicit simple_queue_element_string_only(int         ctrl_idx,
                                            taskIndex_t TaskIndex,
                                            String   && req);

  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return nullptr;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return nullptr;
  }

  String txt;
};


#endif // CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
