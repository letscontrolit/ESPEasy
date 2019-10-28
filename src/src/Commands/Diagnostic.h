#ifndef COMMAND_DIAGNOSTIC_H
#define COMMAND_DIAGNOSTIC_H

#include <stdint.h>
#include <map>

class String;
struct portStatusStruct;


String Command_Lowmem(struct EventStruct *event, const char* Line);
String Command_Malloc(struct EventStruct *event, const char* Line);
String Command_SysLoad(struct EventStruct *event, const char* Line);
String Command_SerialFloat(struct EventStruct *event, const char* Line);
String Command_MemInfo(struct EventStruct *event, const char* Line);
String Command_MemInfo_detail(struct EventStruct *event, const char* Line);
String Command_Background(struct EventStruct *event, const char* Line);
String Command_Debug(struct EventStruct *event, const char* Line);
String Command_logentry(struct EventStruct *event, const char* Line);
String Command_JSONPortStatus(struct EventStruct *event, const char* Line);
void createLogPortStatus(std::map< uint32_t, portStatusStruct >::iterator it);
void debugPortStatus(std::map< uint32_t, portStatusStruct >::iterator it);
void logPortStatus(String from);
String Command_logPortStatus(struct EventStruct *event, const char* Line);


#ifndef BUILD_MINIMAL_OTA
// Show settings file layout in charts on sysinfo page.
// Will be enabled after running command "meminfodetail"
extern bool showSettingsFileLayout;
#endif


#endif // COMMAND_DIAGNOSTIC_H
