#ifndef HELPERS_PLUGINSTATS_H
#define HELPERS_PLUGINSTATS_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

# include "../DataStructs/ChartJS_dataset_config.h"
# include "../DataStructs/PluginStats_size.h"
# include "../DataStructs/PluginStats_timestamp.h"
# include "../DataTypes/TaskIndex.h"


# if FEATURE_CHART_JS
#  include "../WebServer/Chart_JS_title.h"
# endif // if FEATURE_CHART_JS

class PluginStats {
public:

  typedef CircularBuffer<float, PLUGIN_STATS_NR_ELEMENTS> PluginStatsBuffer_t;

  PluginStats() = delete;
  PluginStats(uint8_t nrDecimals,
              float   errorValue);

  ~PluginStats();

  void processTimeSet(const double& time_offset);

  void setPluginStats_timestamp(PluginStats_timestamp *plugin_stats_timestamps)
  {
    _plugin_stats_timestamps = plugin_stats_timestamps;
  }

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

  uint32_t getPeakLowTimestamp() const {
    return hasPeaks() ? _minValueTimestamp : 0;
  }

  uint32_t getPeakHighTimestamp() const {
    return hasPeaks() ? _maxValueTimestamp : 0;
  }

  uint32_t getPeakLowLocalTimestamp() const;

  uint32_t getPeakHighLocalTimestamp() const;

  bool     hasPeaks() const {
    return _maxValue >= _minValue;
  }

  // Set the peaks to unset values
  void   resetPeaks();

  void   clearSamples();

  size_t getNrSamples() const;

  // Compute average over all stored values
  float  getSampleAvg() const;

  // Compute average over last N stored values
  float  getSampleAvg(PluginStatsBuffer_t::index_t lastNrSamples) const;

  // Compute the standard deviation over all stored values
  float  getSampleStdDev() const {
    return getSampleStdDev(getNrSamples());
  }

  // Compute average over all stored values, taking timestamp into account.
  // Returns average per second.
  float getSampleAvg_time(uint32_t& totalDuration) const {
    return getSampleAvg_time(getNrSamples(), totalDuration);
  }

  // Compute average over last N stored values, taking timestamp into account.
  // Returns average per second.
  float getSampleAvg_time(PluginStatsBuffer_t::index_t lastNrSamples,
                          uint32_t                   & totalDuration) const;

  // Compute the standard deviation  over last N stored values
  float getSampleStdDev(PluginStatsBuffer_t::index_t lastNrSamples) const;

  // Compute min/max over last N stored values
  float getSampleExtreme(PluginStatsBuffer_t::index_t lastNrSamples,
                         bool                         getMax) const;

  // Compute sample stored values
  float getSample(int lastNrSamples) const;

  float operator[](PluginStatsBuffer_t::index_t index) const;

private:

  static bool matchedCommand(const String             & command,
                             const __FlashStringHelper *cmd_match,
                             int                      & nrSamples);

public:

  // Support task value notation to 'get' statistics
  // Notations like [taskname#taskvalue.avg] can then be used to compute the average over a number of samples.
  bool plugin_get_config_value_base(struct EventStruct *event,
                                    String            & string) const;

  bool webformLoad_show_stats(struct EventStruct *event) const;

  bool webformLoad_show_avg(struct EventStruct *event) const;
  bool webformLoad_show_stdev(struct EventStruct *event) const;
  bool webformLoad_show_peaks(struct EventStruct *event,
                              bool                include_peak_to_peak = true) const;
  bool webformLoad_show_peaks_timestamp(struct EventStruct *event,
                                        const String      & labelPrefix) const;

  void webformLoad_show_val(
    struct EventStruct      *event,
    const String           & label,
    ESPEASY_RULES_FLOAT_TYPE value,
    const String           & unit) const;


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
  uint32_t _minValueTimestamp;
  uint32_t _maxValueTimestamp;

  PluginStatsBuffer_t *_samples = nullptr;
  float _errorValue;
  bool _errorValueIsNaN;

  uint8_t _nrDecimals = 3u;

  PluginStats_timestamp *_plugin_stats_timestamps = nullptr;
};


#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_H
