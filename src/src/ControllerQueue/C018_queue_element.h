#ifndef CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/ESPEasyLimits.h"

struct EventStruct;

// #ifdef USES_C018

/*********************************************************************************************\
* C018_queue_element for queueing requests for C018: TTN/RN2483
\*********************************************************************************************/


class C018_queue_element {
public:

  C018_queue_element();

  C018_queue_element(struct EventStruct *event, uint8_t sampleSetCount);

  size_t getSize() const;

  int controller_idx = 0;
  String packed;
};

// #endif //USES_C018


#endif // CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
