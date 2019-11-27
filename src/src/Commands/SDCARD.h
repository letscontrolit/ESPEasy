#ifndef COMMAND_SDCARD_H
#define COMMAND_SDCARD_H

class String;

#ifdef FEATURE_SD

String Command_SD_LS(struct EventStruct *event, const char* Line);
String Command_SD_Remove(struct EventStruct *event, const char* Line);

#endif

#endif // COMMAND_SDCARD_H
