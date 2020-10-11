#ifndef DATASTRUCT_TASKINDEX_H
#define DATASTRUCT_TASKINDEX_H

#include <Arduino.h>

#include "../CustomBuild/ESPEasyLimits.h"

#define USERVAR_MAX_INDEX    (VARS_PER_TASK * TASKS_MAX)

typedef byte taskIndex_t;
typedef uint16_t userVarIndex_t;
typedef uint16_t taskVarIndex_t;

extern taskIndex_t INVALID_TASK_INDEX;
extern userVarIndex_t INVALID_USERVAR_INDEX;
extern taskVarIndex_t INVALID_TASKVAR_INDEX;


#endif // ifndef DATASTRUCT_TASKINDEX_H
