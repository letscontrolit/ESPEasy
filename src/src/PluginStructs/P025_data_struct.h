#ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
#define PLUGINSTRUCTS_P025_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P025


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
# define P025_CAL_GET         bitRead(PCONFIG(3), 0)
# define P025_CAL_SET(x)      bitWrite(PCONFIG(3), 0, x)
# define P025_VOLT_OUT_GET    bitRead(PCONFIG(3), 1)
# define P025_VOLT_OUT_SET(x) bitWrite(PCONFIG(3), 1, x)

// If ever set, return set value, or else the default (0x04)
# define P025_SAMPLE_RATE_GET ((PCONFIG(3) & 0x4000) ? ((PCONFIG(3) >> 2) & 0x07) : 0x04)

// Set MSB bit (signed) to 1 to detect whether this was ever set.
# define P025_SAMPLE_RATE_SET(x) (PCONFIG(3) = (PCONFIG(3) & ~(0x07u << 2)) | ((x & 0x07) << 2) | 0x4000)


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
