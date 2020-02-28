#ifndef COMMAND_TIME_H
#define COMMAND_TIME_H

class String;

#include <time.h>

String Command_NTPHost(struct EventStruct *event, const char* Line);
String Command_useNTP(struct EventStruct *event, const char* Line);
String Command_TimeZone(struct EventStruct *event, const char* Line);
String Command_DST(struct EventStruct *event, const char* Line);
String Command_SetDateTime(struct EventStruct *event, const char *Line);

#endif // COMMAND_TIME_H
