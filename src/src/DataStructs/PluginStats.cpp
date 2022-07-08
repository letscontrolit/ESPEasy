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

  for (; i < _samples.size(); ++i) {
    sum += _samples[i];
  }
  return sum / _samples.size();
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
  } else if (command == F("avg")) { // [taskname#valuename.avg] Average value of the last N kept samples
    value   = getSampleAvg();
    success = true;
  }

  if (success) {
    string = toString(value, _nrDecimals);
  }
  return success;
}
