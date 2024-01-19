#include "../DataStructs/PluginStats.h"

#if FEATURE_PLUGIN_STATS
# include "../../_Plugin_Helper.h"

# include "../Helpers/ESPEasy_math.h"

# include "../WebServer/Chart_JS.h"

PluginStats::PluginStats(uint8_t nrDecimals, float errorValue) :
  _errorValue(errorValue),
  _nrDecimals(nrDecimals)

{
  _errorValueIsNaN = isnan(_errorValue);
  _minValue        = std::numeric_limits<float>::max();
  _maxValue        = std::numeric_limits<float>::lowest();
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
  _maxValue = std::numeric_limits<float>::lowest();
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

  for (; i < _samples.size(); ++i) {
    const float sample(_samples[i]);

    if (usableValue(sample)) {
      ++samplesUsed;
      sum += sample;
    }
  }

  if (samplesUsed == 0) { return _errorValue; }
  return sum / samplesUsed;
}

float PluginStats::getSampleStdDev(PluginStatsBuffer_t::index_t lastNrSamples) const
{
  float variance      = 0.0f;
  const float average = getSampleAvg(lastNrSamples);

  if (!usableValue(average)) { return 0.0f; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < _samples.size()) {
    i = _samples.size() - lastNrSamples;
  }
  PluginStatsBuffer_t::index_t samplesUsed = 0;

  for (; i < _samples.size(); ++i) {
    const float sample(_samples[i]);

    if (usableValue(sample)) {
      ++samplesUsed;
      const float diff = sample - average;
      variance += diff * diff;
    }
  }

  if (samplesUsed < 2) { return 0.0f; }

  variance /= samplesUsed;
  return sqrtf(variance);
}

float PluginStats::getSampleExtreme(PluginStatsBuffer_t::index_t lastNrSamples, bool getMax) const
{
  if (_samples.size() == 0) { return _errorValue; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < _samples.size()) {
    i = _samples.size() - lastNrSamples;
  }

  bool changed = false;

  float res = getMax ? INT_MIN : INT_MAX;

  for (; i < _samples.size(); ++i) {
    const float sample(_samples[i]);

    if (usableValue(sample)) {
      if ((getMax && (sample > res)) ||
          (!getMax && (sample < res))) {
        changed = true;
        res     = sample;
      }
    }
  }

  if (!changed) { return _errorValue; }

  return res;
}

float PluginStats::getSample(int lastNrSamples) const
{
  if ((_samples.size() == 0) || (_samples.size() < abs(lastNrSamples))) { return _errorValue; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples > 0) {
    i = _samples.size() - lastNrSamples;
  } else if (lastNrSamples < 0) {
    i = abs(lastNrSamples) - 1;
  }

  if (i < _samples.size()) {
    return _samples[i];
  }
  return _errorValue;
}

float PluginStats::operator[](PluginStatsBuffer_t::index_t index) const
{
  if (index < _samples.size()) { return _samples[index]; }
  return _errorValue;
}

bool PluginStats::matchedCommand(const String& command, const __FlashStringHelper *cmd_match, int& nrSamples)
{
  const String cmd_match_str(cmd_match);

  if (command.equals(cmd_match_str)) {
    nrSamples = INT_MIN;
    return true;
  }

  if (command.startsWith(cmd_match_str)) {
    nrSamples = 0;

    // FIXME TD-er: ESP_IDF 5.x needs strict matching thus int32_t != int
    int32_t tmp{};

    if (validIntFromString(command.substring(cmd_match_str.length()), tmp)) {
      nrSamples = tmp;
      return true;
    }
  }
  return false;
}

