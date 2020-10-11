#ifndef COMMAND_MQTT_H
#define COMMAND_MQTT_H

#include "../../ESPEasy_common.h"

#ifdef USES_MQTT

class String;

String Command_MQTT_Publish(struct EventStruct *event,
                            const char         *Line);

String Command_MQTT_Subscribe(struct EventStruct *event,
                              const char* Line);

#endif // ifdef USES_MQTT

#endif // COMMAND_MQTT_H
