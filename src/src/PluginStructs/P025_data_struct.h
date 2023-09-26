#ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
#define PLUGINSTRUCTS_P025_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P025


union P025_VARIOUS_BITS_t {
  struct {
    uint16_t cal           : 1;
    uint16_t outputVolt    : 1;
    uint16_t sampleRateSet : 1;
    uint16_t sampleRate    : 3;
    uint16_t unused        : 10;
  };
  int16_t pconfigvalue{};

  P025_VARIOUS_BITS_t(int16_t value) : pconfigvalue(value) {}

  uint16_t getSampleRate() const {
    if (sampleRateSet) { return sampleRate; }
    return 0x04; // Default sample rate
  }

  void setSampleRate(uint16_t sr) {
    sampleRate    = sr;
    sampleRateSet = 1;
  }
};


// For compatibility reasons with existing settings, the output selectors are located at:
// - PCONFIG(2)  ( = P025_MUX(0) )
// - PCONFIG(5)
// - PCONFIG(6)
// - PCONFIG(7)
# define P025_SENSOR_TYPE_INDEX 4 // Storing the output selector
# define P025_PCONFIG_INDEX(x) ((x == 0) ? 2 : x + P025_SENSOR_TYPE_INDEX)

# define P025_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P025_SENSOR_TYPE_INDEX)))

# define P025_I2C_ADDR        PCONFIG(0)
# define P025_GAIN            PCONFIG(1)
# define P025_MUX(x)          PCONFIG(P025_PCONFIG_INDEX(x))
# define P025_VARIOUS_BITS    PCONFIG(3)

# define P025_CAL_ADC1    PCONFIG_LONG(0)
# define P025_CAL_OUT1    PCONFIG_FLOAT(0)
# define P025_CAL_ADC2    PCONFIG_LONG(1)
# define P025_CAL_OUT2    PCONFIG_FLOAT(1)


const __FlashStringHelper* Plugin_025_valuename(uint8_t value_nr,
                                                bool    displayString);

enum class P025_sensorType {
  None,
  ADS1015,
  ADS1115
};


struct P025_data_struct : public PluginTaskData_base {
public:

  P025_data_struct(struct EventStruct *event);
  P025_data_struct()          = delete;
  virtual ~P025_data_struct() = default;

  bool        read(float        & value,
                   taskVarIndex_t index) const;

  static bool webformLoad(struct EventStruct *event);

  static bool webformSave(struct EventStruct *event);

  static bool webform_showConfig(struct EventStruct *event);

private:

  bool                   readConversionRegister025(int16_t& value) const;

  static P025_sensorType detectType(uint8_t i2cAddress);

  static bool            startMeasurement(uint8_t  i2cAddress,
                                          uint16_t configRegisterValue);

  // Check to see if the sensor is ready for new values
  // @retval detected SPS, 0 if timeout
  static long waitReady025(uint8_t i2cAddress);

  float    _fullScaleFactor{};
  uint16_t _configRegisterValue{};
  uint8_t  _i2cAddress{};
  uint8_t  _mux[VARS_PER_TASK]{};
};

#endif // ifdef USES_P025
#endif // ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
