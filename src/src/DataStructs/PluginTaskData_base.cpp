#include "../DataStructs/PluginTaskData_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"

#include "../WebServer/Chart_JS.h"
#include "../WebServer/HTML_wrappers.h"


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
