#ifndef CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#if FEATURE_MQTT

# include "../ControllerQueue/Queue_element_base.h"
# include "../Globals/CPlugins.h"
#include "../DataStructs/MessageRouteInfo.h"

/*********************************************************************************************\
* MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element : public Queue_element_base {
public:

  MQTT_queue_element() = default;

  MQTT_queue_element(const MQTT_queue_element& other) = delete;

  MQTT_queue_element(MQTT_queue_element&& other) = default;

  explicit MQTT_queue_element(int           ctrl_idx,
                              taskIndex_t   TaskIndex,
                              const String& topic,
                              const String& payload,
                              bool          retained,
                              bool          callbackTask);

  explicit MQTT_queue_element(int         ctrl_idx,
                              taskIndex_t TaskIndex,
                              String   && topic,
                              String   && payload,
                              bool        retained,
                              bool        callbackTask);

  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const MessageRouteInfo_t* getMessageRouteInfo() const {
    return &MessageRouteInfo;
  }

  MessageRouteInfo_t* getMessageRouteInfo() {
    return &MessageRouteInfo;
  }

  void removeEmptyTopics();

  String _topic{};
  String _payload{};
  bool _retained = false; 
  MessageRouteInfo_t MessageRouteInfo{};
};

#endif // if FEATURE_MQTT

#endif // CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
