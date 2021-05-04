#include "../ControllerQueue/SimpleQueueElement_string_only.h"

simple_queue_element_string_only::simple_queue_element_string_only() {}

simple_queue_element_string_only::simple_queue_element_string_only(simple_queue_element_string_only&& other)
  : txt(std::move(other.txt)), _timestamp(other._timestamp), TaskIndex(other.TaskIndex), controller_idx(other.controller_idx)
{}

simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  String&& req) :
  txt(std::move(req)), TaskIndex(TaskIndex), controller_idx(ctrl_idx) {}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const simple_queue_element_string_only& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.txt != txt)) {
    return false;
  }
  return true;
}
