#pragma once

#include "../Helpers/LongTermTimer.h"

class LongTermOnOffTimer
{
public:

  void                    clear();

  void                    resetCount() { _changeToOffCount = 0; _changeToOnCount = 0; }

  // Return true when state changed
  bool                    setOn();

  // Return true when state changed
  bool                    setOff();

  bool                    set(bool onState);

  // Set to specific on/off state + reset last change timer to now
  bool                    forceSet(bool onState);

  // Return true if either 'on' or 'off' was set at least once (since 'clear()' or construction)
  bool                    isSet() const;

  // Returns true if the last set state was to 'on'
  bool                    isOn() const;

  // Returns true if the last set state was to 'off'
  bool                    isOff() const;

  LongTermTimer::Duration getLastOnDuration_ms() const;
  LongTermTimer::Duration getLastOffDuration_ms() const;

  LongTermTimer::Duration getLastOnDuration_usec() const;
  LongTermTimer::Duration getLastOffDuration_usec() const;

  size_t                  getCycleCount() const       { return (_changeToOnCount + _changeToOffCount) / 2; }

  size_t                  getChangeToOnCount() const  { return _changeToOnCount; }

  size_t                  getChangeToOffCount() const { return _changeToOffCount; }

  bool                    changedSinceLastCheck() const { return _changedSinceLastCheck; }

  bool                    changedSinceLastCheck_and_clear()
  {
    if (_changedSinceLastCheck) {
      _changedSinceLastCheck = false;
      return true;
    }
    return false;
  }

private:

  LongTermTimer _onTimer;
  LongTermTimer _offTimer;

  LongTermTimer::Duration _prevDuration{};
  size_t _changeToOnCount{};
  size_t _changeToOffCount{};

  bool _changedSinceLastCheck{};

}; // class LongTermOnOffTimer
