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

  int         Par1      = 0;
  int         Par2      = 0;
  int         Par3      = 0;
  int         Par4      = 0;
  int         Par5      = 0;
  taskIndex_t TaskIndex = INVALID_TASK_INDEX;


  // ***************
  // Rules Timer use
  // ***************
  systemTimerStruct(int           recurringCount,
                    unsigned long msecFromNow,
                    unsigned int  timerIndex);

  bool          isRecurring() const;

  void          markNextRecurring();

  unsigned long getInterval() const;

  unsigned int  getTimerIndex() const;

  bool          isPaused() const;

  int           getRemainder() const;

  void          setRemainder(int timeLeft);

  int           getLoopCount() const;
};


#endif // DATASTRUCTS_SYSTEMTIMERSTRUCT_H
