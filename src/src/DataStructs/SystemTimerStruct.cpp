#include "../DataStructs/SystemTimerStruct.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"


// Rules Timer use
// ***************
systemTimerStruct::systemTimerStruct(int recurringCount, unsigned long msecFromNow, unsigned int timerIndex, int alternateInterval) :
  _recurringCount(recurringCount), _interval(msecFromNow), _timerIndex(timerIndex), _remainder(0), _loopCount(1), _alternateInterval(
    alternateInterval)
{
  if ((recurringCount > 0) && !hasAlternateInterval()) {
    // Will run with _recurringCount == 0, so must subtract one when setting the value.
    _recurringCount--;
  }

  if (msecFromNow == 0) {
    // Create a new timer which should be "scheduled" now to clear up any data
    _recurringCount = 0; // Do not reschedule
    _loopCount      = 0; // Do not execute
    addLog(LOG_LEVEL_INFO, F("TIMER: disable timer"));
  }

  if (hasAlternateInterval()) {
    // Need to double the recurring count, or at least set it to 1 to make sure the alternate interval is also ran at least once.
    if (_recurringCount > 0) {
      _recurringCount *= 2;
    } else if (_recurringCount == 0) {
      _recurringCount = 1;
    }
  }
}

struct EventStruct systemTimerStruct::toEvent() const {
  struct EventStruct TempEvent(TaskIndex);

  TempEvent.Par1 = _recurringCount;
  TempEvent.Par2 = _interval;
  TempEvent.Par3 = _timerIndex;
  TempEvent.Par4 = _remainder;
  TempEvent.Par5 = _loopCount;
  return TempEvent;
}

void systemTimerStruct::fromEvent(taskIndex_t taskIndex,
                                  int         Par1,
                                  int         Par2,
                                  int         Par3,
                                  int         Par4,
                                  int         Par5)
{
  TaskIndex       = taskIndex;
  _recurringCount = Par1;
  _interval       = Par2;
  _timerIndex     = Par3;
  _remainder      = Par4;
  _loopCount      = Par5;
}

bool systemTimerStruct::isRecurring() const {
  return _recurringCount != 0;
}

void systemTimerStruct::markNextRecurring() {
  toggleAlternateState(); // Will only toggle if it has an alternate state

  if (_recurringCount > 0) {
    // This is a timer with a limited number of runs, so decrease its value.
    _recurringCount--;
  }

  if (_loopCount > 0) {
    // This one should be executed, so increase the count.
    _loopCount++;
  }
}

unsigned long systemTimerStruct::getInterval() const {
  return _alternateState ? _alternateInterval : _interval;
}

unsigned int systemTimerStruct::getTimerIndex() const {
  return _timerIndex;
}

bool systemTimerStruct::isPaused() const {
  return _remainder != 0;
}

int systemTimerStruct::getRemainder() const {
  return _remainder;
}

void systemTimerStruct::setRemainder(int timeLeft) {
  _remainder = timeLeft;
}

int systemTimerStruct::getLoopCount() const {
  if (hasAlternateInterval()) { return _loopCount / 2; }
  return _loopCount;
}

void systemTimerStruct::toggleAlternateState() {
  if (hasAlternateInterval()) {
    _alternateState = !_alternateState;
  } else {
    _alternateState = false;
  }
}
