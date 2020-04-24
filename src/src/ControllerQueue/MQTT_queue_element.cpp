#include "../ControllerQueue/MQTT_queue_element.h"



MQTT_queue_element::MQTT_queue_element() {}

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       const String& topic, const String& payload, bool retained) :
  _topic(topic), _payload(payload), controller_idx(ctrl_idx), _retained(retained)
{}

size_t MQTT_queue_element::getSize() const {
  return sizeof(*this) + _topic.length() + _payload.length();
}
