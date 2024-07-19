#include "../DataStructs/PluginStats.h"

#if FEATURE_PLUGIN_STATS
# include "../../_Plugin_Helper.h"

# include "../Globals/TimeZone.h"

# include "../Helpers/ESPEasy_math.h"
# include "../Helpers/Memory.h"

# include "../WebServer/Chart_JS.h"


PluginStats::PluginStats(uint8_t nrDecimals, float errorValue) :
  _errorValue(errorValue),
  _nrDecimals(nrDecimals),
  _plugin_stats_timestamps(nullptr)

{
  // Try to allocate in PSRAM if possible
  void *ptr = special_calloc(1, sizeof(PluginStatsBuffer_t));

  if (ptr == nullptr) { _samples = nullptr; }
  else {
    _samples = new (ptr) PluginStatsBuffer_t();
  }
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  _offset = 0.0f;
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  _errorValueIsNaN   = isnan(_errorValue);
  _minValue          = std::numeric_limits<ESPEASY_RULES_FLOAT_TYPE>::max();
  _maxValue          = std::numeric_limits<ESPEASY_RULES_FLOAT_TYPE>::lowest();
  _minValueTimestamp = 0;
  _maxValueTimestamp = 0;
}

PluginStats::~PluginStats()
{
  if (_samples != nullptr) {
    free(_samples);

    //    delete _samples;
  }
  _samples                 = nullptr;
  _plugin_stats_timestamps = nullptr;
}

void PluginStats::processTimeSet(const double& time_offset)
{
  // Check to see if there was a unix time set before the system time was set
  // For example when receiving data from a p2p node
  const int64_t cur_micros    = getMicros64();
  const int64_t offset_micros = time_offset * 1000000ull;

  if ((_maxValueTimestamp > cur_micros) && (_maxValueTimestamp > offset_micros)) {
    _maxValueTimestamp -= offset_micros;
  }

  if ((_minValueTimestamp > cur_micros) && (_minValueTimestamp > offset_micros)) {
    _minValueTimestamp -= offset_micros;
  }
}

bool PluginStats::push(ESPEASY_RULES_FLOAT_TYPE value)
{
  if (_samples == nullptr) { return false; }
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  if (_samples->isEmpty() && usableValue(value)) {
    // When the first value isn't usable, we keep the offset at 0 and thus loose accuracy
    _offset = value;
  }
  return _samples->push(value - _offset);
# else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return _samples->push(value);
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
}

bool PluginStats::matchesLastTwoEntries(ESPEASY_RULES_FLOAT_TYPE value) const
{
  const size_t nrSamples = getNrSamples();

  if (nrSamples < 2) { return false; }

# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return matchesLastTwoEntries(doubleToString(value, _nrDecimals));
# else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return matchesLastTwoEntries(toString(value, _nrDecimals));
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
}

bool PluginStats::matchesLastTwoEntries(const String& value_str) const
{
  const size_t nrSamples = getNrSamples();

  if (nrSamples < 2) { return false; }

  const ESPEASY_RULES_FLOAT_TYPE last       = get(nrSamples - 1);
  const ESPEASY_RULES_FLOAT_TYPE beforeLast = get(nrSamples - 2);

# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return
    doubleToString(last,       _nrDecimals).equals(value_str) &&
    doubleToString(beforeLast, _nrDecimals).equals(value_str);
# else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return
    toString(last,       _nrDecimals).equals(value_str) &&
    toString(beforeLast, _nrDecimals).equals(value_str);
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
}

void PluginStats::trackPeak(ESPEASY_RULES_FLOAT_TYPE value, int64_t timestamp)
{
  if ((value > _maxValue) || (value < _minValue)) {
    if (timestamp == 0) {
      // Make sure both extremes are flagged with the same timestamp.
      timestamp = getMicros64();
    }

    if (value > _maxValue) {
      _maxValueTimestamp = timestamp;
      _maxValue          = value;
    }

    if (value < _minValue) {
      _minValueTimestamp = timestamp;
      _minValue          = value;
    }
  }
}

void PluginStats::resetPeaks()
{
  _minValue          = std::numeric_limits<ESPEASY_RULES_FLOAT_TYPE>::max();
  _maxValue          = std::numeric_limits<ESPEASY_RULES_FLOAT_TYPE>::lowest();
  _minValueTimestamp = 0;
  _maxValueTimestamp = 0;
}

