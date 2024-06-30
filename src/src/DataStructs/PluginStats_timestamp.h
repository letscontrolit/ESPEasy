#ifndef HELPERS_PLUGINSTATS_TIMESTAMP_H
#define HELPERS_PLUGINSTATS_TIMESTAMP_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

# include "../DataStructs/PluginStats_size.h"

#define PLUGINSTATS_INTERNAL_TO_MICROS_RATIO 20000ull

// Timestamps are stored internally with 0.02 sec resolution.
// Stored timestamp will be based on the system micros
// This also implies there might be overflow issues when the 
// full period exceeds (2^32 / 50) seconds (~ 1000 days)
class PluginStats_timestamp {
public:

  typedef CircularBuffer<uint32_t, PLUGIN_STATS_NR_ELEMENTS> PluginStatsTimestamps_t;

  PluginStats_timestamp() = default;
  ~PluginStats_timestamp();

  bool     push(const int64_t& timestamp_sysmicros);

  bool     updateLast(const int64_t& timestamp_sysmicros);

  void     clear();

  // Update any logged timestamp with this newly set system time.
  void     processTimeSet(const double& time_offset);

  int64_t  getTimestamp(int lastNrSamples) const;

  // Compute the duration between first and last sample in seconds
  // For 0 or 1 samples, the period will be 0 seconds.
  uint32_t getFullPeriodInSec() const;

  int64_t operator[](PluginStatsTimestamps_t::index_t index) const;

private:

  // Conversion from system micros to internal timestamp
  static uint32_t systemMicros_to_internalTimestamp(const int64_t& timestamp_sysmicros);

  // Conversion from internal timestamp to system micros
  static int64_t internalTimestamp_to_systemMicros(const uint32_t& internalTimestamp);

  PluginStatsTimestamps_t _timestamps;
};

#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_TIMESTAMP_H
