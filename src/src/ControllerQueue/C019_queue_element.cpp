#include "../ControllerQueue/C019_queue_element.h"


#ifdef USES_C019

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"


#if FEATURE_PACKED_RAW_DATA
String getPackedFromPlugin(struct EventStruct *event,
                           uint8_t             sampleSetCount);
#endif

C019_queue_element::C019_queue_element(struct EventStruct *event_p)
{
  _controller_idx = event_p->ControllerIndex;
  _taskIndex      = event_p->TaskIndex;

  event.deep_copy(event_p);
  #if FEATURE_PACKED_RAW_DATA
  packed = getPackedFromPlugin(event_p, 0);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C019 queue element: ");
    log += packed;
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif

  // Extra check to make sure sensorType is correct.
  event.sensorType = event.getSensorType();
}

size_t C019_queue_element::getSize() const {
  return sizeof(*this) + packed.length();
}

bool C019_queue_element::isDuplicate(const Queue_element_base& other) const {
  const C019_queue_element& oth = static_cast<const C019_queue_element&>(other);

  if ((oth._controller_idx != _controller_idx) ||
      (oth._taskIndex != _taskIndex) ||
      (oth.plugin_id != plugin_id) ||
      (oth.packed != packed)) {
    return false;
  }

  // FIXME TD-er: Must check event too?
  return false;
}


#endif