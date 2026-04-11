#ifndef GLOBALS_RUNTIMEDATA_H
#define GLOBALS_RUNTIMEDATA_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../DataStructs/UserVarStruct.h"

#include <map>

/*********************************************************************************************\
* Custom Variables for usage in rules and http.
* This is volatile data, meaning it is lost after a reboot.
* Syntax: %vX%, %v_Y%, where X is numeric and Y numeric or alphanumeric, not case-sensitive
* usage:
* let,1,10
* let,two,20
* if %v1%=10 do ...
* if %v_two%=20 do ...
\*********************************************************************************************/
extern std::map<String, ESPEASY_RULES_FLOAT_TYPE> customFloatVar;
#if FEATURE_STRING_VARIABLES
extern std::map<String, String> customStringVar;
#endif // if FEATURE_STRING_VARIABLES

ESPEASY_RULES_FLOAT_TYPE getCustomFloatVar(String index, ESPEASY_RULES_FLOAT_TYPE defaultValue = 0.0);
void setCustomFloatVar(String index, const ESPEASY_RULES_FLOAT_TYPE& value);

bool getNextCustomFloatVar(String& index, ESPEASY_RULES_FLOAT_TYPE& value);

#if FEATURE_STRING_VARIABLES
String getCustomStringVar(String index, String defaultValue = EMPTY_STRING);
void setCustomStringVar(String index, const String& value);
void clearCustomStringVar(String index);
bool hasCustomStringVar(String index);

bool getNextCustomStringVar(String& index, String& value);
#endif // if FEATURE_STRING_VARIABLES

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