#ifndef COMMAND_SDCARD_H
#define COMMAND_SDCARD_H

#include "../../ESPEasy_common.h"
#if FEATURE_SD

# include <FS.h>

void                       printDirectory(fs::File dir,
                                          int      numTabs);
const __FlashStringHelper* Command_SD_LS(struct EventStruct *event,
                                         const char         *Line);
String                     Command_SD_Remove(struct EventStruct *event,
                                             const char         *Line);

#endif // if FEATURE_SD

#endif // COMMAND_SDCARD_H
