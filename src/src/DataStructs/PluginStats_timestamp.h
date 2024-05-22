#ifndef HELPERS_PLUGINSTATS_TIMESTAMP_H
#define HELPERS_PLUGINSTATS_TIMESTAMP_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

# include "../DataStructs/PluginStats_size.h"

class PluginStats_timestamp {
public:

  typedef CircularBuffer<uint32_t, PLUGIN_STATS_NR_ELEMENTS> PluginStatsTimestamps_t;

  PluginStats_timestamp() = default;
  ~PluginStats_timestamp();

  bool     push(uint32_t unixTime);

  void     clear();


  uint32_t getTimestamp(int lastNrSamples) const;

  // Compute the duration between first and last sample in seconds
  // For 0 or 1 samples, the period will be 0 seconds.
  uint32_t getFullPeriodInSec() const;

  uint32_t operator[](PluginStatsTimestamps_t::index_t index) const;

private:

  PluginStatsTimestamps_t _timestamps;
};

#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_TIMESTAMP_H
