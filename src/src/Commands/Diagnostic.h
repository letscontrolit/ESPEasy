#ifndef COMMAND_DIAGNOSTIC_H
#define COMMAND_DIAGNOSTIC_H

#include <stdint.h>
#include <map>

#include "../../ESPEasy_common.h"

class String;
struct portStatusStruct;

#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
String Command_Lowmem(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Malloc(struct EventStruct *event, const char* Line);
String Command_SysLoad(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_SerialFloat(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_MemInfo(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_MemInfo_detail(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Background(struct EventStruct *event, const char* Line);
#endif
const __FlashStringHelper * Command_Debug(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_logentry(struct EventStruct *event, const char* Line);
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
const __FlashStringHelper * Command_JSONPortStatus(struct EventStruct *event, const char* Line);
//void createLogPortStatus(std::map< uint32_t, portStatusStruct >::iterator it);
//void debugPortStatus(std::map< uint32_t, portStatusStruct >::iterator it);
void logPortStatus(const String& from);
const __FlashStringHelper * Command_logPortStatus(struct EventStruct *event, const char* Line);
#endif


#ifndef BUILD_MINIMAL_OTA
// Show settings file layout in charts on sysinfo page.
// Will be enabled after running command "meminfodetail"
extern bool showSettingsFileLayout;
#endif


#endif // COMMAND_DIAGNOSTIC_H
