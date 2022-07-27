#ifndef PLUGINSTRUCTS_P002_DATA_STRUCT_H
#define PLUGINSTRUCTS_P002_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P002

# include "src/Helpers/Hardware.h"
# include <vector>

# ifdef ESP32

// Needed to get ADC Vref
  #  include <esp_adc_cal.h>
  #  include <driver/adc.h>
# endif // ifdef ESP32


# define P002_OVERSAMPLING        PCONFIG(0)
# ifdef ESP32
#  define P002_APPLY_FACTORY_CALIB  PCONFIG(1)
#  define P002_ATTENUATION          PCONFIG(2)
# endif // ifdef ESP32
# define P002_CALIBRATION_ENABLED PCONFIG(3)
# define P002_CALIBRATION_POINT1  PCONFIG_LONG(0)
# define P002_CALIBRATION_POINT2  PCONFIG_LONG(1)
# define P002_CALIBRATION_VALUE1  PCONFIG_FLOAT(0)
# define P002_CALIBRATION_VALUE2  PCONFIG_FLOAT(1)

# define P002_MULTIPOINT_ENABLED  PCONFIG(4)
# define P002_NR_MULTIPOINT_ITEMS PCONFIG(5)

# define P002_USE_CURENT_SAMPLE   0
# define P002_USE_OVERSAMPLING    1
# define P002_USE_BINNING         2

// FIXME TD-er: Must test if HTML POST on ESP8266 will not take too much ram on save
# define P002_MAX_NR_MP_ITEMS     64

// We store the multipoint values and formula in a number of strings
// These will be stored in CustomTaskSettings
# define P002_SAVED_NR_LINES      0
# define P002_LINE_INDEX_FORMULA  1

# define P002_LINE_IDX_FIRST_MP   6 // Leave some room for extra lines in the settings later
# define P002_STRINGS_PER_MP      2 // Nr of items per multi-point set
# define P002_Nlines              (P002_LINE_IDX_FIRST_MP + (P002_STRINGS_PER_MP * (P002_NR_MULTIPOINT_ITEMS)))
# define P002_MAX_FORMULA_LENGTH  64

// Need to define the attenuation values to make sure no old or uninitialized value may be setting this to the wrong value.
# define P002_ADC_0db              (ADC_ATTEN_DB_0  + 10)
# define P002_ADC_2_5db            (ADC_ATTEN_DB_2_5 + 10)
# define P002_ADC_6db              (ADC_ATTEN_DB_6 + 10)
# define P002_ADC_11db             (ADC_ATTEN_DB_11 + 10)


struct P002_ADC_Value_pair {
  P002_ADC_Value_pair(float adc, float value) : _adc(adc), _value(value) {}

  P002_ADC_Value_pair(const P002_ADC_Value_pair&) = default;

  P002_ADC_Value_pair& operator=(const P002_ADC_Value_pair&) = default;

  P002_ADC_Value_pair& operator=(P002_ADC_Value_pair&&) = default;


  // Needed to sort based on ADC value
  bool operator<(const P002_ADC_Value_pair& other) const {
    return this->_adc < other._adc;
  }

  float _adc;
  float _value;
};

struct P002_binningRange {
  void set(int currentValue) {
    if (currentValue > _maxADC) {
      _maxADC = currentValue;
    }

    if (currentValue < _minADC) {
      _minADC = currentValue;
    }
  }

  bool inRange(int currentValue) const {
    return _minADC <= currentValue && currentValue <= _maxADC;
  }

  int _minADC = INT_MAX;
  int _maxADC = INT_MIN;
};

struct P002_data_struct : public PluginTaskData_base {
  P002_data_struct(struct EventStruct *event);

private:

# ifndef LIMIT_BUILD_SIZE
  void load(struct EventStruct *event);
# endif // ifndef LIMIT_BUILD_SIZE

  void webformLoad_2p_calibPoint(
    const __FlashStringHelper *label,
    const __FlashStringHelper *id_point,
    const __FlashStringHelper *id_value,
    int                        point,
    float                      value) const;

public:

  void webformLoad(struct EventStruct *event);

# ifdef USES_PLUGIN_STATS
  bool webformLoad_show_stats(struct EventStruct *event);
# endif // ifdef USES_PLUGIN_STATS

private:

