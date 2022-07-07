#include "../DataStructs/PluginStats.h"

#include "../../_Plugin_Helper.h"

PluginStats::PluginStats()
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
  return false;
}
