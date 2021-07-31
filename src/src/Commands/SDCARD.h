#ifndef COMMAND_SDCARD_H
#define COMMAND_SDCARD_H

#include "../../ESPEasy_common.h"
#ifdef FEATURE_SD

#include <FS.h>

void printDirectory(File dir, int numTabs);
const __FlashStringHelper * Command_SD_LS(struct EventStruct *event, const char* Line);
String Command_SD_Remove(struct EventStruct *event, const char* Line);

#endif

#endif // COMMAND_SDCARD_H
