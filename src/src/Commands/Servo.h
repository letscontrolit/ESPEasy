#ifndef COMMANDS_SERVO_H
#define COMMANDS_SERVO_H

#include "../../ESPEasy_common.h"

#include <Arduino.h>
#include <map>

#ifdef USE_SERVO
# include <Servo.h>

// IRAM: doing servo stuff uses 740 bytes IRAM. (doesnt matter how many instances)

typedef std::map<uint8_t, Servo> ServoPinMap_t;
extern ServoPinMap_t ServoPinMap;
#endif // USE_SERVO

const __FlashStringHelper * Command_Servo(struct EventStruct *event,
                     const char         *Line);


#endif // ifndef COMMANDS_SERVO_H
