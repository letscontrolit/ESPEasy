#ifndef COMMAND_WD_H
#define COMMAND_WD_H

class String;
struct EventStruct;

String Command_WD_Config(EventStruct *event, const char* Line);
String Command_WD_Read(EventStruct *event, const char* Line);

#endif // COMMAND_WD_H
