#ifndef COMMAND_TIME_H
#define COMMAND_TIME_H

class String;

String Command_NTPHost(struct EventStruct *event, const char* Line);
String Command_useNTP(struct EventStruct *event, const char* Line);
String Command_TimeZone(struct EventStruct *event, const char* Line);
String Command_DST(struct EventStruct *event, const char* Line);
String Command_DateTime(struct EventStruct *event, const char *Line);

#endif // COMMAND_TIME_H
