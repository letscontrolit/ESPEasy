#ifndef CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

struct EventStruct;


// #ifdef USES_C007

/*********************************************************************************************\
* C007_queue_element for queueing requests for C007 Emoncms
\*********************************************************************************************/
class C007_queue_element {
public:

  C007_queue_element();

  C007_queue_element(const struct EventStruct *event);

  size_t getSize() const;

  int controller_idx = 0;
  byte TaskIndex     = 0;
  int idx            = 0;
  byte sensorType    = 0;
};

// #endif //USES_C007

#endif // CONTROLLERQUEUE_C007_QUEUE_ELEMENT_H
