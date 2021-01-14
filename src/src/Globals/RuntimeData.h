#ifndef GLOBALS_RUNTIMEDATA_H
#define GLOBALS_RUNTIMEDATA_H

#include "../CustomBuild/ESPEasyLimits.h"

#include <map>

/*********************************************************************************************\
* Custom Variables for usage in rules and http.
* This is volatile data, meaning it is lost after a reboot.
* Syntax: %vX%
* usage:
* let,1,10
* if %v1%=10 do ...
\*********************************************************************************************/
extern std::map<uint32_t, double> customFloatVar;

double getCustomFloatVar(uint32_t index);
void setCustomFloatVar(uint32_t index, const double& value);

bool getNextCustomFloatVar(uint32_t& index, double& value);


/*********************************************************************************************\
* Task Value data.
* Also stored into RTC memory, and restored at boot.
* usage:
* let,1,10
* if %v1%=10 do ...
\*********************************************************************************************/
extern float UserVar[VARS_PER_TASK * TASKS_MAX];



#endif // GLOBALS_RUNTIMEDATA_H