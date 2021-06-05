#ifndef COMMAND_GPIO_H
#define COMMAND_GPIO_H

#define PLUGIN_GPIO          1
#define PLUGIN_MCP           9
#define PLUGIN_PCF          19

#define GPIO_TYPE_INVALID   0
#define GPIO_TYPE_INTERNAL  1
#define GPIO_TYPE_MCP       2
#define GPIO_TYPE_PCF       3

#include <Arduino.h>


// FIXME TD-er: This fwd declaration should not be in .h file.
// Only needed till GPIO can be set from ESPEasy core.
void createAndSetPortStatus_Mode_State(uint32_t key, byte newMode, int8_t newState);


const __FlashStringHelper * Command_GPIO(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_Toggle(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_PWM(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_Tone(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_RTTTL(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_Pulse(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_LongPulse(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_LongPulse_Ms(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_Monitor(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_UnMonitor(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_Status(struct EventStruct *event, const char* Line);

const __FlashStringHelper * Command_GPIO_McpGPIORange(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_McpGPIOPattern(struct EventStruct *event, const char* Line);

const __FlashStringHelper * Command_GPIO_PcfGPIORange(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_PcfGPIOPattern(struct EventStruct *event, const char* Line);

const __FlashStringHelper * Command_GPIO_Mode(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_ModeRange(struct EventStruct *event, const char* Line);

const __FlashStringHelper * Command_GPIO_MonitorRange(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_GPIO_UnMonitorRange(struct EventStruct *event, const char* Line);

bool getGPIOPinStateValues(String& str);

#endif // COMMAND_GPIO_H