  void formatADC_statistics(const __FlashStringHelper *label,
                            int                        raw,
                            bool                       includeOutputValue = false) const;
  void format_2point_calib_statistics(const __FlashStringHelper *label,
                                      int                        raw,
                                      float                      float_value) const;

# ifdef ESP32
  static adc_atten_t                getAttenuation(struct EventStruct *event);
  static const __FlashStringHelper* AttenuationToString(adc_atten_t attenuation);
  #  ifdef USES_CHART_JS
  static void                       webformLoad_calibrationCurve(struct EventStruct *event);
  #  endif // ifdef USES_CHART_JS
# endif    // ifdef ESP32

# ifdef USES_CHART_JS
  static const __FlashStringHelper* getChartXaxisLabel(struct EventStruct *event);
# endif // ifdef USES_CHART_JS
  static void                       getInputRange(struct EventStruct *event,
                                                  int               & min_value,
                                                  int               & max_value,
                                                  bool                ignoreCalibration = false);
# ifdef USES_CHART_JS
  static void getChartRange(struct EventStruct *event,
                            int                 values[],
                            int                 count,
                            bool                ignoreCalibration = false);

  static void webformLoad_2pt_calibrationCurve(struct EventStruct *event);

  void        webformLoad_multipointCurve(struct EventStruct *event) const;
# endif // ifdef USES_CHART_JS

public:

  static String webformSave(struct EventStruct *event);

  void          takeSample();

  bool          getValue(float& float_value,
                         int  & raw_value) const;

  void          reset();

private:

  void resetOversampling();

  void addOversamplingValue(int currentValue);

  bool getOversamplingValue(float& float_value,
                            int  & raw_value) const;

private:

# ifndef LIMIT_BUILD_SIZE

  // Get index of the bin to match.
  // Return -1 if no bin matched.
  int  getBinIndex(float currentValue) const;

  int  computeADC_to_bin(const int& currentValue) const;

  void addBinningValue(int currentValue);

  bool getBinnedValue(float& float_value,
                      int  & raw_value) const;
# endif // ifndef LIMIT_BUILD_SIZE

public:

  // This needs to be a static function, as the object may not exist if the task is not enabled.
  static float applyCalibration(struct EventStruct *event,
                                float               float_value,
                                bool                force = false);

  static float getCurrentValue(struct EventStruct *event,
                               int               & raw_value);

  float        applyCalibration(float float_value) const;

# ifdef ESP32
  static bool  useFactoryCalibration(struct EventStruct *event);

  static float applyFactoryCalibration(float       raw_value,
                                       adc_atten_t attenuation);


# endif // ifdef ESP32

private:

# ifndef LIMIT_BUILD_SIZE
  float        applyMultiPointInterpolation(float float_value, bool force = false) const;
# endif // ifndef LIMIT_BUILD_SIZE

  static float mapADCtoFloat(float float_value,
                             float adc1,
                             float adc2,
                             float out1,
                             float out2);

public:

  uint16_t OversamplingCount = 0;

private:

  int32_t OversamplingValue  = 0;
  int16_t OversamplingMinVal = MAX_ADC_VALUE;
  int16_t OversamplingMaxVal = -MAX_ADC_VALUE;

  int   _calib_adc1 = 0;
  int   _calib_adc2 = 0;
  float _calib_out1 = 0.0f;
  float _calib_out2 = 0.0f;

  bool _use2pointCalibration = false;
# ifndef LIMIT_BUILD_SIZE
  std::vector<P002_ADC_Value_pair>_multipoint;
  std::vector<unsigned int>       _binning;
  std::vector<P002_binningRange>  _binningRange;
  bool                            _useMultipoint = false;
# endif // ifndef LIMIT_BUILD_SIZE

  int _pin_analogRead = -1;

  uint8_t _sampleMode = P002_USE_CURENT_SAMPLE;

  uint8_t _nrDecimals = 0;
# ifndef LIMIT_BUILD_SIZE
  uint8_t _nrMultiPointItems = 0;
  String  _formula;
  String  _formula_preprocessed;
# endif // ifndef LIMIT_BUILD_SIZE
# ifdef ESP32
  bool        _useFactoryCalibration = false;
  adc_atten_t _attenuation           = ADC_ATTEN_DB_11;
# endif // ifdef ESP32
};


#endif // ifdef USES_P002
#endif // ifndef PLUGINSTRUCTS_P002_DATA_STRUCT_H
