#include "../DataStructs/PluginStats.h"

#if FEATURE_PLUGIN_STATS
# include "../../_Plugin_Helper.h"

# include "../Helpers/ESPEasy_math.h"

# include "../WebServer/Chart_JS.h"

PluginStats::PluginStats(uint8_t nrDecimals, float errorValue) :
  _errorValue(errorValue),
  _nrDecimals(nrDecimals)

{
  resetPeaks();
}

bool PluginStats::push(float value)
{
  return _samples.push(value);
}

void PluginStats::trackPeak(float value)
{
  if (value > _maxValue) { _maxValue = value; }

  if (value < _minValue) { _minValue = value; }
}

void PluginStats::resetPeaks()
{
  _minValue = std::numeric_limits<float>::max();
  _maxValue = std::numeric_limits<float>::min();
}

float PluginStats::getSampleAvg(PluginStatsBuffer_t::index_t lastNrSamples) const
{
  if (_samples.size() == 0) { return _errorValue; }
  float sum = 0.0f;

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < _samples.size()) {
    i = _samples.size() - lastNrSamples;
  }
  PluginStatsBuffer_t::index_t samplesUsed = 0;

  const bool errorValueIsNaN = isnan(_errorValue);

  for (; i < _samples.size(); ++i) {
    if (!isnan(_samples[i])) {
      if (errorValueIsNaN || !essentiallyEqual(_errorValue, _samples[i])) {
        ++samplesUsed;
        sum += _samples[i];
      }
    }
  }

  if (samplesUsed == 0) { return _errorValue; }
  return sum / samplesUsed;
}

float PluginStats::operator[](PluginStatsBuffer_t::index_t index) const
{
  if (index < _samples.size()) { return _samples[index]; }
  return _errorValue;
}

bool PluginStats::plugin_get_config_value_base(struct EventStruct *event, String& string) const
{
  bool success = false;

  // Full value name is something like "taskvaluename.avg"
  const String fullValueName = parseString(string, 1);
  const String command       = parseString(fullValueName, 2, '.');

  float value;

  if (command.equals(F("min"))) {        // [taskname#valuename.min] Lowest value seen since value reset
    value   = getPeakLow();
    success = true;
  } else if (command.equals(F("max"))) { // [taskname#valuename.max] Highest value seen since value reset
    value   = getPeakHigh();
    success = true;
  } else if (command.startsWith(F("avg"))) {
    if (command.equals(F("avg"))) { // [taskname#valuename.avg] Average value of the last N kept samples
      value   = getSampleAvg();
      success = true;
    } else {
      // Check for "avgN", where N is the number of most recent samples to use.
      int nrSamples = 0;

      if (validIntFromString(command.substring(3), nrSamples)) {
        if (nrSamples > 0) {
          // [taskname#valuename.avgN] Average over N most recent samples
          value   = getSampleAvg(nrSamples);
          success = true;
        }
      }
    }
  }

  if (success) {
    string = toString(value, _nrDecimals);
  }
  return success;
}

bool PluginStats::webformLoad_show_stats(struct EventStruct *event) const
{
  bool somethingAdded = false;

  if (webformLoad_show_avg(event)) { somethingAdded = true; }

  if (webformLoad_show_peaks(event)) { somethingAdded = true; }

  if (somethingAdded) {
    addFormSeparator(4);
  }

  return somethingAdded;
}

bool PluginStats::webformLoad_show_avg(struct EventStruct *event) const
{
  if (getNrSamples() > 0) {
    addRowLabel(getLabel() +  F(" Average"));
    addHtmlFloat(getSampleAvg(), _nrDecimals);
    addHtml(' ', '(');
    addHtmlInt(getNrSamples());
    addHtml(F(" samples)"));
    return true;
  }
  return false;
}

bool PluginStats::webformLoad_show_peaks(struct EventStruct *event) const
{
  if (hasPeaks()) {
    addRowLabel(getLabel() +  F(" Peak Low/High"));
    addHtmlFloat(getPeakLow(), _nrDecimals);
    addHtml('/');
    addHtmlFloat(getPeakHigh(), _nrDecimals);
    return true;
  }
  return false;
}

