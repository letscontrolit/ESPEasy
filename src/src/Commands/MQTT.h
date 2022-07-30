#ifndef COMMAND_MQTT_H
#define COMMAND_MQTT_H

#include "../../ESPEasy_common.h"

#if FEATURE_MQTT

#include <Arduino.h>

const __FlashStringHelper * Command_MQTT_Publish(struct EventStruct *event,
                            const char         *Line);

const __FlashStringHelper * Command_MQTT_Subscribe(struct EventStruct *event,
                              const char* Line);

#endif // if FEATURE_MQTT

#endif // COMMAND_MQTT_H
