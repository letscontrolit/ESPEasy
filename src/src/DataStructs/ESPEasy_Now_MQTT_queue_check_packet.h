#ifndef DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H
#define DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H

#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

#include <Arduino.h>

#include "../Globals/ESPEasy_now_state.h"
#include "../DataTypes/ESPEasy_Now_MQTT_queue_check_state.h"

class ESPEasy_Now_MQTT_queue_check_packet {
public:

  ESPEasy_Now_MQTT_queue_check_packet();

  void setState(bool isFull);

  bool isFull() const;

  bool isSet() const;

  void markSendTime();

  unsigned long _millis_out = millis();
  ESPEasy_Now_MQTT_QueueCheckState::Enum state = ESPEasy_Now_MQTT_QueueCheckState::Enum::Unset;
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H
