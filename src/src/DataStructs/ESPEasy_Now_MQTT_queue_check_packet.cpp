#include "../DataStructs/ESPEasy_Now_MQTT_queue_check_packet.h"


#ifdef USES_ESPEASY_NOW

ESPEasy_Now_MQTT_queue_check_packet::ESPEasy_Now_MQTT_queue_check_packet() {}

void ESPEasy_Now_MQTT_queue_check_packet::markSendTime() {
  _millis_out = millis();
  state       = ESPEasy_Now_MQTT_QueueCheckState::Enum::Unset;
}

void ESPEasy_Now_MQTT_queue_check_packet::setState(bool isFull)
{
  state = isFull ? ESPEasy_Now_MQTT_QueueCheckState::Enum::Full : ESPEasy_Now_MQTT_QueueCheckState::Enum::Empty;
}

bool ESPEasy_Now_MQTT_queue_check_packet::isFull() const
{
  return state == ESPEasy_Now_MQTT_QueueCheckState::Enum::Full;
}

bool ESPEasy_Now_MQTT_queue_check_packet::isSet() const
{
  return state != ESPEasy_Now_MQTT_QueueCheckState::Enum::Unset;
}

#endif // ifdef USES_ESPEASY_NOW
