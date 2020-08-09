#ifndef GLOBALS_SYSTEMTIMERS_H
#define GLOBALS_SYSTEMTIMERS_H

#include "../DataStructs/SystemTimerStruct.h"

#include <map>

// Map mixed timer ID to system timer struct.
// N.B. Must use Mixed timer ID, similar to how it is handled in the scheduler.
extern std::map<unsigned long, systemTimerStruct> systemTimers;

#endif // GLOBALS_SYSTEMTIMERS_H