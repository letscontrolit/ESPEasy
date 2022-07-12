#include "../DataStructs/PluginTaskData_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"

#include "../WebServer/Chart_JS.h"
#include "../WebServer/HTML_wrappers.h"

PluginTaskData_base::PluginTaskData_base()
  : _taskdata_pluginID(INVALID_PLUGIN_ID)
#ifdef USES_PLUGIN_STATS
  , _plugin_stats_array(nullptr)
#endif // ifdef USES_PLUGIN_STATS
{}

PluginTaskData_base::~PluginTaskData_base() {
#ifdef USES_PLUGIN_STATS
  delete _plugin_stats_array;
  _plugin_stats_array = nullptr;
#endif // ifdef USES_PLUGIN_STATS
}

bool PluginTaskData_base::hasPluginStats() const
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->hasStats();
  }
#endif // ifdef USES_PLUGIN_STATS
  return false;
}

bool PluginTaskData_base::hasPeaks() const
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->hasPeaks();
  }
#endif // ifdef USES_PLUGIN_STATS
  return false;
}

uint8_t PluginTaskData_base::nrSamplesPresent() const
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->nrSamplesPresent();
  }
#endif // ifdef USES_PLUGIN_STATS
  return 0;
}

#ifdef USES_PLUGIN_STATS
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

#endif // ifdef USES_PLUGIN_STATS
void PluginTaskData_base::pushPluginStatsValues(struct EventStruct *event, bool trackPeaks)
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    _plugin_stats_array->pushPluginStatsValues(event, trackPeaks);
  }
#endif // ifdef USES_PLUGIN_STATS
}

bool PluginTaskData_base::plugin_get_config_value_base(struct EventStruct *event,
                                                       String            & string) const
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->plugin_get_config_value_base(event, string);
  }
#endif // ifdef USES_PLUGIN_STATS
  return false;
}

bool PluginTaskData_base::plugin_write_base(struct EventStruct *event, const String& string)
{
#ifdef USES_PLUGIN_STATS

  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->plugin_write_base(event, string);
  }
#endif // ifdef USES_PLUGIN_STATS
  return false;
}

#ifdef USES_PLUGIN_STATS
bool PluginTaskData_base::webformLoad_show_stats(struct EventStruct *event) const
{
  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->webformLoad_show_stats(event);
  }
  return false;
}

# ifdef USES_CHART_JS
void PluginTaskData_base::plot_ChartJS() const
{
  if (_plugin_stats_array != nullptr) {
    _plugin_stats_array->plot_ChartJS();
  }
}

# endif // ifdef USES_CHART_JS

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

#endif // ifdef USES_PLUGIN_STATS
