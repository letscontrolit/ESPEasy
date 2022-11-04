#include "../DataStructs/NTP_candidate.h"

#if FEATURE_ESPEASY_P2P

# include "../CustomBuild/CompiletimeDefines.h"
# include "../DataTypes/ESPEasyTimeSource.h"
# include "../Globals/Settings.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"


bool NTP_candidate_struct::set(const NodeStruct& node)
{
  if (node.unit == Settings.Unit) { return false; }

  if (node.unix_time_sec < get_build_unixtime()) { return false; }
  const timeSource_t timeSource = static_cast<timeSource_t>(node.timeSource);

  if (timeSource == timeSource_t::No_time_source) { return false; }

  // Only allow time from p2p nodes who only got it via p2p themselves as "last resource"
  const unsigned long p2p_source_penalty =
    isExternalTimeSource(timeSource)  ? 0 : 10000;
  const unsigned long time_wander_other =
    p2p_source_penalty + computeExpectedWander(timeSource, node.lastUpdated);

  if (timePassedSince(_received_moment) > EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_MSEC) { clear(); }

  if ((_time_wander < 0) || (time_wander_other < static_cast<unsigned long>(_time_wander))) {
    _time_wander     = time_wander_other;
    _unix_time_sec   = node.unix_time_sec;
    _unix_time_frac  = node.unix_time_frac;
    _received_moment = millis();
    _unit            = node.unit;

    if (_first_received_moment == 0) {
      _first_received_moment = _received_moment;
    }
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
  _unix_time_sec         = 0;
  _unix_time_frac        = 0;
  _time_wander           = -1;
  _received_moment       = 0;
  _first_received_moment = 0;
}

bool NTP_candidate_struct::getUnixTime(double& unix_time_d, uint8_t& unit) const
{
  if ((_unix_time_sec == 0) || (_time_wander < 0) || (_received_moment == 0)) {
    return false;
  }

  if (timePassedSince(_first_received_moment) < 30000) {
    // Make sure to allow for enough time to collect the "best" option.
    return false;
  }

  unit = _unit;

  unix_time_d = static_cast<double>(_unix_time_sec);

  // Add fractional part.
  unix_time_d += (static_cast<double>(_unix_time_frac) / 4294967295.0);

  // Add time since it was received
  unix_time_d += static_cast<double>(timePassedSince(_received_moment)) / 1000.0;

  return true;
}

#endif // if FEATURE_ESPEASY_P2P
