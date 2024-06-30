#include "../DataStructs/PluginStats_array.h"

#if FEATURE_PLUGIN_STATS

# include "../../_Plugin_Helper.h"

# include "../Globals/TimeZone.h"

# include "../Helpers/ESPEasy_math.h"
# include "../Helpers/Memory.h"

# include "../WebServer/Chart_JS.h"

PluginStats_array::~PluginStats_array()
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_plugin_stats[i] != nullptr) {
      delete _plugin_stats[i];
      _plugin_stats[i] = nullptr;
    }
  }

  if (_plugin_stats_timestamps != nullptr) {
    free(_plugin_stats_timestamps);
    _plugin_stats_timestamps = nullptr;
  }
}

void PluginStats_array::initPluginStats(taskIndex_t taskIndex, taskVarIndex_t taskVarIndex)
{
  if (taskVarIndex < VARS_PER_TASK) {
    delete _plugin_stats[taskVarIndex];
    _plugin_stats[taskVarIndex] = nullptr;

    if (!hasStats()) {
      if (_plugin_stats_timestamps != nullptr) {
        free(_plugin_stats_timestamps);
        _plugin_stats_timestamps = nullptr;
      }
    }

    if (ExtraTaskSettings.enabledPluginStats(taskVarIndex)) {
      # ifdef USE_SECOND_HEAP
      HeapSelectIram ephemeral;
      # endif // ifdef USE_SECOND_HEAP

      // Try to allocate in PSRAM if possible
      constexpr unsigned size = sizeof(PluginStats);
      void *ptr               = special_calloc(1, size);

      if (ptr == nullptr) { _plugin_stats[taskVarIndex] = nullptr; }
      else {
        _plugin_stats[taskVarIndex] = new (ptr) PluginStats(
          ExtraTaskSettings.TaskDeviceValueDecimals[taskVarIndex],
          ExtraTaskSettings.TaskDeviceErrorValue[taskVarIndex]);
      }


      if (_plugin_stats[taskVarIndex] != nullptr) {
        _plugin_stats[taskVarIndex]->setLabel(ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex]);
        # if FEATURE_CHART_JS
        const __FlashStringHelper *colors[] = { F("#A52422"), F("#BEA57D"), F("#0F4C5C"), F("#A4BAB7") };
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.color         = colors[taskVarIndex];
        _plugin_stats[taskVarIndex]->_ChartJS_dataset_config.displayConfig = ExtraTaskSettings.getPluginStatsConfig(taskVarIndex);
        # endif // if FEATURE_CHART_JS

        if (_plugin_stats_timestamps != nullptr) {
          _plugin_stats[taskVarIndex]->setPluginStats_timestamp(_plugin_stats_timestamps);
        }
      }
    }
  }

  if (hasStats()) {
    if (_plugin_stats_timestamps == nullptr) {
      // Try to allocate in PSRAM if possible
      constexpr unsigned size = sizeof(PluginStats_timestamp);
      void *ptr               = special_calloc(1, size);

      if (ptr != nullptr) {
        // TODO TD-er: Let the task decide whether we need 1/50 sec resolution or 1/10
        // 1/10 sec resolution allows for ~13.6 years without overflow
        _plugin_stats_timestamps = new (ptr) PluginStats_timestamp(true);
      }

      for (size_t i = 0; i < VARS_PER_TASK; ++i) {
        _plugin_stats[taskVarIndex]->setPluginStats_timestamp(_plugin_stats_timestamps);
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

  if (!hasStats()) {
    if (_plugin_stats_timestamps != nullptr) {
      free(_plugin_stats_timestamps);
      _plugin_stats_timestamps = nullptr;
    }
  }
}

void PluginStats_array::processTimeSet(const double& time_offset)
{
  if (_plugin_stats_timestamps != nullptr) {
    _plugin_stats_timestamps->processTimeSet(time_offset);
  }

  // Also update timestamps of peaks
  for (taskVarIndex_t taskVarIndex = 0; taskVarIndex < VARS_PER_TASK; ++taskVarIndex) {
    PluginStats *stats = getPluginStats(taskVarIndex);

    if (stats != nullptr) {
      stats->processTimeSet(time_offset);
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

uint32_t PluginStats_array::getFullPeriodInSec(uint32_t& time_frac) const
{
  if (_plugin_stats_timestamps == nullptr) {
    time_frac = 0u;
    return 0u;
  }
  return _plugin_stats_timestamps->getFullPeriodInSec(time_frac);
}

void PluginStats_array::pushPluginStatsValues(
  struct EventStruct *event,
  bool                trackPeaks,
  bool                onlyUpdateTimestampWhenSame)
{
  if (validTaskIndex(event->TaskIndex)) {
    const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

    if (valueCount > 0) {
      const Sensor_VType sensorType = event->getSensorType();

      const int64_t timestamp_sysmicros = event->getTimestamp_as_systemMicros();

      if (onlyUpdateTimestampWhenSame && (_plugin_stats_timestamps != nullptr)) {
        // When only updating the timestamp of the last entry,
        // we should look at the last 2 entries to see if they are the same.
        bool   isSame = true;
        size_t i      = 0;

        while (isSame && i < valueCount) {
          if (_plugin_stats[i] != nullptr) {
            const float value = UserVar.getAsDouble(event->TaskIndex, i, sensorType);

            if (!_plugin_stats[i]->matchesLastTwoEntries(value)) {
              isSame = false;
            }
          }
          ++i;
        }

        if (isSame) {
          _plugin_stats_timestamps->updateLast(timestamp_sysmicros);
          return;
        }
      }

      if (_plugin_stats_timestamps != nullptr) {
        _plugin_stats_timestamps->push(timestamp_sysmicros);
      }

      for (size_t i = 0; i < valueCount; ++i) {
        if (_plugin_stats[i] != nullptr) {
          const float value = UserVar.getAsDouble(event->TaskIndex, i, sensorType);
          _plugin_stats[i]->push(value);

          if (trackPeaks) {
            _plugin_stats[i]->trackPeak(value, timestamp_sysmicros);
          }
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

  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  for (taskVarIndex_t i = 0; i < valueCount; i++)
  {
    if (_plugin_stats[i] != nullptr) {
      // Check case insensitive, since the user entered value name can have any case.
      if (valueName.equalsIgnoreCase(Cache.getTaskDeviceValueName(event->TaskIndex, i)))
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

bool PluginStats_array::webformLoad_show_stats(struct EventStruct *event, bool showTaskValues) const
{
  bool somethingAdded = false;

  uint32_t time_frac{};
  const uint32_t duration  = getFullPeriodInSec(time_frac);
  const uint32_t nrSamples = nrSamplesPresent();

  if ((duration > 0) && (nrSamples > 1)) {
    const uint32_t duration_millis = unix_time_frac_to_millis(time_frac);
    addRowLabel(F("Total Duration"));
    addHtml(strformat(
              F("%s.%03u (%u.%03u sec)"),
              secondsToDayHourMinuteSecond(duration).c_str(),
              duration_millis,
              duration,
              duration_millis));
    const float duration_f = static_cast<float>(duration) + (duration_millis / 1000.0f);
    addRowLabel(F("Total Nr Samples"));
    addHtmlInt(nrSamples);
    addRowLabel(F("Avg Rate"));
    addHtmlFloat(duration_f / static_cast<float>(nrSamples - 1), 2);
    addUnit(F("sec/sample"));
    addFormSeparator(4);
    somethingAdded = true;
  }

  if (showTaskValues) {
    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      if (_plugin_stats[i] != nullptr) {
        if (_plugin_stats[i]->webformLoad_show_stats(event)) {
          somethingAdded = true;
        }
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
    {
      ChartJS_options_scale scaleOption(F("x"));

      if (_plugin_stats_timestamps != nullptr) {
        scaleOption.scaleType = F("time");
      }
      scales.add(scaleOption);
    }

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

    const bool enableZoom = true;

    add_ChartJS_chart_header(
      F("line"),
      F("TaskStatsChart"),
      {},
      500 + (70 * (scales.nr_Y_scales() - 1)),
      500,
      scales.toString(),
      enableZoom,
      nrSamples,
      onlyJSON);
  }


  // Add labels
  addHtml(F("\"labels\":["));

  for (size_t i = 0; i < nrSamples; ++i) {
    if (i != 0) {
      addHtml(',');
    }

    if (_plugin_stats_timestamps != nullptr) {
      struct tm ts;
      uint32_t  unix_time_frac{};
      const uint32_t uinxtime_sec    = node_time.systemMicros_to_Unixtime((*_plugin_stats_timestamps)[i], unix_time_frac);
      const uint32_t local_timestamp = time_zone.toLocal(uinxtime_sec);
      breakTime(local_timestamp, ts);
      addHtml('"');
      addHtml(formatDateTimeString(ts));
      addHtml(strformat(F(".%03u"), unix_time_frac_to_millis(unix_time_frac)));
      addHtml('"');
    } else {
      addHtmlInt(i);
    }
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


  const size_t nrSamples  = stats_X->getNrSamples();
  const bool   enableZoom = false;

  add_ChartJS_chart_header(
    F("scatter"),
    id,
    chartTitle,
    width,
    height,
    axisOptions,
    enableZoom,
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
