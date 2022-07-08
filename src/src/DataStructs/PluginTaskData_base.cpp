#include "../DataStructs/PluginTaskData_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"

#include "../WebServer/Chart_JS.h"
#include "../WebServer/HTML_wrappers.h"


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

bool PluginTaskData_base::hasPluginStats() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      return true;
    }
  }
  return false;
}

bool PluginTaskData_base::hasPeaks() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr && _plugin_stats[i]->hasPeaks()) {
      return true;
    }
  }
  return false;
}

uint8_t PluginTaskData_base::nrSamplesPresent() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      return _plugin_stats[i]->getNrSamples();
    }
  }
  return 0;
}

void PluginTaskData_base::initPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    clearPluginStats(taskVarIndex);

    if (ExtraTaskSettings.enablePluginStats(taskVarIndex)) {
      _plugin_stats[taskVarIndex] = new PluginStats(
        ExtraTaskSettings.TaskDeviceValueDecimals[taskVarIndex],
        ExtraTaskSettings.TaskDeviceErrorValue[taskVarIndex]);

      if (_plugin_stats[taskVarIndex] != nullptr) {
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.label = ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex];
        const __FlashStringHelper *colors[] = { F("#A52422"), F("#BEA57D"), F("#EFF2C0"), F("#A4BAB7") };
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.color = colors[taskVarIndex];
      }
    }
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

bool PluginTaskData_base::plugin_get_config_value_base(struct EventStruct *event,
                                                       String            & string) const
{
  // Full value name is something like "taskvaluename.avg"
  const String fullValueName = parseString(string, 1);
  const String valueName     = parseString(fullValueName, 1, '.');

  for (uint8_t valueNr = 0; valueNr < VARS_PER_TASK; valueNr++)
  {
    if (_plugin_stats[valueNr] != nullptr) {
      // Check case insensitive, since the user entered value name can have any case.
      if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[valueNr]))
      {
        return _plugin_stats[valueNr]->plugin_get_config_value_base(event, string);
      }
    }
  }
  return false;
}

bool PluginTaskData_base::plugin_write_base(struct EventStruct *event, const String& string)
{
  bool success = false;

  const String cmd = parseString(string, 1);               // command

  const bool resetPeaks   = cmd.equals(F("resetpeaks"));   // Command: "taskname.resetPeaks"
  const bool clearSamples = cmd.equals(F("clearsamples")); // Command: "taskname.clearSamples"

  if (resetPeaks || clearSamples) {
    success = true;

    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        if (resetPeaks) {
          _plugin_stats[i]->resetPeaks();
        }

        if (clearSamples) {
          _plugin_stats[i]->clearSamples();
        }
      }
    }
  }

  return success;
}

bool PluginTaskData_base::webformLoad_show_stats(struct EventStruct *event) const
{
    bool somethingAdded = false;
    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        if (_plugin_stats[i]->webformLoad_show_stats(event)) somethingAdded = true;
      }
    }
    return somethingAdded;
}

void PluginTaskData_base::plot_ChartJS() const
{
  const uint8_t nrSamples = nrSamplesPresent();

  if (nrSamples == 0) { return; }

  // Chart Header
  add_ChartJS_chart_header(F("line"), F("TaskStatsChart"), F(""), 500, 500);

  // Add labels
  for (size_t i = 0; i < nrSamples; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtmlInt(i);
  }
  addHtml(F("],datasets: ["));


  // Data sets
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      _plugin_stats[i]->plot_ChartJS_dataset();
    }
  }
  add_ChartJS_chart_footer();
}
