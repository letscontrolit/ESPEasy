#include "../ControllerQueue/MQTT_queue_element.h"


MQTT_queue_element::MQTT_queue_element() {}

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       const String& topic, const String& payload, bool retained) :
  _topic(topic), _payload(payload), controller_idx(ctrl_idx), _retained(retained)
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
