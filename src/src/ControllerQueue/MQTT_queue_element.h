#ifndef CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#if FEATURE_MQTT

#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"

/*********************************************************************************************\
* MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element {
public:

  MQTT_queue_element() = default;

#ifdef USE_SECOND_HEAP
  MQTT_queue_element(const MQTT_queue_element& other) = default;
#else
  MQTT_queue_element(const MQTT_queue_element& other) = delete;
#endif
  
  MQTT_queue_element(MQTT_queue_element&& other) = default;

  explicit MQTT_queue_element(int           ctrl_idx,
                              taskIndex_t   TaskIndex,
                              const String& topic,
                              const String& payload,
                              bool          retained);

  explicit MQTT_queue_element(int         ctrl_idx,
                              taskIndex_t TaskIndex,
                              String   && topic,
                              String   && payload,
                              bool        retained);

  size_t getSize() const;

  bool isDuplicate(const MQTT_queue_element& other) const;

#ifdef USES_ESPEASY_NOW
  const MessageRouteInfo_t* getMessageRouteInfo() const { return &MessageRouteInfo; }
  MessageRouteInfo_t* getMessageRouteInfo() { return &MessageRouteInfo; }
#endif

  void removeEmptyTopics();

  String _topic;
  String _payload;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  bool _retained                   = false;
#ifdef USES_ESPEASY_NOW
  MessageRouteInfo_t MessageRouteInfo;
#endif
};

#endif

#endif // CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
