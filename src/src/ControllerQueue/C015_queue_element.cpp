#include "../ControllerQueue/C015_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#ifdef USES_C015

C015_queue_element::C015_queue_element(C015_queue_element&& other)
  : idx(other.idx)
  , valuesSent(other.valuesSent)
  , valueCount(other.valueCount)
{
  _timestamp      = other._timestamp;
  _controller_idx = other._controller_idx;
  _taskIndex      = other._taskIndex;
  # ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  # endif // ifdef USE_SECOND_HEAP

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i]  = std::move(other.txt[i]);
    vPin[i] = other.vPin[i];
  }
}

C015_queue_element::C015_queue_element(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  valuesSent(0),
  valueCount(value_count) {
  _controller_idx = event->ControllerIndex;
  _taskIndex      = event->TaskIndex;
}

C015_queue_element& C015_queue_element::operator=(C015_queue_element&& other) {
  idx             = other.idx;
  _timestamp      = other._timestamp;
  _taskIndex      = other._taskIndex;
  _controller_idx = other._controller_idx;
  valuesSent      = other.valuesSent;
  valueCount      = other.valueCount;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i]  = std::move(other.txt[i]);
    vPin[i] = other.vPin[i];
  }
  return *this;
}

bool C015_queue_element::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t C015_queue_element::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool C015_queue_element::isDuplicate(const Queue_element_base& other) const {
  const C015_queue_element& oth = static_cast<const C015_queue_element&>(other);

  if ((oth._controller_idx != _controller_idx) ||
      (oth._taskIndex != _taskIndex) ||
      (oth.valueCount != valueCount) ||
      (oth.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (oth.txt[i] != txt[i]) {
      return false;
    }

    if (oth.vPin[i] != vPin[i]) {
      return false;
    }
  }
  return true;
}

#endif // ifdef USES_C015
