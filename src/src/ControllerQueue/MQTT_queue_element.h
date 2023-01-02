#ifndef CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"

#if FEATURE_MQTT

# include "../ControllerQueue/Queue_element_base.h"
# include "../DataStructs/UnitMessageCount.h"
# include "../Globals/CPlugins.h"

/*********************************************************************************************\
* MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element : public Queue_element_base {
public:

  MQTT_queue_element() = default;

# ifdef USE_SECOND_HEAP
  MQTT_queue_element(const MQTT_queue_element& other) = default;
# else // ifdef USE_SECOND_HEAP
  MQTT_queue_element(const MQTT_queue_element& other) = delete;
# endif // ifdef USE_SECOND_HEAP

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

  size_t                    getSize() const;

  bool                      isDuplicate(const Queue_element_base& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const {
    return &UnitMessageCount;
  }

  UnitMessageCount_t* getUnitMessageCount() {
    return &UnitMessageCount;
  }

  void removeEmptyTopics();

  String _topic;
  String _payload;
  bool _retained = false;
  UnitMessageCount_t UnitMessageCount;
};

#endif // if FEATURE_MQTT

#endif // CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
