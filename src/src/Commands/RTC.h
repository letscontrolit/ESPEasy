#ifndef COMMAND_RTC_H
#define COMMAND_RTC_H

class String;

String Command_RTC_Clear(struct EventStruct *event, const char* Line);
String Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line);

#endif // COMMAND_RTC_H
