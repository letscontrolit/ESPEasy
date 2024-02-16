#include "../DataStructs/PluginTaskData_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"

#include "../WebServer/Chart_JS.h"
#include "../WebServer/HTML_wrappers.h"


PluginTaskData_base::PluginTaskData_base()
  : _taskdata_pluginID(INVALID_PLUGIN_ID)
#if FEATURE_PLUGIN_STATS
  , _plugin_stats_array(nullptr)
#endif // if FEATURE_PLUGIN_STATS
{}


PluginTaskData_base::~PluginTaskData_base()  {
#if FEATURE_PLUGIN_STATS
  delete _plugin_stats_array;
  _plugin_stats_array = nullptr;
#endif // if FEATURE_PLUGIN_STATS
}

bool PluginTaskData_base::hasPluginStats() const {
#if FEATURE_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->hasStats();
  }
#endif // if FEATURE_PLUGIN_STATS
  return false;
}

bool PluginTaskData_base::hasPeaks() const {
#if FEATURE_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->hasPeaks();
  }
#endif // if FEATURE_PLUGIN_STATS
  return false;
}

size_t PluginTaskData_base::nrSamplesPresent() const {
#if FEATURE_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->nrSamplesPresent();
  }
#endif // if FEATURE_PLUGIN_STATS
  return 0;
}

#if FEATURE_PLUGIN_STATS
void PluginTaskData_base::initPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    if (_plugin_stats_array == nullptr) {
      _plugin_stats_array = new (std::nothrow) PluginStats_array();
    }

    if (_plugin_stats_array != nullptr) {
      _plugin_stats_array->initPluginStats(taskVarIndex);
    }
  }
}

void PluginTaskData_base::clearPluginStats(taskVarIndex_t taskVarIndex)
{
  if ((taskVarIndex < VARS_PER_TASK) && _plugin_stats_array) {
    _plugin_stats_array->clearPluginStats(taskVarIndex);

    if (!_plugin_stats_array->hasStats()) {
      delete _plugin_stats_array;
      _plugin_stats_array = nullptr;
    }
  }
}

#endif // if FEATURE_PLUGIN_STATS

void PluginTaskData_base::pushPluginStatsValues(struct EventStruct *event,
                                                bool                trackPeaks)
{
#if FEATURE_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    _plugin_stats_array->pushPluginStatsValues(event, trackPeaks);
  }
#endif // if FEATURE_PLUGIN_STATS
}

bool PluginTaskData_base::plugin_get_config_value_base(struct EventStruct *event,
                                                       String            & string) const
{
#if FEATURE_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->plugin_get_config_value_base(event, string);
  }
#endif // if FEATURE_PLUGIN_STATS
  return false;
}

bool PluginTaskData_base::plugin_write_base(struct EventStruct *event,
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
bool PluginTaskData_base::webformLoad_show_stats(struct EventStruct *event) const
{
  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->webformLoad_show_stats(event);
  }
  return false;
}

# if FEATURE_CHART_JS
void PluginTaskData_base::plot_ChartJS(bool onlyJSON) const
{
  if (_plugin_stats_array != nullptr) {
    _plugin_stats_array->plot_ChartJS(onlyJSON);
  }
}

void PluginTaskData_base::plot_ChartJS_scatter(
  taskVarIndex_t                values_X_axis_index,
  taskVarIndex_t                values_Y_axis_index,
  const __FlashStringHelper    *id,
  const ChartJS_title         & chartTitle,
  const ChartJS_dataset_config& datasetConfig,
  int                           width,
  int                           height,
  bool                          showAverage,
  const String                & options,
  bool                          onlyJSON) const
{
  if (_plugin_stats_array != nullptr) {
    _plugin_stats_array->plot_ChartJS_scatter(
      values_X_axis_index,
      values_Y_axis_index,
      id,
      chartTitle,
      datasetConfig,
      width,
      height,
      showAverage,
      options,
      onlyJSON);
  }
}

# endif // if FEATURE_CHART_JS


PluginStats * PluginTaskData_base::getPluginStats(taskVarIndex_t taskVarIndex) const
{
  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->getPluginStats(taskVarIndex);
  }
  return nullptr;
}

PluginStats * PluginTaskData_base::getPluginStats(taskVarIndex_t taskVarIndex)
{
  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->getPluginStats(taskVarIndex);
  }
  return nullptr;
}

#endif // if FEATURE_PLUGIN_STATS
