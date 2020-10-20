#ifndef COMMAND_GPIO_H
#define COMMAND_GPIO_H

#define PLUGIN_GPIO          1
#define PLUGIN_MCP           9
#define PLUGIN_PCF          19

#define GPIO_TYPE_INVALID   0
#define GPIO_TYPE_INTERNAL  1
#define GPIO_TYPE_MCP       2
#define GPIO_TYPE_PCF       3


class String;

String Command_GPIO(struct EventStruct *event, const char* Line);
String Command_GPIO_Toggle(struct EventStruct *event, const char* Line);
String Command_GPIO_Pulse(struct EventStruct *event, const char* Line);
String Command_GPIO_LongPulse(struct EventStruct *event, const char* Line);
String Command_GPIO_LongPulse_Ms(struct EventStruct *event, const char* Line);
String Command_GPIO_Monitor(struct EventStruct *event, const char* Line);
String Command_GPIO_UnMonitor(struct EventStruct *event, const char* Line);
String Command_GPIO_Status(struct EventStruct *event, const char* Line);

#endif // COMMAND_GPIO_H