# if FEATURE_CHART_JS
void PluginStats::plot_ChartJS_dataset() const
{
  add_ChartJS_dataset_header(getLabel(), _ChartJS_dataset_config.color);

  PluginStatsBuffer_t::index_t i = 0;

  for (; i < _samples.size(); ++i) {
    if (i != 0) {
      addHtml(',');
    }

    if (!isnan(_samples[i])) {
      addHtmlFloat(_samples[i], _nrDecimals);
    }
    else {
      addHtml(F("null"));
    }
  }
  add_ChartJS_dataset_footer(_ChartJS_dataset_config.hidden);
}

# endif // if FEATURE_CHART_JS

PluginStats_array::PluginStats_array()
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    _plugin_stats[i] = nullptr;
  }
}

PluginStats_array::~PluginStats_array()
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      delete _plugin_stats[i];
      _plugin_stats[i] = nullptr;
    }
  }
}

void PluginStats_array::initPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    delete _plugin_stats[taskVarIndex];
    _plugin_stats[taskVarIndex] = nullptr;

    if (ExtraTaskSettings.enabledPluginStats(taskVarIndex)) {
      _plugin_stats[taskVarIndex] = new (std::nothrow) PluginStats(
        ExtraTaskSettings.TaskDeviceValueDecimals[taskVarIndex],
        ExtraTaskSettings.TaskDeviceErrorValue[taskVarIndex]);

      if (_plugin_stats[taskVarIndex] != nullptr) {
        _plugin_stats[taskVarIndex]->setLabel(ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex]);
        # if FEATURE_CHART_JS
        const __FlashStringHelper *colors[] = { F("#A52422"), F("#BEA57D"), F("#EFF2C0"), F("#A4BAB7") };
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.color = colors[taskVarIndex];
        # endif // if FEATURE_CHART_JS
      }
    }
  }
}

void PluginStats_array::clearPluginStats(taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    if (_plugin_stats[taskVarIndex] != nullptr) {
      delete _plugin_stats[taskVarIndex];
      _plugin_stats[taskVarIndex] = nullptr;
    }
  }
}

bool PluginStats_array::hasStats() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) { return true; }
  }
  return false;
}

bool PluginStats_array::hasPeaks() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if ((_plugin_stats[i] != nullptr) && _plugin_stats[i]->hasPeaks()) {
      return true;
    }
  }
  return false;
}

uint8_t PluginStats_array::nrSamplesPresent() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      return _plugin_stats[i]->getNrSamples();
    }
  }
  return 0;
}

void PluginStats_array::pushPluginStatsValues(struct EventStruct *event, bool trackPeaks)
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      _plugin_stats[i]->push(UserVar[event->BaseVarIndex + i]);

      if (trackPeaks) {
        _plugin_stats[i]->trackPeak(UserVar[event->BaseVarIndex + i]);
      }
    }
  }
}

bool PluginStats_array::plugin_get_config_value_base(struct EventStruct *event,
                                                     String            & string) const
{
  // Full value name is something like "taskvaluename.avg"
  const String fullValueName = parseString(string, 1);
  const String valueName     = parseString(fullValueName, 1, '.');

  for (uint8_t i = 0; i < VARS_PER_TASK; i++)
  {
    if (_plugin_stats[i] != nullptr) {
      // Check case insensitive, since the user entered value name can have any case.
      if (valueName.equalsIgnoreCase(getTaskValueName(event->TaskIndex, i)))
      {
        return _plugin_stats[i]->plugin_get_config_value_base(event, string);
      }
    }
  }
  return false;
}

bool PluginStats_array::plugin_write_base(struct EventStruct *event, const String& string)
{
  bool success     = false;
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

bool PluginStats_array::webformLoad_show_stats(struct EventStruct *event) const
{
  bool somethingAdded = false;

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      if (_plugin_stats[i]->webformLoad_show_stats(event)) { somethingAdded = true; }
    }
  }
  return somethingAdded;
}

# if FEATURE_CHART_JS
void PluginStats_array::plot_ChartJS() const
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

# endif // if FEATURE_CHART_JS


PluginStats * PluginStats_array::getPluginStats(taskVarIndex_t taskVarIndex) const
{
  if ((taskVarIndex < VARS_PER_TASK)) {
    return _plugin_stats[taskVarIndex];
  }
  return nullptr;
}

PluginStats * PluginStats_array::getPluginStats(taskVarIndex_t taskVarIndex)
{
  if ((taskVarIndex < VARS_PER_TASK)) {
    return _plugin_stats[taskVarIndex];
  }
  return nullptr;
}

#endif // if FEATURE_PLUGIN_STATS
