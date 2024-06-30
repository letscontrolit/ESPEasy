#include "../DataStructs/PluginStats_timestamp.h"

#if FEATURE_PLUGIN_STATS

# include "../Globals/ESPEasy_time.h"
# include "../Helpers/ESPEasy_time_calc.h"

PluginStats_timestamp::~PluginStats_timestamp()
{}

bool PluginStats_timestamp::push(const int64_t& timestamp_sysmicros)
{
  return _timestamps.push(systemMicros_to_internalTimestamp(timestamp_sysmicros));
}

bool PluginStats_timestamp::updateLast(const int64_t& timestamp_sysmicros)
{
  const size_t nrElements = _timestamps.size();

  if (nrElements == 0) { return false; }
  return _timestamps.set(nrElements - 1, systemMicros_to_internalTimestamp(timestamp_sysmicros));
}

void PluginStats_timestamp::clear()
{
  _timestamps.clear();
}

void PluginStats_timestamp::processTimeSet(const double& time_offset)
{
  /*
     const size_t nrSamples = _timestamps.size();

     constexpr int64_t unixTime_20200101 = 1577836800ll * PLUGINSTATS_INTERNAL_TO_MICROS_RATIO;

     // GMT	Wed Jan 01 2020 00:00:00 GMT+0000

     for (PluginStatsTimestamps_t::index_t i = 0; i < nrSamples; ++i) {
     if (_timestamps[i] < unixTime_20200101) {
      _timestamps.set(i, _timestamps[i] + time_offset);
     }
     }
   */
}

int64_t PluginStats_timestamp::getTimestamp(int lastNrSamples) const
{
  if ((_timestamps.size() == 0) || (_timestamps.size() < abs(lastNrSamples))) { return 0u; }

  PluginStatsTimestamps_t::index_t i = 0;

  if (lastNrSamples > 0) {
    i = _timestamps.size() - lastNrSamples;
  } else if (lastNrSamples < 0) {
    i = abs(lastNrSamples) - 1;
  }

  if (i < _timestamps.size()) {
    return internalTimestamp_to_systemMicros(_timestamps[i]);
  }
  return 0u;
}

uint32_t PluginStats_timestamp::getFullPeriodInSec() const
{
  const size_t nrSamples = _timestamps.size();

  if (nrSamples <= 1) {
    return 0u;
  }

  const int64_t start = internalTimestamp_to_systemMicros(_timestamps[0]);
  const int64_t end   = internalTimestamp_to_systemMicros(_timestamps[nrSamples - 1]);

  if (end < start) { return (start - end) / 1000000ll; }
  return (end - start) / 1000000ll;
}

int64_t PluginStats_timestamp::operator[](PluginStatsTimestamps_t::index_t index) const
{
  if (index < _timestamps.size()) {
    return internalTimestamp_to_systemMicros(_timestamps[index]);
  }
  return 0u;
}

uint32_t PluginStats_timestamp::systemMicros_to_internalTimestamp(const int64_t& timestamp_sysmicros)
{
  return static_cast<uint32_t>(timestamp_sysmicros / PLUGINSTATS_INTERNAL_TO_MICROS_RATIO);
}

int64_t PluginStats_timestamp::internalTimestamp_to_systemMicros(const uint32_t& internalTimestamp)
{
  const uint64_t cur_micros        = getMicros64();
  constexpr uint64_t overflow_step = 4294967296ull * PLUGINSTATS_INTERNAL_TO_MICROS_RATIO;

  uint64_t sysMicros = static_cast<uint64_t>(internalTimestamp) * PLUGINSTATS_INTERNAL_TO_MICROS_RATIO;

  // Try to get in the range of the current system micros
  // This only does play a role when uptime is over 994 days.
  while ((sysMicros + overflow_step) < cur_micros) {
    sysMicros += overflow_step;
  }
  return sysMicros;
}

#endif // if FEATURE_PLUGIN_STATS
