#include "../ControllerQueue/MQTT_queue_element.h"


MQTT_queue_element::MQTT_queue_element() {}

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       const String& topic, const String& payload, bool retained) :
  _topic(topic), _payload(payload), TaskIndex(TaskIndex), controller_idx(ctrl_idx), _retained(retained)
{
  // some parts of the topic may have been replaced by empty strings,
  // or "/status" may have been appended to a topic ending with a "/"
  // Get rid of "//"
  while (_topic.indexOf(F("//")) != -1) {
    _topic.replace(F("//"), F("/"));
  }
}

size_t MQTT_queue_element::getSize() const {
  return sizeof(*this) + _topic.length() + _payload.length();
}

bool MQTT_queue_element::isDuplicate(const MQTT_queue_element& other) const {
  if (other.controller_idx != controller_idx ||
      other._retained != _retained) {
    return false;
  }
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    if (other._topic[i] != _topic[i]) {
      return false;
    }
    if (other._payload[i] != _payload[i]) {
      return false;
    }
  }
  return true;
}
