#ifndef CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/ESPEasyLimits.h"
#include "../Globals/CPlugins.h"


// #ifdef USES_C019

/*********************************************************************************************\
* C019_queue_element for queueing requests for C019: ESPEasy-Now
\*********************************************************************************************/


class C019_queue_element {
public:

  C019_queue_element();

  C019_queue_element(struct EventStruct *event);

  size_t getSize() const;

  String packed;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  pluginID_t plugin_id = INVALID_PLUGIN_ID;
  EventStruct event;
};

// #endif //USES_C019


#endif // CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
