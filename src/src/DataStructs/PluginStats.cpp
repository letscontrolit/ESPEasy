#include "../DataStructs/PluginStats.h"

#include "../../_Plugin_Helper.h"

#include "../Helpers/ESPEasy_math.h"

#include "../WebServer/Chart_JS.h"

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

  if (command == F("min")) {        // [taskname#valuename.min] Lowest value seen since value reset
    value   = getPeakLow();
    success = true;
  } else if (command == F("max")) { // [taskname#valuename.max] Highest value seen since value reset
    value   = getPeakHigh();
    success = true;
  } else if (command.startsWith(F("avg"))) {
    if (command == F("avg")) { // [taskname#valuename.avg] Average value of the last N kept samples
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

  if (hasPeaks()) {
    addRowLabel(_ChartJS_dataset_config.label +  F(" Peak Low/High"));
    addHtmlFloat(getPeakLow(), _nrDecimals);
    addHtml('/');
    addHtmlFloat(getPeakHigh(), _nrDecimals);
    somethingAdded = true;
  }

  if (getNrSamples() > 0) {
    addRowLabel(F("Avg. ouput value"));
    addHtmlFloat(getSampleAvg());
    addHtml(' ', '(');
    addHtmlInt(getNrSamples());
    addHtml(F(" samples)"));
    somethingAdded = true;
  }

  if (somethingAdded) {
    addFormSeparator(4);
  }

  return somethingAdded;
}

void PluginStats::plot_ChartJS_dataset() const
{
  add_ChartJS_dataset_header(_ChartJS_dataset_config.label, _ChartJS_dataset_config.color);

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
