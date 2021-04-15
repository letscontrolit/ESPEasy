#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

class String;

String Command_System_NoSleep(struct EventStruct *event, const char* Line);
String Command_System_deepSleep(struct EventStruct *event, const char* Line);
String Command_System_Reboot(struct EventStruct *event, const char* Line);

#endif // COMMAND_SYSTEM_H
