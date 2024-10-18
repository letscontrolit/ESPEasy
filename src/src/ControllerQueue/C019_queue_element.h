#ifndef CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#ifdef USES_C019

# include "../ControllerQueue/Queue_element_base.h"
# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../DataStructs/MessageRouteInfo.h"
# include "../Globals/CPlugins.h"

/*********************************************************************************************\
* C019_queue_element for queueing requests for C019: ESPEasy-NOW
\*********************************************************************************************/


class C019_queue_element : public Queue_element_base  {
public:

  C019_queue_element() = default;

  C019_queue_element(const C019_queue_element& other) = delete;

  C019_queue_element(C019_queue_element&& other) = default;

  C019_queue_element(struct EventStruct *event);

  size_t getSize() const;

  bool isDuplicate(const Queue_element_base& other) const;


  const MessageRouteInfo_t* getMessageRouteInfo() const { return nullptr; }

  MessageRouteInfo_t* getMessageRouteInfo() { return nullptr; }


  String packed;
  pluginID_t plugin_id = INVALID_PLUGIN_ID;
  EventStruct event;
};

#endif //USES_C019


#endif // CONTROLLERQUEUE_C019_QUEUE_ELEMENT_H
