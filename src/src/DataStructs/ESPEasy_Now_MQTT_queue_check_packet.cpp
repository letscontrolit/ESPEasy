#include "ESPEasy_Now_MQTT_queue_check_packet.h"



void ESPEasy_Now_MQTT_queue_check_packet::markSendTime() {
  _millis_out = millis();
  state = QueueState::Unset;
}

void ESPEasy_Now_MQTT_queue_check_packet::setState(bool isFull)
{
  state = isFull ? QueueState::Full : QueueState::Empty;
}

bool ESPEasy_Now_MQTT_queue_check_packet::isSet() const
{
  return state != QueueState::Unset;
}