void PluginStats::clearSamples() {
  if (_samples != nullptr) {
    _samples->clear();
  }
}

size_t PluginStats::getNrSamples() const {
  if (_samples == nullptr) { return 0u; }
  return _samples->size();
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSampleAvg() const {
  return getSampleAvg(getNrSamples());
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSampleAvg(PluginStatsBuffer_t::index_t lastNrSamples) const
{
  const size_t nrSamples = getNrSamples();

  if (nrSamples == 0) { return _errorValue; }
  ESPEASY_RULES_FLOAT_TYPE sum{};

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < nrSamples) {
    i = nrSamples - lastNrSamples;
  }
  PluginStatsBuffer_t::index_t samplesUsed = 0;

  for (; i < nrSamples; ++i) {
    const ESPEASY_RULES_FLOAT_TYPE sample(get(i));

    if (usableValue(sample)) {
      ++samplesUsed;
      sum += sample;
    }
  }

  if (samplesUsed == 0) { return _errorValue; }
  return sum / samplesUsed;
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSampleAvg_time(PluginStatsBuffer_t::index_t lastNrSamples, uint64_t& totalDuration_usec) const
{
  const size_t nrSamples = getNrSamples();

  totalDuration_usec = 0u;

  if ((nrSamples == 0) || (_plugin_stats_timestamps == nullptr)) {
    return _errorValue;
  }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < nrSamples) {
    i = nrSamples - lastNrSamples;
  }

  int64_t lastTimestamp = 0;
  ESPEASY_RULES_FLOAT_TYPE lastValue{};
  bool lastValueUsable = false;
  ESPEASY_RULES_FLOAT_TYPE sum{};

  for (; i < nrSamples; ++i) {
    const ESPEASY_RULES_FLOAT_TYPE sample(get(i));
    const int64_t curTimestamp   = (*_plugin_stats_timestamps)[i];
    const bool    curValueUsable = usableValue(sample);

    if ((lastTimestamp != 0) && lastValueUsable) {
      const int64_t duration_usec = abs(timeDiff64(lastTimestamp, curTimestamp));

      if (curValueUsable) {
        // Old and new value usable, take average of this period.
        sum += ((lastValue + sample) / 2) * duration_usec;
      } else {
        // New value is not usable, so just add the last value for the duration.
        sum += lastValue * duration_usec;
      }
      totalDuration_usec += duration_usec;
    }

    lastValueUsable = curValueUsable;
    lastTimestamp   = curTimestamp;
    lastValue       = sample;
  }

  if (totalDuration_usec == 0) { return _errorValue; }
  return sum / totalDuration_usec;
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSampleStdDev(PluginStatsBuffer_t::index_t lastNrSamples) const
{
  const size_t nrSamples = getNrSamples();
  ESPEASY_RULES_FLOAT_TYPE variance{};
  const ESPEASY_RULES_FLOAT_TYPE average = getSampleAvg(lastNrSamples);

  if (!usableValue(average)) { return 0.0f; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < nrSamples) {
    i = nrSamples - lastNrSamples;
  }
  PluginStatsBuffer_t::index_t samplesUsed = 0;

  for (; i < nrSamples; ++i) {
    const ESPEASY_RULES_FLOAT_TYPE sample(get(i));

    if (usableValue(sample)) {
      ++samplesUsed;
      const ESPEASY_RULES_FLOAT_TYPE diff = sample - average;
      variance += diff * diff;
    }
  }

  if (samplesUsed < 2) { return 0.0f; }

  variance /= samplesUsed;
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return sqrt(variance);
# else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return sqrtf(variance);
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSampleExtreme(PluginStatsBuffer_t::index_t lastNrSamples, bool getMax) const
{
  const size_t nrSamples = getNrSamples();

  if (nrSamples == 0) { return _errorValue; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples < nrSamples) {
    i = nrSamples - lastNrSamples;
  }

  bool changed = false;

  ESPEASY_RULES_FLOAT_TYPE res = getMax ? INT_MIN : INT_MAX;

  for (; i < nrSamples; ++i) {
    const ESPEASY_RULES_FLOAT_TYPE sample(get(i));

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

ESPEASY_RULES_FLOAT_TYPE PluginStats::getSample(int lastNrSamples) const
{
  const size_t nrSamples = getNrSamples();

  if ((nrSamples == 0) || (nrSamples < abs(lastNrSamples))) { return _errorValue; }

  PluginStatsBuffer_t::index_t i = 0;

  if (lastNrSamples > 0) {
    i = nrSamples - lastNrSamples;
  } else if (lastNrSamples < 0) {
    i = abs(lastNrSamples) - 1;
  }

  if (i < nrSamples) {
    return get(i);
  }
  return _errorValue;
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::operator[](PluginStatsBuffer_t::index_t index) const
{
  return get(index);
}

ESPEASY_RULES_FLOAT_TYPE PluginStats::get(PluginStatsBuffer_t::index_t index) const
{
  const size_t nrSamples = getNrSamples();

  if (index < nrSamples) {
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    ESPEASY_RULES_FLOAT_TYPE res = _offset + (*_samples)[index];
    return res;
# else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    return (*_samples)[index];
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  }
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
        value   = getNrSamples();
        success = true;
      } else if (matchedCommand(command, F("sample"), nrSamples)) {
        success = nrSamples != 0;

        if (nrSamples == INT_MIN) {
          // [taskname#valuename.sample] Number of samples in memory.
          value   = getNrSamples();
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
    addRowLabel(concat(getLabel(),  F(" Average / sample")));
    addHtmlFloat(getSampleAvg(), (_nrDecimals == 0) ? 1 : _nrDecimals);
    addHtml(strformat(F(" (%u samples)"), getNrSamples()));

    if (_plugin_stats_timestamps != nullptr) {
      uint64_t totalDuration_usec = 0u;
      const float avg_per_sec     = getSampleAvg_time(totalDuration_usec);

      if (totalDuration_usec > 0) {
        addRowLabel(concat(getLabel(),  F(" Average / sec")));
        addHtmlFloat(avg_per_sec, (_nrDecimals == 0) ? 1 : _nrDecimals);
        addHtml(strformat(F(" (%s duration)"), secondsToDayHourMinuteSecond_ms(totalDuration_usec).c_str()));
      }
    }
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
    return webformLoad_show_peaks(
      event,
      getLabel(),
      toString(getPeakLow(),  _nrDecimals),
      toString(getPeakHigh(), _nrDecimals),
      include_peak_to_peak);
  }
  return false;
}

bool PluginStats::webformLoad_show_peaks(struct EventStruct *event,
                                         const String      & label,
                                         const String      & lowValue,
                                         const String      & highValue,
                                         bool                include_peak_to_peak) const
{
  if (hasPeaks() && (getNrSamples() > 1)) {
    uint32_t peakLow_frac{};
    uint32_t peakHigh_frac{};
    const uint32_t peakLow     = node_time.systemMicros_to_Unixtime(getPeakLowTimestamp(), peakLow_frac);
    const uint32_t peakHigh    = node_time.systemMicros_to_Unixtime(getPeakHighTimestamp(), peakHigh_frac);
    const uint32_t current     = node_time.getUnixTime();
    const bool     useTimeOnly = (current - peakLow) < 86400 && (current - peakHigh) < 86400;
    struct tm ts;
    breakTime(time_zone.toLocal(peakLow), ts);


    addRowLabel(concat(label,  F(" Peak Low")));
    addHtml(strformat(
              F("%s @ %s.%03u"),
              lowValue.c_str(),
              useTimeOnly
      ? formatTimeString(ts).c_str()
      : formatDateTimeString(ts).c_str(),
              unix_time_frac_to_millis(peakLow_frac)));


    breakTime(time_zone.toLocal(peakHigh), ts);

    addRowLabel(concat(label,  F(" Peak High")));
    addHtml(strformat(
              F("%s @ %s.%03u"),
              highValue.c_str(),
              useTimeOnly
      ? formatTimeString(ts).c_str()
      : formatDateTimeString(ts).c_str(),
              unix_time_frac_to_millis(peakHigh_frac)));

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
  const size_t nrSamples         = getNrSamples();

  for (; i < nrSamples; ++i) {
    if (i != 0) {
      addHtml(',');
    }

    if (!isnan(get(i))) {
      addHtmlFloat(get(i), _nrDecimals);
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
