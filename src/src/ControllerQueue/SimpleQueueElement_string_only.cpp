#include "../ControllerQueue/SimpleQueueElement_string_only.h"


simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  String&& req) :
  TaskIndex(TaskIndex)
{
  controller_idx = ctrl_idx;
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  if (req.length() > 0 && !mmu_is_iram(&(req[0]))) {
    // The string was not allocated on the 2nd heap, so copy instead of move
    txt = req;
  } else {
    txt = std::move(req);
  }
  #else
  txt = std::move(req);
  #endif
}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const Queue_element_base& other) const {
  const simple_queue_element_string_only& oth = static_cast<const simple_queue_element_string_only&>(other);
  if ((oth.controller_idx != controller_idx) ||
      (oth.TaskIndex != TaskIndex) ||
      (oth.txt != txt)) {
    return false;
  }
  return true;
}
