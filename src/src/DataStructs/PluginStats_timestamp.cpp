#include "../DataStructs/PluginStats_timestamp.h"

#if FEATURE_PLUGIN_STATS

# include "../Globals/ESPEasy_time.h"

PluginStats_timestamp::~PluginStats_timestamp()
{}

bool PluginStats_timestamp::push(uint32_t unixTime)
{
  return _timestamps.push(unixTime);
}

void PluginStats_timestamp::clear()
{
  _timestamps.clear();
}

void PluginStats_timestamp::processTimeSet()
{
  const size_t nrSamples = _timestamps.size();

  const uint32_t unixTime   = node_time.getUnixTime();
  const uint32_t uptime_sec = (millis() / 1000);

  if (unixTime < uptime_sec) {
    return;
  }
  const uint32_t timeOffset = unixTime - uptime_sec;

  for (PluginStatsTimestamps_t::index_t i = 0; i < nrSamples; ++i) {
    if (_timestamps[i] > uptime_sec) {
      return;
    }
    _timestamps[i] += timeOffset;
  }
}

uint32_t PluginStats_timestamp::getTimestamp(int lastNrSamples) const
{
  if ((_timestamps.size() == 0) || (_timestamps.size() < abs(lastNrSamples))) { return 0u; }

  PluginStatsTimestamps_t::index_t i = 0;

  if (lastNrSamples > 0) {
    i = _timestamps.size() - lastNrSamples;
  } else if (lastNrSamples < 0) {
    i = abs(lastNrSamples) - 1;
  }

  if (i < _timestamps.size()) {
    return _timestamps[i];
  }
  return 0u;
}

uint32_t PluginStats_timestamp::getFullPeriodInSec() const
{
  const size_t nrSamples = _timestamps.size();

  if (nrSamples <= 1) {
    return 0u;
  }

  const uint32_t start = _timestamps[0];
  const uint32_t end   = _timestamps[nrSamples - 1];

  if (end < start) { return start - end; }
  return end - start;
}

uint32_t PluginStats_timestamp::operator[](PluginStatsTimestamps_t::index_t index) const
{
  if (index < _timestamps.size()) { return _timestamps[index]; }
  return 0u;
}

#endif // if FEATURE_PLUGIN_STATS
