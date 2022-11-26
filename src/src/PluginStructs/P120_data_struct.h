#ifndef PLUGINSTRUCTS_P120_DATA_STRUCT_H
#define PLUGINSTRUCTS_P120_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#if defined(USES_P120) || defined(USES_P125)

# include "../Globals/EventQueue.h"

# include <vector>

# include <Wire.h> // Needed for I2C
// https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
# include <SparkFun_ADXL345.h>

# define PLUGIN_120_DEBUG false // Set to true to enable debug out log

// Make accessing specific parameters more readable in the code
// # define P120_I2C_OR_SPI      PCONFIG(0)
# define P120_I2C_ADDR                    PCONFIG(1) // Kept (1) from template
# define P120_CS_PIN                      PIN(0)
# define P120_AVERAGE_BUFFER              PCONFIG(2)
# define P120_FREQUENCY                   PCONFIG(3)
# define P120_FREQUENCY_10                0 // 10x per second
# define P120_FREQUENCY_50                1 // 50x per second
// First set of configuration flags
# define P120_CONFIG_FLAGS1               PCONFIG_LONG(0)
# define P120_FLAGS1_RANGE                0 // Range setting, size 2 bits
# define P120_RANGE_2G                      0
# define P120_RANGE_4G                      1
# define P120_RANGE_8G                      2
# define P120_RANGE_16G                     3
# define P120_FLAGS1_ACTIVITY_X           2    // X-axis activity setting
# define P120_FLAGS1_ACTIVITY_Y           3    // Y-axis activity setting
# define P120_FLAGS1_ACTIVITY_Z           4    // Z-axis activity setting
# define P120_FLAGS1_TAP_X                5    // X-axis tap detection
# define P120_FLAGS1_TAP_Y                6    // Y-axis tap detection
# define P120_FLAGS1_TAP_Z                7    // Z-axis tap detection
# define P120_FLAGS1_DBL_TAP              8    // Double-tap detection
# define P120_FLAGS1_FREE_FALL            9    // Free-fall detection
# define P120_FLAGS1_SEND_ACTIVITY        10   // Send (in)activity events
# define P120_FLAGS1_LOG_ACTIVITY         11   // Log activity at INFO level
# define P120_FLAGS1_EVENT_RAW_VALUES     12   // Events use direct raw sensorvalues
# define P120_FLAGS1_ANGLE_IN_RAD         13   // Whether to output angles in radians or degrees
# define P120_FLAGS1_ACTIVITY_TRESHOLD    16   // Activity treshold, 8 bits
# define P120_DEFAULT_ACTIVITY_TRESHOLD     75 // Default treshold: 75 * 62.5 mg = 4.6875 g
# define P120_FLAGS1_INACTIVITY_TRESHOLD  24   // Inactivity treshold, 8 bits
# define P120_DEFAULT_INACTIVITY_TRESHOLD   75 // Default treshold: 75 * 62.5 mg = 4.6875 g

// Second set of configuration flags, Tap settings
# define P120_CONFIG_FLAGS2               PCONFIG_LONG(1)
# define P120_FLAGS2_TAP_TRESHOLD         0
# define P120_DEFAULT_TAP_TRESHOLD          50  // Default treshold: 50 * 62.5 mg = 3.125 g
# define P120_FLAGS2_TAP_DURATION         8
# define P120_DEFAULT_TAP_DURATION          15  // Default duration: 15 * 625 us = .09375 sec
# define P120_FLAGS2_DBL_TAP_LATENCY      16
# define P120_DEFAULT_DBL_TAP_LATENCY       80  // Default latency: 80 * 1.25 ms = .01 sec
# define P120_FLAGS2_DBL_TAP_WINDOW       24
# define P120_DEFAULT_DBL_TAP_WINDOW        200 // Default window: 200 * 1.25 ms = .25 sec

// Third set of configuration flags, Free-fall settings
# define P120_CONFIG_FLAGS3               PCONFIG_LONG(2)
# define P120_FLAGS3_FREEFALL_TRESHOLD    0
# define P120_DEFAULT_FREEFALL_TRESHOLD     7  // Default treshold: 7 * 62.5 mg = 0.4375 g
# define P120_FLAGS3_FREEFALL_DURATION    8
# define P120_DEFAULT_FREEFALL_DURATION     30 // Default duration: 30 * 5 ms = .15 sec

// Fourth set of configuration flags, Axis Offset settings
# define P120_CONFIG_FLAGS4               PCONFIG_LONG(3)
# define P120_FLAGS4_OFFSET_X             0
# define P120_FLAGS4_OFFSET_Y             8
# define P120_FLAGS4_OFFSET_Z             16

# define P120_QUERY1_CONFIG_POS  4
# define P120_SENSOR_TYPE_INDEX  0
# define P120_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P120_SENSOR_TYPE_INDEX)))
# define P120_NR_OUTPUT_OPTIONS  9



struct P120_data_struct : public PluginTaskData_base {
public:
  // Do not change nrs as these are stored
  enum class valueType {
    Empty = 0,
    X_RAW = 1,
    Y_RAW = 2,
    Z_RAW = 3,
    X_g   = 4,
    Y_g   = 5,
    Z_g   = 6,
    Pitch = 7,
    Roll  = 8,

    NR_ValueTypes // keep as last
  };

  static bool isXYZ(valueType vtype);

  P120_data_struct(uint8_t aSize);
  P120_data_struct() = delete;
  virtual ~P120_data_struct();

  void setI2Caddress(uint8_t i2c_addr);
  void setSPI_CSpin(int     cs_pin);

  bool read_sensor(struct EventStruct *event);

  bool read_data(struct EventStruct *event) const;

private:
  bool get_XYZ(float& X,
               float& Y,
               float& Z) const;


public:

  bool getPitchRoll(struct EventStruct *event,
                    float             & pitch,
                    float             & roll) const;

  bool initialized() const {
    return adxl345 != nullptr;
  }

  static bool plugin_webform_loadOutputSelector(struct EventStruct *event);
  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_set_defaults(struct EventStruct *event);

  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string) const;

  static void plugin_get_device_value_names(struct EventStruct *event);

  static const __FlashStringHelper * valuename(uint8_t value_nr, bool displayString);

private:

  bool init_sensor(struct EventStruct *event);
  void sensor_check_interrupt(struct EventStruct *event);
  void appendPayloadXYZ(struct EventStruct *event,
                        String            & payload,
                        uint8_t             useX,
                        uint8_t             useY,
                        uint8_t             useZ);
  void send_task_event(struct EventStruct *event,
                       String            & eventPayload);

  ADXL345 *adxl345 = nullptr;

  uint8_t _i2c_addr = 0;
  int     _cs_pin   = -1;
  uint8_t _aSize    = 0;

  std::vector<int>_XA;
  std::vector<int>_YA;
  std::vector<int>_ZA;
  uint8_t         _aUsed = 0;
  uint8_t         _aMax  = 0;

  int _x = 0, _y = 0, _z = 0; // Last measured values
  mutable float last_scale_factor_g = 0.0f;

  bool activityTriggered   = false;
  bool inactivityTriggered = false;
  bool i2c_mode            = false;
};

#endif // if defined(USES_P120) || defined(USES_P125)
#endif // ifndef PLUGINSTRUCTS_P120_DATA_STRUCT_H
