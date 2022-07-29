#include "../ControllerQueue/C019_queue_element.h"


#ifdef USES_C019

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"


#if FEATURE_PACKED_RAW_DATA
String getPackedFromPlugin(struct EventStruct *event,
                           uint8_t             sampleSetCount);
#endif

#ifdef USE_SECOND_HEAP
C019_queue_element::C019_queue_element(const C019_queue_element& other) :
  packed(other.packed),
  _timestamp(other._timestamp),
  TaskIndex(other.TaskIndex),
  controller_idx(other.controller_idx),
  plugin_id(other.plugin_id)
#ifdef USES_ESPEASY_NOW
  , MessageRouteInfo(other.MessageRouteInfo)
#endif
{
   event.deep_copy(other.event);
}
#endif

C019_queue_element::C019_queue_element(struct EventStruct *event_p) :
  controller_idx(event_p->ControllerIndex)
{
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

bool C019_queue_element::isDuplicate(const C019_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.plugin_id != plugin_id) ||
      (other.packed != packed)) {
    return false;
  }

  // FIXME TD-er: Must check event too?
  return false;
}


#endif