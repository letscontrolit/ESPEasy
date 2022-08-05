#include "../ControllerQueue/MQTT_queue_element.h"

#if FEATURE_MQTT

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       const String& topic, const String& payload, bool retained) :
  TaskIndex(TaskIndex), controller_idx(ctrl_idx), _retained(retained)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif
  // Copy in the scope of the constructor, so we might store it in the 2nd heap
  _topic = topic;
  _payload = payload;

  removeEmptyTopics();
}

MQTT_queue_element::MQTT_queue_element(int         ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       String   && topic,
                                       String   && payload,
                                       bool        retained)
  : TaskIndex(TaskIndex), controller_idx(ctrl_idx), _retained(retained)
{
  // Copy in the scope of the constructor, so we might store it in the 2nd heap
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  if (topic.length() && !mmu_is_iram(&(topic[0]))) {
    _topic = topic;
  } else {
    _topic = std::move(topic);  
  }
  if (payload.length() && !mmu_is_iram(&(payload[0]))) {
    _payload = payload;
  } else {
    _payload = std::move(payload);
  }
  #else
  _topic = std::move(topic);
  _payload = std::move(payload);
  #endif

  removeEmptyTopics();
}

size_t MQTT_queue_element::getSize() const {
  return sizeof(*this) + _topic.length() + _payload.length();
}

bool MQTT_queue_element::isDuplicate(const MQTT_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other._retained != _retained) ||
      other._topic != _topic ||
      other._payload != _payload) {
    return false;
  }
  return true;
}

void MQTT_queue_element::removeEmptyTopics() {
  // some parts of the topic may have been replaced by empty strings,
  // or "/status" may have been appended to a topic ending with a "/"
  // Get rid of "//"
  while (_topic.indexOf(F("//")) != -1) {
    _topic.replace(F("//"), F("/"));
  }
}
#endif // if FEATURE_MQTT
