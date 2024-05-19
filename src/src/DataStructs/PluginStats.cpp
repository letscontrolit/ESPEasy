#include "../DataStructs/PluginStats.h"

#if FEATURE_PLUGIN_STATS
# include "../../_Plugin_Helper.h"

# include "../Globals/TimeZone.h"

# include "../Helpers/ESPEasy_math.h"

# include "../WebServer/Chart_JS.h"

PluginStats::PluginStats(uint8_t nrDecimals, float errorValue) :
  _errorValue(errorValue),
  _nrDecimals(nrDecimals)

{
  _errorValueIsNaN   = isnan(_errorValue);
  _minValue          = std::numeric_limits<float>::max();
  _maxValue          = std::numeric_limits<float>::lowest();
  _minValueTimestamp = 0;
  _maxValueTimestamp = 0;
}

bool PluginStats::push(float value)
{
  return _samples.push(value);
}

void PluginStats::trackPeak(float value)
{
  if (value > _maxValue) {
    _maxValueTimestamp = node_time.getUnixTime();
    _maxValue          = value;
  }

  if (value < _minValue) {
    _minValueTimestamp = node_time.getUnixTime();
    _minValue          = value;
  }
}

uint32_t PluginStats::getPeakLowLocalTimestamp() const
{
  return time_zone.toLocal(getPeakLowTimestamp());
}

uint32_t PluginStats::getPeakHighLocalTimestamp() const
{
  return time_zone.toLocal(getPeakHighTimestamp());
}

void PluginStats::resetPeaks()
{
  _minValue          = std::numeric_limits<float>::max();
  _maxValue          = std::numeric_limits<float>::lowest();
  _minValueTimestamp = 0;
  _maxValueTimestamp = 0;
}

void PluginStats::clearSamples() {
  _samples.clear();
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
    addHtmlFloat(getSampleAvg(), (_nrDecimals == 0) ? 1 : _nrDecimals);
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
    addHtmlFloat(stdDev, (_nrDecimals == 0) ? 1 : _nrDecimals);
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
    addHtml(F(" / "));
    addHtmlFloat(getPeakHigh(), _nrDecimals);

    webformLoad_show_peaks_timestamp(event, getLabel());

    if (include_peak_to_peak) {
      addRowLabel(concat(getLabel(),  F(" Peak-to-peak")));
      addHtmlFloat(getPeakHigh() - getPeakLow(), _nrDecimals);
    }
    return true;
  }
  return false;
}

bool PluginStats::webformLoad_show_peaks_timestamp(struct EventStruct *event, const String& labelPrefix) const
{
  if (hasPeaks() && (getNrSamples() > 1)) {
    addRowLabel(concat(labelPrefix,  F(" Peak Low/High TimeStamp")));
    const uint32_t peakLow     = getPeakLowTimestamp();
    const uint32_t peakHigh    = getPeakHighTimestamp();
    const uint32_t current     = node_time.getUnixTime();
    const bool     useTimeOnly = (current - peakLow) < 86400 && (current - peakHigh) < 86400;
    struct tm ts;
    breakTime(time_zone.toLocal(peakLow), ts);
    addHtml(useTimeOnly ? formatTimeString(ts) : formatDateTimeString(ts));
    addHtml(F(" / "));
    breakTime(time_zone.toLocal(peakHigh), ts);
    addHtml(useTimeOnly ? formatTimeString(ts) : formatDateTimeString(ts));
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

#endif // if FEATURE_PLUGIN_STATS
