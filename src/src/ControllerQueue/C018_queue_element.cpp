#include "../ControllerQueue/C018_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Helpers/_CPlugin_LoRa_TTN_helper.h"

#ifdef USES_C018

C018_queue_element::C018_queue_element() {}

C018_queue_element::C018_queue_element(struct EventStruct *event, uint8_t sampleSetCount) :
  controller_idx(event->ControllerIndex)
{
    #ifdef USES_PACKED_RAW_DATA
  packed = getPackedFromPlugin(event, sampleSetCount);
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C018 queue element: ");
    log += packed;
    addLog(LOG_LEVEL_INFO, log);
  }
    #endif // USES_PACKED_RAW_DATA
}

size_t C018_queue_element::getSize() const {
  return sizeof(*this) + packed.length();
}

#endif
