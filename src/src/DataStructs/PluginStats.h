#ifndef HELPERS_PLUGINSTATS_H
#define HELPERS_PLUGINSTATS_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

# include "../DataStructs/ChartJS_dataset_config.h"
#include "../DataTypes/TaskIndex.h"


# include <CircularBuffer.h>

# ifndef PLUGIN_STATS_NR_ELEMENTS
#  ifdef ESP8266
#   define PLUGIN_STATS_NR_ELEMENTS 16
#  endif // ifdef ESP8266
#  ifdef ESP32
#   define PLUGIN_STATS_NR_ELEMENTS 64
#  endif // ifdef ESP32
# endif  // ifndef PLUGIN_STATS_NR_ELEMENTS

class PluginStats {
public:

  typedef CircularBuffer<float, PLUGIN_STATS_NR_ELEMENTS> PluginStatsBuffer_t;

  PluginStats() = delete;
  PluginStats(uint8_t nrDecimals,
              float   errorValue);


  // Add a sample to the _sample buffer
  // This does not also track peaks as the peaks could be raw sensor data and the samples processed data.
  bool push(float value);

  // Keep track of peaks.
  // Use this for sensors that need to take several samples before actually output a task value.
  // For example the ADC with oversampling
  void  trackPeak(float value);

  // Get lowest recorded value since reset
  float getPeakLow() const {
    return hasPeaks() ? _minValue : _errorValue;
  }

  // Get highest recorded value since reset
  float getPeakHigh() const {
    return hasPeaks() ? _maxValue : _errorValue;
  }

  bool hasPeaks() const {
    return _maxValue >= _minValue;
  }

  // Set the peaks to unset values
  void resetPeaks();

  void clearSamples() {
    _samples.clear();
  }

  size_t getNrSamples() const {
    return _samples.size();
  }

  // Compute average over all stored values
  float getSampleAvg() const {
    return getSampleAvg(_samples.size());
  }

  // Compute average over last N stored values
  float getSampleAvg(PluginStatsBuffer_t::index_t lastNrSamples) const;

  // Compute the standard deviation over all stored values
  float getSampleStdDev() const {
    return getSampleStdDev(_samples.size());
  }

  // Compute the standard deviation  over last N stored values
  float getSampleStdDev(PluginStatsBuffer_t::index_t lastNrSamples) const;


  float operator[](PluginStatsBuffer_t::index_t index) const;


  // Support task value notation to 'get' statistics
  // Notations like [taskname#taskvalue.avg] can then be used to compute the average over a number of samples.
  bool plugin_get_config_value_base(struct EventStruct *event,
                                    String            & string) const;

  bool webformLoad_show_stats(struct EventStruct *event) const;

  bool webformLoad_show_avg(struct EventStruct *event) const;
  bool webformLoad_show_stdev(struct EventStruct *event) const;
  bool webformLoad_show_peaks(struct EventStruct *event,
                              bool                include_peak_to_peak = true) const;
  void webformLoad_show_val(
    struct EventStruct *event,
    const String      & label,
    double              value,
    const String      & unit) const;


  const String& getLabel() const {
# if FEATURE_CHART_JS
    return _ChartJS_dataset_config.label;
# else // if FEATURE_CHART_JS
    return _label;
# endif // if FEATURE_CHART_JS
  }

  void setLabel(const String& label) {
# if FEATURE_CHART_JS
    _ChartJS_dataset_config.label = label;
# else // if FEATURE_CHART_JS
    _label = label;
# endif // if FEATURE_CHART_JS
  }

# if FEATURE_CHART_JS
  void plot_ChartJS_dataset() const;
# endif // if FEATURE_CHART_JS

# if FEATURE_CHART_JS

public:

  ChartJS_dataset_config _ChartJS_dataset_config;
# else // if FEATURE_CHART_JS

private:

  String _label;

public:

# endif // if FEATURE_CHART_JS

private:

  bool usableValue(float value) const;

  float _minValue;
  float _maxValue;

  PluginStatsBuffer_t _samples;
  float _errorValue;
  bool _errorValueIsNaN;

  uint8_t _nrDecimals = 3u;
};

class PluginStats_array {
public:

  PluginStats_array();
  ~PluginStats_array();

  void    initPluginStats(taskVarIndex_t taskVarIndex);
  void    clearPluginStats(taskVarIndex_t taskVarIndex);

  bool    hasStats() const;
  bool    hasPeaks() const;

  uint8_t nrSamplesPresent() const;

  void    pushPluginStatsValues(struct EventStruct *event,
                                bool                trackPeaks);

  bool    plugin_get_config_value_base(struct EventStruct *event,
                                       String            & string) const;

  bool    plugin_write_base(struct EventStruct *event,
                            const String      & string);

  bool    webformLoad_show_stats(struct EventStruct *event) const;

# if FEATURE_CHART_JS
  void    plot_ChartJS() const;
# endif // if FEATURE_CHART_JS


  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex) const;

  PluginStats* getPluginStats(taskVarIndex_t taskVarIndex);

private:

  PluginStats *_plugin_stats[VARS_PER_TASK] = { nullptr, };
};

#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_H
