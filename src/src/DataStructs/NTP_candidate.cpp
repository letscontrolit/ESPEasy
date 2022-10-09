#include "../DataStructs/NTP_candidate.h"

#if FEATURE_ESPEASY_P2P

# include "../CustomBuild/CompiletimeDefines.h"
# include "../DataTypes/ESPEasyTimeSource.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"


bool NTP_candidate_struct::set(const NodeStruct& node)
{
  if (node.unix_time_sec < get_build_unixtime()) { return false; }
  const timeSource_t timeSource = static_cast<timeSource_t>(node.timeSource);

  if (!isExternalTimeSource(timeSource)) { return false; }
  const unsigned long time_wander = computeExpectedWander(timeSource, node.lastUpdated);

  if (timePassedSince(_received_moment) > 3600000) { clear(); }

  if ((_time_wander < 0) || (time_wander < static_cast<unsigned long>(_time_wander))) {
    _time_wander     = time_wander;
    _unix_time_sec   = node.unix_time_sec;
    _unix_time_frac  = node.unix_time_frac;
    _received_moment = millis();
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("NTP  : Time candidate: ");
      log += node.getSummary();

      log += concat(F(" time: "), _unix_time_sec);
      log += ' ';
      log += toString(timeSource);
      log += concat(F(" est. wander: "), _time_wander);
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
    return true;
  }
  return false;
}

void NTP_candidate_struct::clear()
{
  _unix_time_sec   = 0;
  _unix_time_frac  = 0;
  _time_wander     = -1;
  _received_moment = 0;
}

bool NTP_candidate_struct::getUnixTime(double& unix_time_d) const
{
  if ((_unix_time_sec == 0) || (_time_wander < 0) || (_received_moment == 0)) {
    return false;
  }

  unix_time_d = static_cast<double>(_unix_time_sec);

  // Add fractional part.
  unix_time_d += (static_cast<double>(_unix_time_frac) / 4294967295.0);

  // Add time since it was received
  unix_time_d += static_cast<double>(timePassedSince(_received_moment)) / 1000.0;

  return true;
}

#endif // if FEATURE_ESPEASY_P2P
