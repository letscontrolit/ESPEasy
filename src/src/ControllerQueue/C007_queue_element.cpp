#include "../ControllerQueue/C007_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"
#include "../../_Plugin_Helper.h"

#ifdef USES_C007

C007_queue_element::C007_queue_element() {}

C007_queue_element::C007_queue_element(EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType)
{
  valueCount = getValueCountForTask(TaskIndex);

  for (byte i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

size_t C007_queue_element::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

#endif // ifdef USES_C007
