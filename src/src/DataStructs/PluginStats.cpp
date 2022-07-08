#include "../DataStructs/PluginStats.h"

#include "../../_Plugin_Helper.h"

PluginStats::PluginStats(uint8_t nrDecimals) : _nrDecimals(nrDecimals)
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

float PluginStats::getSampleAvg(uint8_t lastNrSamples) const
{
  if (_samples.size() == 0) { return 0.0f; }
  float sum = 0.0f;

  decltype(_samples)::index_t i = 0;

  if (lastNrSamples < _samples.size()) {
    i = _samples.size() - lastNrSamples;
  }
  decltype(_samples)::index_t samplesUsed = 0;

  for (; i < _samples.size(); ++i) {
    if (!isnan(_samples[i])) {
      ++samplesUsed;
      sum += _samples[i];
    }
  }

  if (samplesUsed == 0) { return 0.0f; }
  return sum / samplesUsed;
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
