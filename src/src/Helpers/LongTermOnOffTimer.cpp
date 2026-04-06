#include "../Helpers/LongTermOnOffTimer.h"

void LongTermOnOffTimer::clear()
{
  _changedSinceLastCheck = _onTimer.isSet() || _offTimer.isSet();
  _onTimer.clear();
  _offTimer.clear();
  _prevDuration = 0;
  resetCount();
}

bool LongTermOnOffTimer::setOn()
{
  if (isOn()) { return false; }
  _changedSinceLastCheck = true;
  ++_changeToOnCount;

  if (isOff()) {
    _prevDuration = _offTimer.usecPassedSince();
  }
  _onTimer.setNow();
  return true;
}

bool LongTermOnOffTimer::setOff()
{
  if (isOff()) { return false; }
  _changedSinceLastCheck = true;
  ++_changeToOffCount;

  if (isOn()) {
    _prevDuration = _onTimer.usecPassedSince();
  }
  _offTimer.setNow();
  return true;
}

bool LongTermOnOffTimer::set(bool onState)
{
  if (onState) { return setOn(); }
  return setOff();
}

bool LongTermOnOffTimer::forceSet(bool onState)
{
  set(!onState);
  return set(onState);
}

bool LongTermOnOffTimer::isSet() const
{
  return _onTimer.isSet() || _offTimer.isSet();
}

bool LongTermOnOffTimer::isOn() const
{
  if (!_onTimer.isSet()) { return false; }

  if (!_offTimer.isSet()) { return true; }

  // return true when 'onTimer' is later than 'offTimer'
  return _offTimer < _onTimer;
}

bool LongTermOnOffTimer::isOff() const
{
  if (!_offTimer.isSet()) { return false; }

  if (!_onTimer.isSet()) { return true; }

  // return true when 'offTimer' is later than 'onTimer'
  return _offTimer > _onTimer;
}

LongTermTimer::Duration LongTermOnOffTimer::getLastOnDuration_ms() const
{
  return getLastOnDuration_usec() / 1000ll;
}

LongTermTimer::Duration LongTermOnOffTimer::getLastOffDuration_ms() const
{
  return getLastOffDuration_usec() / 1000ll;
}

LongTermTimer::Duration LongTermOnOffTimer::getLastOnDuration_usec() const
{
  if (isOn()) { return _onTimer.usecPassedSince(); }
  return _prevDuration;
}

LongTermTimer::Duration LongTermOnOffTimer::getLastOffDuration_usec() const
{
  if (isOff()) { return _offTimer.usecPassedSince(); }
  return _prevDuration;
}
