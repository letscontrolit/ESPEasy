#ifndef DATASTRUCTS_SYSTEMTIMERSTRUCT_H
#define DATASTRUCTS_SYSTEMTIMERSTRUCT_H

#include "../../ESPEasy_common.h"
#include <map>
#include "../Globals/Plugins.h"

/*********************************************************************************************\
 * systemTimerStruct
\*********************************************************************************************/
struct systemTimerStruct
{
  systemTimerStruct() {}

  unsigned long timer = 0;
  int Par1 = 0;
  int Par2 = 0;
  int Par3 = 0;
  int Par4 = 0;
  int Par5 = 0;
  taskIndex_t TaskIndex = INVALID_TASK_INDEX;
  byte plugin = 0;
};
std::map<unsigned long, systemTimerStruct> systemTimers;



#endif // DATASTRUCTS_SYSTEMTIMERSTRUCT_H
