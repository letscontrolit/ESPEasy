#ifndef HELPERS_PLUGINSTATS_ARRAY_H
#define HELPERS_PLUGINSTATS_ARRAY_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

#include "../DataStructs/PluginStats.h"
#include "../DataStructs/PluginStats_timestamp.h"

# include "../DataStructs/ChartJS_dataset_config.h"
# include "../DataTypes/TaskIndex.h"


# if FEATURE_CHART_JS
#  include "../WebServer/Chart_JS_title.h"
# endif // if FEATURE_CHART_JS


class PluginStats_array {
public:

  PluginStats_array() = default;
  ~PluginStats_array();

  void   initPluginStats(taskVarIndex_t taskVarIndex);
  void   clearPluginStats(taskVarIndex_t taskVarIndex);

  bool   hasStats() const;
  bool   hasPeaks() const;

  size_t nrSamplesPresent() const;
  size_t nrPluginStats() const;

  // Compute the duration between first and last sample in seconds
  // For 0 or 1 samples, the period will be 0 seconds.
  uint32_t getFullPeriodInSec() const;
  
  void   pushPluginStatsValues(struct EventStruct *event,
                               bool                trackPeaks);

  bool   plugin_get_config_value_base(struct EventStruct *event,
                                      String            & string) const;

  bool   plugin_write_base(struct EventStruct *event,
                           const String      & string);

  bool   webformLoad_show_stats(struct EventStruct *event) const;

# if FEATURE_CHART_JS
  void   plot_ChartJS(bool onlyJSON = false) const;

  void   plot_ChartJS_scatter(
    taskVarIndex_t                values_X_axis_index,
    taskVarIndex_t                values_Y_axis_index,
    const __FlashStringHelper    *id,
    const ChartJS_title         & chartTitle,
    const ChartJS_dataset_config& datasetConfig,
    int                           width,
    int                           height,
    bool                          showAverage = true,
    const String                & options     = EMPTY_STRING,
    bool                          onlyJSON    = false) const;


# endif // if FEATURE_CHART_JS


  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex) const;

  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex);

private:

  PluginStats *_plugin_stats[VARS_PER_TASK] = {};
  PluginStats_timestamp *_plugin_stats_timestamps = nullptr;
};

#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_ARRAY_H
