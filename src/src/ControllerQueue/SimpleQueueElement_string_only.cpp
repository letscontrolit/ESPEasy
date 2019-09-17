#include "../ControllerQueue/SimpleQueueElement_string_only.h"

simple_queue_element_string_only::simple_queue_element_string_only() : controller_idx(0) {}

simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, const String& req) :
  controller_idx(ctrl_idx), txt(req) {}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(this) + txt.length();
}
