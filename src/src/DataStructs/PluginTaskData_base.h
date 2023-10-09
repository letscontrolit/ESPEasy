#ifndef DATASTRUCTS_PLUGINTASKDATA_BASE_H
#define DATASTRUCTS_PLUGINTASKDATA_BASE_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/PluginStats.h"

#include "../DataTypes/PluginID.h"
#include "../DataTypes/TaskIndex.h"

// ==============================================
// Data used by instances of plugins.
// =============================================

// base class to be able to delete a data object from the array.
// N.B. in order to use this, a data object must inherit from this base class.
//      This is a compile time check.
struct PluginTaskData_base {
  PluginTaskData_base()
    : _taskdata_pluginID(INVALID_PLUGIN_ID)
#if FEATURE_PLUGIN_STATS
    , _plugin_stats_array(nullptr)
#endif // if FEATURE_PLUGIN_STATS
  {}

  virtual ~PluginTaskData_base()  {
#if FEATURE_PLUGIN_STATS
    delete _plugin_stats_array;
    _plugin_stats_array = nullptr;
#endif // if FEATURE_PLUGIN_STATS
  }

  bool baseClassOnly() const {
    return _baseClassOnly;
  }

  bool hasPluginStats() const {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->hasStats();
    }
#endif // if FEATURE_PLUGIN_STATS
    return false;
  }

  bool hasPeaks() const {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->hasPeaks();
    }
#endif // if FEATURE_PLUGIN_STATS
    return false;
  }

  size_t nrSamplesPresent() const {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->nrSamplesPresent();
    }
#endif // if FEATURE_PLUGIN_STATS
    return 0;
  }

  #if FEATURE_PLUGIN_STATS
  void initPluginStats(taskVarIndex_t taskVarIndex);
  void clearPluginStats(taskVarIndex_t taskVarIndex);
  #endif // if FEATURE_PLUGIN_STATS

  // Called right after successful PLUGIN_READ to store task values
  void pushPluginStatsValues(struct EventStruct *event,
                             bool                trackPeaks)
  {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      _plugin_stats_array->pushPluginStatsValues(event, trackPeaks);
    }
#endif // if FEATURE_PLUGIN_STATS
  }

  // Support task value notation to 'get' statistics
  // Notations like [taskname#taskvalue.avg] can then be used to compute the average over a number of samples.
  bool plugin_get_config_value_base(struct EventStruct *event,
                                    String            & string) const
  {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->plugin_get_config_value_base(event, string);
    }
#endif // if FEATURE_PLUGIN_STATS
    return false;
  }

  bool plugin_write_base(struct EventStruct *event,
                         const String      & string)
  {
#if FEATURE_PLUGIN_STATS

    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->plugin_write_base(event, string);
    }
#endif // if FEATURE_PLUGIN_STATS
    return false;
  }

#if FEATURE_PLUGIN_STATS
  bool webformLoad_show_stats(struct EventStruct *event) const
  {
    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->webformLoad_show_stats(event);
    }
    return false;
  }

# if FEATURE_CHART_JS
  void plot_ChartJS() const
  {
    if (_plugin_stats_array != nullptr) {
      _plugin_stats_array->plot_ChartJS();
    }
  }

# endif // if FEATURE_CHART_JS
#endif  // if FEATURE_PLUGIN_STATS

  // We cannot use dynamic_cast, so we must keep track of the plugin ID to
  // perform checks on the casting.
  // This is also a check to only use these functions and not to insert pointers
  // at random in the Plugin_task_data array.
  pluginID_t _taskdata_pluginID = INVALID_PLUGIN_ID;
#if FEATURE_PLUGIN_STATS

  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex) const
  {
    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->getPluginStats(taskVarIndex);
    }
    return nullptr;
  }

  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex)
  {
    if (_plugin_stats_array != nullptr) {
      return _plugin_stats_array->getPluginStats(taskVarIndex);
    }
    return nullptr;
  }

private:

  // Array of pointers to PluginStats. One per task value.
  PluginStats_array *_plugin_stats_array = nullptr;
#endif // if FEATURE_PLUGIN_STATS

protected:

  bool _baseClassOnly = false;
};

#endif // ifndef DATASTRUCTS_PLUGINTASKDATA_BASE_H
