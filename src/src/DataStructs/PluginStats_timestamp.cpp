#include "../DataStructs/PluginStats_timestamp.h"

#if FEATURE_PLUGIN_STATS

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

uint32_t PluginStats_timestamp::operator[](PluginStatsTimestamps_t::index_t index) const
{
  if (index < _timestamps.size()) { return _timestamps[index]; }
  return 0u;
}

#endif // if FEATURE_PLUGIN_STATS
