#include "CAN.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/_Plugin_Helper_CAN.h"

const __FlashStringHelper* Command_CAN_SendToCAN(struct EventStruct *event, const char *line)
{
    if (canHelper_sendCmd(event)) {
        return F("OK");
    }

    return F("ERROR");
}

const __FlashStringHelper* Command_CAN_SendCAN(struct EventStruct *event, const char * line)
{
    if (canHelper_sendData(event->Par2, event->Par1, static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE), event->Par3)) {
        return F("OK");
    }

    return F("ERROR");
}