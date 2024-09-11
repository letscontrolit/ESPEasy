#ifndef HELPERS__PLUGIN_HELPER_CAN_H
#define HELPERS__PLUGIN_HELPER_CAN_H

#include "../DataStructs/ESPEasy_EventStruct.h"

void canHelper_sendTaskData(EventStruct *event);

bool canHelper_sendData(const uint8_t taskIndex, const uint8_t valIndex, const uint8_t sensorType, const uint32_t val32);

bool canHelper_sendCmd(EventStruct *event);

void canHelper_recv();

#endif
