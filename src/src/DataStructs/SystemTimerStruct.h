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

private:

  int         _recurringCount    = 0;
  int         _interval          = 0;
  int         _timerIndex        = 0;
  int         _remainder         = 0;
  int         _loopCount         = 0;
  int         _alternateInterval = 0;
  taskIndex_t TaskIndex          = INVALID_TASK_INDEX;
  bool        _alternateState    = false;

public:

  // ***************
  // Rules Timer use
  // ***************
  systemTimerStruct(int           recurringCount,
                    unsigned long msecFromNow,
                    unsigned int  timerIndex,
                    int alternateInterval = 0);

  struct EventStruct toEvent() const;

  void               fromEvent(taskIndex_t TaskIndex,
                               int         Par1,
                               int         Par2,
                               int         Par3,
                               int         Par4,
                               int         Par5);

  bool          isRecurring() const;

  void          markNextRecurring();

  unsigned long getInterval() const;

  unsigned int  getTimerIndex() const;

  bool          isPaused() const;

  int           getRemainder() const;

  void          setRemainder(int timeLeft);

  int           getLoopCount() const;

  void          toggleAlternateState();

  bool          isAlternateState() const {
    return _alternateState;
  }

  bool hasAlternateInterval() const {
    return _alternateInterval > 0;
  }
};


#endif // DATASTRUCTS_SYSTEMTIMERSTRUCT_H