bool PluginStats::plugin_get_config_value_base(struct EventStruct *event, String& string) const
{
  // Full value name is something like "taskvaluename.avg"
  const String fullValueName = parseString(string, 1);
  const String command       = parseString(fullValueName, 2, '.');

  if (command.isEmpty()) {
    return false;
  }

  float value{};
  int   nrSamples = 0;
  bool  success   = false;

  switch (command[0])
  {
    case 'a':

      if (matchedCommand(command, F("avg"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples < 0) { // [taskname#valuename.avg] Average value of the last N kept samples
          value = getSampleAvg();
        } else {
          // Check for "avgN", where N is the number of most recent samples to use.
          if (nrSamples > 0) {
            // [taskname#valuename.avgN] Average over N most recent samples
            value = getSampleAvg(nrSamples);
          }
        }
      }
      break;
    case 'm':

      if (matchedCommand(command, F("min"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples < 0) { // [taskname#valuename.min] Lowest value seen since value reset
          value = getPeakLow();
        } else {             // Check for "minN", where N is the number of most recent samples to use.
          if (nrSamples > 0) {
            value = getSampleExtreme(nrSamples, false);
          }
        }
      } else if (matchedCommand(command, F("max"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples < 0) { // [taskname#valuename.max] Highest value seen since value reset
          value = getPeakHigh();
        } else {             // Check for "maxN", where N is the number of most recent samples to use.
          if (nrSamples > 0) {
            value = getSampleExtreme(nrSamples, true);
          }
        }
      }
      break;
    case 's':

      if (matchedCommand(command, F("stddev"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples < 0) { // [taskname#valuename.stddev] Std deviation of the last N kept samples
          value = getSampleStdDev();
        } else {
          // Check for "stddevN", where N is the number of most recent samples to use.
          if (nrSamples > 0) {
            // [taskname#valuename.stddevN] Std. deviation over N most recent samples
            value = getSampleStdDev(nrSamples);
          }
        }
      } else if (matchedCommand(command, F("size"), nrSamples)) {
        // [taskname#valuename.size] Number of samples in memory
        value   = _samples.size();
        success = true;
      } else if (matchedCommand(command, F("sample"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples == INT_MIN) {
          // [taskname#valuename.sample] Number of samples in memory.
          value   = _samples.size();
          success = true;
        } else {
          if (nrSamples != 0) {
            // [taskname#valuename.sampleN]
            // With sample N:
            //   N > 0: Return N'th most recent sample
            //   N < 0: Return abs(N)'th sample in memory, starting at the oldest one.
            //   abs(N) > [number of samples]: return error value
            value = getSample(nrSamples);
          }
        }
      }
      break;
    default:
      return false;
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

  if (webformLoad_show_stdev(event)) { somethingAdded = true; }

  if (webformLoad_show_peaks(event)) { somethingAdded = true; }

  if (somethingAdded) {
    addFormSeparator(4);
  }

  return somethingAdded;
}

bool PluginStats::webformLoad_show_avg(struct EventStruct *event) const
{
  if (getNrSamples() > 0) {
    addRowLabel(concat(getLabel(),  F(" Average")));
    addHtmlFloat(getSampleAvg(), _nrDecimals);
    addHtml(strformat(F(" (%u samples)"), getNrSamples()));
    return true;
  }
  return false;
}

bool PluginStats::webformLoad_show_stdev(struct EventStruct *event) const
{
  const float stdDev = getSampleStdDev();

  if (usableValue(stdDev) && (getNrSamples() > 1)) {
    addRowLabel(concat(getLabel(),  F(" std. dev")));
    addHtmlFloat(stdDev, _nrDecimals);
    addHtml(strformat(F(" (%u samples)"), getNrSamples()));
    return true;
  }
  return false;
}

bool PluginStats::webformLoad_show_peaks(struct EventStruct *event, bool include_peak_to_peak) const
{
  if (hasPeaks() && (getNrSamples() > 1)) {
    addRowLabel(concat(getLabel(),  F(" Peak Low/High")));
    addHtmlFloat(getPeakLow(), _nrDecimals);
    addHtml('/');
    addHtmlFloat(getPeakHigh(), _nrDecimals);

    if (include_peak_to_peak) {
      addRowLabel(concat(getLabel(),  F(" Peak-to-peak")));
      addHtmlFloat(getPeakHigh() - getPeakLow(), _nrDecimals);
    }
    return true;
  }
  return false;
}

void PluginStats::webformLoad_show_val(
  struct EventStruct      *event,
  const String           & label,
  ESPEASY_RULES_FLOAT_TYPE value,
  const String           & unit) const
{
  addRowLabel(concat(getLabel(), label));
  addHtmlFloat(value, _nrDecimals);

  if (!unit.isEmpty()) {
    addUnit(unit);
  }
}

# if FEATURE_CHART_JS
void PluginStats::plot_ChartJS_dataset() const
{
  add_ChartJS_dataset_header(_ChartJS_dataset_config);

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
  add_ChartJS_dataset_footer();
}

# endif // if FEATURE_CHART_JS

bool PluginStats::usableValue(float value) const
{
  if (!isnan(value)) {
    if (_errorValueIsNaN || !essentiallyEqual(_errorValue, value)) {
      return true;
    }
  }
  return false;
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
      # ifdef USE_SECOND_HEAP
      HeapSelectIram ephemeral;
      # endif // ifdef USE_SECOND_HEAP

      _plugin_stats[taskVarIndex] = new (std::nothrow) PluginStats(
        ExtraTaskSettings.TaskDeviceValueDecimals[taskVarIndex],
        ExtraTaskSettings.TaskDeviceErrorValue[taskVarIndex]);

      if (_plugin_stats[taskVarIndex] != nullptr) {
        _plugin_stats[taskVarIndex]->setLabel(ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex]);
        # if FEATURE_CHART_JS
        const __FlashStringHelper *colors[] = { F("#A52422"), F("#BEA57D"), F("#0F4C5C"), F("#A4BAB7") };
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.color         = colors[taskVarIndex];
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.displayConfig = ExtraTaskSettings.getPluginStatsConfig(taskVarIndex);
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

size_t PluginStats_array::nrSamplesPresent() const
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      return _plugin_stats[i]->getNrSamples();
    }
  }
  return 0;
}

size_t PluginStats_array::nrPluginStats() const
{
  size_t res{};

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      ++res;
    }
  }
  return res;
}

void PluginStats_array::pushPluginStatsValues(struct EventStruct *event, bool trackPeaks)
{
  if (validTaskIndex(event->TaskIndex)) {
    const uint8_t valueCount      = getValueCountForTask(event->TaskIndex);
    const Sensor_VType sensorType = event->getSensorType();

    for (size_t i = 0; i < valueCount; ++i) {
      if (_plugin_stats[i] != nullptr) {
        const float value = UserVar.getAsDouble(event->TaskIndex, i, sensorType);
        _plugin_stats[i]->push(value);

        if (trackPeaks) {
          _plugin_stats[i]->trackPeak(value);
        }
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

  for (taskVarIndex_t i = 0; i < VARS_PER_TASK; i++)
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
  const String cmd = parseString(string, 1);                // command

  const bool resetPeaks   = equals(cmd, F("resetpeaks"));   // Command: "taskname.resetPeaks"
  const bool clearSamples = equals(cmd, F("clearsamples")); // Command: "taskname.clearSamples"

  if (resetPeaks || clearSamples) {
    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        if (resetPeaks) {
          success = true;
          _plugin_stats[i]->resetPeaks();
        }

        if (clearSamples) {
          success = true;
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
      if (_plugin_stats[i]->webformLoad_show_stats(event)) {
        somethingAdded = true;
      }
    }
  }
  return somethingAdded;
}

# if FEATURE_CHART_JS
void PluginStats_array::plot_ChartJS(bool onlyJSON) const
{
  const size_t nrSamples = nrSamplesPresent();

  if (nrSamples == 0) { return; }

  // Chart Header
  {
    ChartJS_options_scales scales;
    scales.add({ F("x") });

    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        ChartJS_options_scale scaleOption(
          _plugin_stats[i]->_ChartJS_dataset_config.displayConfig,
          _plugin_stats[i]->getLabel());
        scaleOption.axisTitle.color = _plugin_stats[i]->_ChartJS_dataset_config.color;
        scales.add(scaleOption);

        _plugin_stats[i]->_ChartJS_dataset_config.axisID = scaleOption.axisID;
      }
    }

    scales.update_Yaxis_TickCount();

    add_ChartJS_chart_header(
      F("line"),
      F("TaskStatsChart"),
      {},
      500 + (70 * (scales.nr_Y_scales() - 1)),
      500,
      scales.toString(),
      nrSamples,
      onlyJSON);
  }


  // Add labels
  addHtml(F("\"labels\":["));

  for (size_t i = 0; i < nrSamples; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtmlInt(i);
  }
  addHtml(F("],\n\"datasets\":["));


  // Data sets
  bool first = true;
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      if (!first) {
        addHtml(',');
      }
      first = false;
      _plugin_stats[i]->plot_ChartJS_dataset();
    }
  }
  add_ChartJS_chart_footer(onlyJSON);
}

void PluginStats_array::plot_ChartJS_scatter(
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
  const PluginStats *stats_X = getPluginStats(values_X_axis_index);
  const PluginStats *stats_Y = getPluginStats(values_Y_axis_index);

  if ((stats_X == nullptr) || (stats_Y == nullptr)) {
    return;
  }

  if ((stats_X->getNrSamples() < 2) || (stats_Y->getNrSamples() < 2)) {
    return;
  }

  String axisOptions;

  {
    ChartJS_options_scales scales;
    scales.add({ F("x"), stats_X->getLabel() });
    scales.add({ F("y"), stats_Y->getLabel() });
    axisOptions = scales.toString();
  }


  const size_t nrSamples = stats_X->getNrSamples();

  add_ChartJS_chart_header(
    F("scatter"),
    id,
    chartTitle,
    width,
    height,
    axisOptions,
    nrSamples,
    onlyJSON);

  // Add labels, which will be shown in a tooltip when hovering with the mouse over a point.
  addHtml(F("\"labels\":["));

  for (size_t i = 0; i < nrSamples; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtmlInt(i);
  }
  addHtml(F("],\n\"datasets\":["));

  // Long/Lat Coordinates
  add_ChartJS_dataset_header(datasetConfig);

  // Add scatter data
  for (size_t i = 0; i < nrSamples; ++i) {
    const float valX = (*stats_X)[i];
    const float valY = (*stats_Y)[i];
    add_ChartJS_scatter_data_point(valX, valY, 6);
  }

  add_ChartJS_dataset_footer(F("\"showLine\":true"));

  if (showAverage) {
    // Add single point showing the average
    addHtml(',');
    add_ChartJS_dataset_header(
    {
      F("Average"),
      F("#0F4C5C") });

    {
      const float valX = stats_X->getSampleAvg();
      const float valY = stats_Y->getSampleAvg();
      add_ChartJS_scatter_data_point(valX, valY, 6);
    }
    add_ChartJS_dataset_footer(F("\"pointRadius\":6,\"pointHoverRadius\":10"));
  }
  add_ChartJS_chart_footer(onlyJSON);
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
