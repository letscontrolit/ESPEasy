#ifndef CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#ifdef USES_C019

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
* C019_queue_element for queueing requests for C019: ESPEasy-NOW
\*********************************************************************************************/


class C019_queue_element {
public:

  C019_queue_element() = default;

  #ifdef USE_SECOND_HEAP
  C019_queue_element(const C019_queue_element& other);
  #else
  C019_queue_element(const C019_queue_element& other) = delete;
  #endif

  C019_queue_element(C019_queue_element&& other) = default;

  C019_queue_element(struct EventStruct *event);

  size_t getSize() const;

  bool isDuplicate(const C019_queue_element& other) const;

#ifdef USES_ESPEASY_NOW
  const MessageRouteInfo_t* getMessageRouteInfo() const { return nullptr; }
#endif

  String packed;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex       = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  pluginID_t plugin_id = INVALID_PLUGIN_ID;
  EventStruct event;
#ifdef USES_ESPEASY_NOW
  MessageRouteInfo_t MessageRouteInfo; 
#endif
};

#endif //USES_C019


#endif // CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
