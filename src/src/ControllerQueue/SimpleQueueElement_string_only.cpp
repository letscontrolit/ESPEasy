#include "../ControllerQueue/SimpleQueueElement_string_only.h"

#include "../Helpers/StringConverter.h"


simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  String&& req)
{
  _controller_idx = ctrl_idx;
  _taskIndex      = TaskIndex;
  move_special(txt, std::move(req));
}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const Queue_element_base& other) const {
  const simple_queue_element_string_only& oth = static_cast<const simple_queue_element_string_only&>(other);

  if ((oth._controller_idx != _controller_idx) ||
      (oth._taskIndex != _taskIndex) ||
      (oth.txt != txt)) {
    return false;
  }
  return true;
}
