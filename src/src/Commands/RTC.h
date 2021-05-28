#ifndef COMMAND_RTC_H
#define COMMAND_RTC_H

#include <Arduino.h>

const __FlashStringHelper * Command_RTC_Clear(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line);

#endif // COMMAND_RTC_H
