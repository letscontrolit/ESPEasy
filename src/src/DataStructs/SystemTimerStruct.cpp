#include "SystemTimerStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"


// Rules Timer use
// ***************
systemTimerStruct::systemTimerStruct(int recurringCount, unsigned long msecFromNow, unsigned int timerIndex) :
  Par1(recurringCount), Par2(msecFromNow), Par3(timerIndex), Par4(0), Par5(1)
{
  if (recurringCount > 0) {
    // Will run with Par1 == 0, so must subtract one when setting the value.
    Par1--;
  }

  if (msecFromNow == 0) {
    // Create a new timer which should be "scheduled" now to clear up any data
    Par1 = 0; // Do not reschedule
    Par5 = 0; // Do not execute
    addLog(LOG_LEVEL_INFO, F("TIMER: disable timer"));
  }
}

bool systemTimerStruct::isRecurring() const {
  return Par1 != 0;
}

void systemTimerStruct::markNextRecurring() {
  if (Par1 > 0) {
    // This is a timer with a limited number of runs, so decrease its value.
    Par1--;
  }

  if (Par5 > 0) {
    // This one should be executed, so increase the count.
    Par5++;
  }
}

unsigned long systemTimerStruct::getInterval() const {
  return Par2;
}

unsigned int systemTimerStruct::getTimerIndex() const {
  return Par3;
}

bool systemTimerStruct::isPaused() const {
  return Par4 != 0;
}

int systemTimerStruct::getRemainder() const {
  return Par4;
}

void systemTimerStruct::setRemainder(int timeLeft) {
  Par4 = timeLeft;
}

int systemTimerStruct::getLoopCount() const {
  return Par5;
}
