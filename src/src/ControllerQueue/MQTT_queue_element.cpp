#include "../ControllerQueue/MQTT_queue_element.h"

#if FEATURE_MQTT

#include "../Helpers/StringConverter.h"

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       const String& topic, const String& payload,
                                       bool retained, bool callbackTask) :
  _retained(retained)
{
  _controller_idx                      = ctrl_idx;
  _taskIndex                           = TaskIndex;
  _call_PLUGIN_PROCESS_CONTROLLER_DATA = callbackTask;

  // Copy in the scope of the constructor, so we might store it in the 2nd heap
  move_special(_topic, String(topic));
  move_special(_payload, String(payload));

  removeEmptyTopics();
}

MQTT_queue_element::MQTT_queue_element(int         ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       String   && topic,
                                       String   && payload,
                                       bool        retained,
                                       bool        callbackTask)
  : _retained(retained)
{
  _controller_idx                      = ctrl_idx;
  _taskIndex                           = TaskIndex;
  _call_PLUGIN_PROCESS_CONTROLLER_DATA = callbackTask;

  // Copy in the scope of the constructor, so we might store it in the 2nd heap
  move_special(_topic, std::move(topic));
  move_special(_payload, std::move(payload));
  removeEmptyTopics();
}

size_t MQTT_queue_element::getSize() const {
  return sizeof(*this) + _topic.length() + _payload.length();
}

bool MQTT_queue_element::isDuplicate(const Queue_element_base& other) const {
  if (_call_PLUGIN_PROCESS_CONTROLLER_DATA || other._call_PLUGIN_PROCESS_CONTROLLER_DATA) {
    return false;
  }
  const MQTT_queue_element& oth = static_cast<const MQTT_queue_element&>(other);

  // TD-er: We do not compare the taskindex.
  // If it were to make a difference, the topic would be different.
  if ((oth._controller_idx != _controller_idx) ||
      (oth._retained != _retained) ||
      (oth._topic != _topic) ||
      (oth._payload != _payload)) {
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
