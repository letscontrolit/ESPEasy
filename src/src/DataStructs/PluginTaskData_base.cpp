#include "../DataStructs/PluginTaskData_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"

PluginTaskData_base::PluginTaskData_base() {
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    _plugin_stats[i] = nullptr;
  }
}

PluginTaskData_base::~PluginTaskData_base() {
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      delete _plugin_stats[i];
      _plugin_stats[i] = nullptr;
    }
  }
}

void PluginTaskData_base::initPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    clearPluginStats(taskVarIndex);
    _plugin_stats[taskVarIndex] = new PluginStats;
  }
}

void PluginTaskData_base::clearPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    if (_plugin_stats[taskVarIndex] != nullptr) {
      delete _plugin_stats[taskVarIndex];
      _plugin_stats[taskVarIndex] = nullptr;
    }
  }
}

void PluginTaskData_base::pushPluginStatsValues(struct EventStruct *event)
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      _plugin_stats[i]->push(UserVar[event->BaseVarIndex + i]);
    }
  }
}

bool PluginTaskData_base::plugin_write_base(struct EventStruct *event, const String& string)
{
  bool success = false;

  const String cmd = parseString(string, 1); // command

  if (cmd.equals(F("resetpeaks"))) {
    // Command: "adc,resetpeaks"
    success = true;

    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        _plugin_stats[i]->resetPeaks();
      }
    }
  }
  return success;
}
