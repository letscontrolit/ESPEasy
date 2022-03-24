#ifndef COMMAND_CONTROLLR_H
#define COMMAND_CONTROLLR_H

#include <Arduino.h>

const __FlashStringHelper * Command_Controller_Disable(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Controller_Enable(struct EventStruct *event, const char* Line);



#endif // COMMAND_CONTROLLR_H