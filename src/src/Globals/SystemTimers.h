#ifndef GLOBALS_SYSTEMTIMERS_H
#define GLOBALS_SYSTEMTIMERS_H

#include "../DataStructs/SystemTimerStruct.h"

#include <map>

extern std::map<unsigned long, systemTimerStruct> systemTimers;

#endif // GLOBALS_SYSTEMTIMERS_H