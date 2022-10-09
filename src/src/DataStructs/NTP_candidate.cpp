#include "../DataStructs/NTP_candidate.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../DataTypes/ESPEasyTimeSource.h"
#include "../Helpers/ESPEasy_time_calc.h"


void NTP_candidate_struct::set(const NodeStruct& node)
{
  if (node.unix_time < get_build_unixtime()) { return; }
  const timeSource_t timeSource = static_cast<timeSource_t>(node.timeSource);

  if (!isExternalTimeSource(timeSource)) { return; }
  const unsigned long time_wander = computeExpectedWander(timeSource, node.lastUpdated);

  if (timePassedSince(_received_moment) > 3600000) { clear(); }

  if ((_time_wander < 0) || (time_wander < static_cast<unsigned long>(_time_wander))) {
    _time_wander     = time_wander;
    _unix_time       = node.unix_time;
    _received_moment = millis();
  }
}

void NTP_candidate_struct::clear()
{
  _unix_time       = 0;
  _time_wander     = -1;
  _received_moment = 0;
}

bool NTP_candidate_struct::getUnixTime(uint32_t& unix_time) const
{
  if ((_unix_time == 0) || (_time_wander < 0)) {
    return false;
  }

  const int seconds_passed = timePassedSince(_received_moment) / 1000;

  if (seconds_passed < 0) { return false; }
  unix_time  = _unix_time;
  unix_time += seconds_passed;
  return false;
}
