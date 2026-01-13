#ifndef HELPERS_ESPEASY_CHECKS_H
#define HELPERS_ESPEASY_CHECKS_H

#include "../../ESPEasy_common.h"

#ifndef LIMIT_BUILD_SIZE
#include "../Globals/Plugins.h"

void run_compiletime_checks();

String ReportOffsetErrorInStruct(const String& structname, size_t offset);

bool SettingsCheck(String& error);

String checkTaskSettings(taskIndex_t taskIndex);
#endif

#endif // HELPERS_ESPEASY_CHECKS_H