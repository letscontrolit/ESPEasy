#ifndef GLOBALS_RUNTIMEDATA_H
#define GLOBALS_RUNTIMEDATA_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../DataStructs/UserVarStruct.h"

#include <map>

/*********************************************************************************************\
* Custom Variables for usage in rules and http.
* This is volatile data, meaning it is lost after a reboot.
* Syntax: %vX%
* usage:
* let,1,10
* if %v1%=10 do ...
\*********************************************************************************************/
extern std::map<uint32_t, ESPEASY_RULES_FLOAT_TYPE> customFloatVar;

ESPEASY_RULES_FLOAT_TYPE getCustomFloatVar(uint32_t index, ESPEASY_RULES_FLOAT_TYPE defaultValue = 0.0);
void setCustomFloatVar(uint32_t index, const ESPEASY_RULES_FLOAT_TYPE& value);

bool getNextCustomFloatVar(uint32_t& index, ESPEASY_RULES_FLOAT_TYPE& value);


/*********************************************************************************************\
* Task Value data.
* Also stored into RTC memory, and restored at boot.
* usage:
* let,1,10
* if %v1%=10 do ...
\*********************************************************************************************/
//extern float UserVar[VARS_PER_TASK * TASKS_MAX];

extern UserVarStruct UserVar;



#endif // GLOBALS_RUNTIMEDATA_H