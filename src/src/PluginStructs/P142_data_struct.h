#ifndef PLUGINSTRUCTS_P142_DATA_STRUCT_H
#define PLUGINSTRUCTS_P142_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P142

# include <AS5600.h>

# define P142_QUERY1_CONFIG_POS   0
# define P142_SENSOR_TYPE_INDEX   (P142_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P142_NR_OUTPUT_VALUES    getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P142_SENSOR_TYPE_INDEX)))

# define P142_I2C_ADDRESS       PCONFIG(P142_SENSOR_TYPE_INDEX + 1)
# define P142_START_POSITION    PCONFIG(P142_SENSOR_TYPE_INDEX + 2)
# define P142_MAX_POSITION      PCONFIG(P142_SENSOR_TYPE_INDEX + 3)

# define P142_PRESET_ANGLE      PCONFIG_FLOAT(0)

# define P142_CONFIG_FLAGS              PCONFIG_ULONG(0) // All flags
# define P142_CONFIG_COUNTER_CLOCKWISE  0                // Flag indexes
# define P142_CONFIG_USE_SPEED          1
# define P142_CONFIG_OUTPUT_MODE        2
# define P142_CONFIG_DIFF_ONLY          3
# define P142_CONFIG_POWER_MODE         4  // 2 bits
# define P142_CONFIG_HYSTERESIS         6  // 2 bits
# define P142_CONFIG_SLOW_FILTER        8  // 2 bits
# define P142_CONFIG_FAST_FILTER        10 // 3 bits
# define P142_CONFIG_WATCHDOG           13
# define P142_CONFIG_ENABLE_LOG         14
# define P142_CONFIG_CHECK_MAGNET       15

# define P142_GET_COUNTER_CLOCKWISE    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_COUNTER_CLOCKWISE))
# define P142_SET_COUNTER_CLOCKWISE(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_COUNTER_CLOCKWISE, T))
# define P142_GET_USE_SPEED    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_USE_SPEED))
# define P142_SET_USE_SPEED(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_USE_SPEED, T))
# define P142_GET_OUTPUT_MODE    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_OUTPUT_MODE))
# define P142_SET_OUTPUT_MODE(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_OUTPUT_MODE, T))
# define P142_GET_UPDATE_DIFF_ONLY    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_DIFF_ONLY))
# define P142_SET_UPDATE_DIFF_ONLY(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_DIFF_ONLY, T))
# define P142_GET_POWER_MODE    (get2BitFromUL(P142_CONFIG_FLAGS, P142_CONFIG_POWER_MODE))
# define P142_SET_POWER_MODE(T) (set2BitToUL(P142_CONFIG_FLAGS, P142_CONFIG_POWER_MODE, T))
# define P142_GET_HYSTERESIS    (get2BitFromUL(P142_CONFIG_FLAGS, P142_CONFIG_HYSTERESIS))
# define P142_SET_HYSTERESIS(T) (set2BitToUL(P142_CONFIG_FLAGS, P142_CONFIG_HYSTERESIS, T))
# define P142_GET_SLOW_FILTER    (get2BitFromUL(P142_CONFIG_FLAGS, P142_CONFIG_SLOW_FILTER))
# define P142_SET_SLOW_FILTER(T) (set2BitToUL(P142_CONFIG_FLAGS, P142_CONFIG_SLOW_FILTER, T))
# define P142_GET_FAST_FILTER    (get3BitFromUL(P142_CONFIG_FLAGS, P142_CONFIG_FAST_FILTER))
# define P142_SET_FAST_FILTER(T) (set3BitToUL(P142_CONFIG_FLAGS, P142_CONFIG_FAST_FILTER, T))
# define P142_GET_WATCHDOG    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_WATCHDOG))
# define P142_SET_WATCHDOG(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_WATCHDOG, T))
# define P142_GET_ENABLE_LOG    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_ENABLE_LOG))
# define P142_SET_ENABLE_LOG(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_ENABLE_LOG, T))
# define P142_GET_CHECK_MAGNET    (bitRead(P142_CONFIG_FLAGS, P142_CONFIG_CHECK_MAGNET))
# define P142_SET_CHECK_MAGNET(T) (bitWrite(P142_CONFIG_FLAGS, P142_CONFIG_CHECK_MAGNET, T))

enum P142_output_options {
  P142_OUTPUT_ANGLE = 0,
  P142_OUTPUT_MAGNET,
  P142_OUTPUT_RPM,
  P142_OUTPUT_RAW_ANGLE,
  P142_OUTPUT_READ_ANGLE,
  P142_OUTPUT_AGC,
  P142_OUTPUT_HAS_MAGNET,

  // keep as last:
  P142_NR_OUTPUT_OPTIONS
};

const __FlashStringHelper* Plugin_142_valuename(uint8_t value_nr,
                                                bool    displayString);

struct P142_data_struct : public PluginTaskData_base {
public:

  P142_data_struct(struct EventStruct *event);

  P142_data_struct() = delete;
  virtual ~P142_data_struct();

  bool init(struct EventStruct *event);

  bool plugin_read(struct EventStruct *event);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return initialized;
  }

  float getOutputValue(struct EventStruct *event,
                       int                 idx,
                       float               currentValue,
                       bool              & changed);

private:

  bool setOutputValues(struct EventStruct *event);

  AS5600 *as5600 = nullptr;

  float    _angleOffset;
  uint16_t _startPosition;
  uint16_t _maxPosition;
  uint8_t  _slowFilter;
  uint8_t  _fastFilter;
  uint8_t  _hysteresis;
  uint8_t  _powerMode;

  bool _dataAvailable = false;

  uint8_t _address;

  bool    initialized = false;
  uint8_t speedCount  = 4; // At least 4 reads before value is valid

  float    _angle{};
  uint16_t _rawAngle{};
  uint16_t _readAngle{};
  uint16_t _magnitude{};
  uint16_t _speed{};
  uint16_t _agc{};
  bool     _hasMagnet = false;
};

#endif // ifdef USES_P142
#endif // ifndef PLUGINSTRUCTS_P142_DATA_STRUCT_H
