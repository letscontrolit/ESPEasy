#include "../ControllerQueue/C018_queue_element.h"

#ifdef USES_C018

# include "../DataStructs/ESPEasy_EventStruct.h"

# include "../ESPEasyCore/ESPEasy_Log.h"

# include "../Helpers/_CPlugin_LoRa_TTN_helper.h"

C018_queue_element::C018_queue_element(struct EventStruct *event, uint8_t sampleSetCount)
{
  _controller_idx = event->ControllerIndex;
  _taskIndex      = event->TaskIndex;
  # if FEATURE_PACKED_RAW_DATA
    #  ifdef USE_SECOND_HEAP

  //    HeapSelectIram ephemeral;
    #  endif // ifdef USE_SECOND_HEAP

  packed = getPackedFromPlugin(event, sampleSetCount);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C018 queue element: ");
    log += packed;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // if FEATURE_PACKED_RAW_DATA
}

size_t C018_queue_element::getSize() const {
  return sizeof(*this) + packed.length();
}

bool C018_queue_element::isDuplicate(const Queue_element_base& other) const {
  const C018_queue_element& oth = static_cast<const C018_queue_element&>(other);

  if ((oth._controller_idx != _controller_idx) ||
      (oth._taskIndex != _taskIndex) ||
      (oth.packed != packed)) {
    return false;
  }
  return true;
}

#endif // ifdef USES_C018
