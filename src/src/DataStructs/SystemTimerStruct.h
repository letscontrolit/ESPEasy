#ifndef DATASTRUCTS_SYSTEMTIMERSTRUCT_H
#define DATASTRUCTS_SYSTEMTIMERSTRUCT_H

#include "../../ESPEasy_common.h"
#include <map>

/*********************************************************************************************\
 * systemTimerStruct
\*********************************************************************************************/
struct systemTimerStruct
{
  systemTimerStruct() :
    timer(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0), TaskIndex(-1), plugin(0) {}

  unsigned long timer;
  int Par1;
  int Par2;
  int Par3;
  int Par4;
  int Par5;
  int16_t TaskIndex;
  byte plugin;
};
std::map<unsigned long, systemTimerStruct> systemTimers;



#endif // DATASTRUCTS_SYSTEMTIMERSTRUCT_H
