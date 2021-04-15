#include "../ControllerQueue/SimpleQueueElement_string_only.h"

simple_queue_element_string_only::simple_queue_element_string_only() {}

simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  const String& req) :
   txt(req), TaskIndex(TaskIndex), controller_idx(ctrl_idx) {}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const simple_queue_element_string_only& other) const {
  if (other.controller_idx != controller_idx || 
      other.TaskIndex != TaskIndex) {
    return false;
  }
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}