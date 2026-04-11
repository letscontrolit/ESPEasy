#ifndef PLUGINSTRUCTS_P150_DATA_STRUCT_H
#define PLUGINSTRUCTS_P150_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P150

# define P150_I2C_ADDRESS           PCONFIG(0)
# define P150_TEMPERATURE_OFFSET    PCONFIG(1)
# define P150_CONFIG                PCONFIG_ULONG(0) // Sensor config
# define P150_OPTIONS               PCONFIG_ULONG(1) // Plugin options

# ifdef LIMIT_BUILD_SIZE
#  define P150_USE_EXTRA_LOG         0               // Enable(1)/disable(0) low-level logging
# else // ifdef LIMIT_BUILD_SIZE
#  define P150_USE_EXTRA_LOG         1               // Enable(1)/disable(0) low-level logging
# endif // ifdef LIMIT_BUILD_SIZE

# define P150_CONFIG_RESET_MASK   0b1111000000011111 // Reset the bits we need to overwrite

// Sensor configuration:
# define P150_CONF_CONVERSION_MODE        10         // Conversion mode 00/10 = Continuous, 01 = Shutdown, 11 = One Shot
# define P150_CONVERSION_CONTINUOUS       0b00
# define P150_CONVERSION_SHUTDOWN         0b01       // Not available in settings
# define P150_CONVERSION_ONE_SHOT         0b11

# define P150_CONF_CYCLE_BITS             7          // 3 bits: Cycle mode 000 = 15.5 msec, 001 = 125 msec, 010 = 250 msec,
                                                     // 011 = 500 msec, 100 = 1 sec, 101 = 4 sec, 110 = 8 sec, 111 = 16 sec
# define P150_CYCLE_15_5_MSEC             0b000
# define P150_CYCLE_125_MSEC              0b001
# define P150_CYCLE_250_MSEC              0b010
# define P150_CYCLE_500_MSEC              0b011
# define P150_CYCLE_1_SEC                 0b100
# define P150_CYCLE_4_SEC                 0b101
# define P150_CYCLE_8_SEC                 0b110
# define P150_CYCLE_16_SEC                0b111

# define P150_CONF_AVERAGING              5 // 2 bits: Averaging mode 00 = None, 01 = 8 samples, 10 = 32 samples 11 = 64 samples
# define P150_AVERAGING_NONE              0b00
# define P150_AVERAGING_8_SAMPLES         0b01
# define P150_AVERAGING_32_SAMPLES        0b10
# define P150_AVERAGING_64_SAMPLES        0b11

# define P150_GET_CONF_CONVERSION_MODE    get2BitFromUL(P150_CONFIG, P150_CONF_CONVERSION_MODE)
# define P150_SET_CONF_CONVERSION_MODE(x) set2BitToUL(P150_CONFIG,               \
                                                      P150_CONF_CONVERSION_MODE, \
                                                      x ? P150_CONVERSION_ONE_SHOT : P150_CONVERSION_CONTINUOUS)
# define P150_GET_CONF_CYCLE_BITS    get3BitFromUL(P150_CONFIG, P150_CONF_CYCLE_BITS)
# define P150_SET_CONF_CYCLE_BITS(x) set3BitToUL(P150_CONFIG, P150_CONF_CYCLE_BITS, x)
# define P150_GET_CONF_AVERAGING    get2BitFromUL(P150_CONFIG, P150_CONF_AVERAGING)
# define P150_SET_CONF_AVERAGING(x) set2BitToUL(P150_CONFIG, P150_CONF_AVERAGING, x)

// Plugin options:
# define P150_OPT_ENABLE_RAW              0 // Use Raw value, default on
# define P150_OPT_ENABLE_LOG              1 // Log measured values, default on
# if P150_USE_EXTRA_LOG
#  define P150_OPT_EXTRA_LOG              2 // Log low-level measured values, default off
# endif // if P150_USE_EXTRA_LOG

# define P150_GET_OPT_ENABLE_RAW    bitRead(P150_OPTIONS, P150_OPT_ENABLE_RAW)
# define P150_SET_OPT_ENABLE_RAW(x) bitWrite(P150_OPTIONS, P150_OPT_ENABLE_RAW, x)
# define P150_GET_OPT_ENABLE_LOG    bitRead(P150_OPTIONS, P150_OPT_ENABLE_LOG)
# define P150_SET_OPT_ENABLE_LOG(x) bitWrite(P150_OPTIONS, P150_OPT_ENABLE_LOG, x)
# if P150_USE_EXTRA_LOG
#  define P150_GET_OPT_EXTRA_LOG    bitRead(P150_OPTIONS, P150_OPT_EXTRA_LOG)
#  define P150_SET_OPT_EXTRA_LOG(x) bitWrite(P150_OPTIONS, P150_OPT_EXTRA_LOG, x)
# endif // if P150_USE_EXTRA_LOG

# define TMP117_RESOLUTION          0.0078125f // Resolution of the device, see specifications
# define TMP117_DEVICE_ID_VALUE     0x0117     // Value found in the device ID register on reset

enum TMP117_register {
  TMP117_TEMP_RESULT   = 0x00,
  TMP117_CONFIGURATION = 0x01,

  // TMP117_T_HIGH_LIMIT  = 0x02, // Unused
  // TMP117_T_LOW_LIMIT   = 0x03,
  // TMP117_EEPROM_UL     = 0x04,
  // TMP117_EEPROM1       = 0x05,
  // TMP117_EEPROM2       = 0x06,
  TMP117_TEMP_OFFSET = 0x07,

  // TMP117_EEPROM3       = 0x08,
  TMP117_DEVICE_ID = 0x0F,
};

struct P150_data_struct : public PluginTaskData_base {
  P150_data_struct() = delete;
  P150_data_struct(struct EventStruct *event);

  virtual ~P150_data_struct() = default;

  bool init();
  void setConfig();
  bool plugin_read(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);

private:

  float readTemp();
  void  setTemperatureOffset(float offset);
  bool  dataReady();

  uint16_t _config            = 0;
  float    _temperatureOffset = 0.0f;
  int8_t   _deviceAddress     = -1;
  bool     _rawEnabled        = false;
  bool     _logEnabled        = false;
  # if P150_USE_EXTRA_LOG
  bool _extraLog = false;
  # endif // if P150_USE_EXTRA_LOG

  int16_t _digitalTempC = 0;
  float   _finalTempC   = 0.0f;
  bool    _readValid    = false;
};

#endif // ifdef USES_P150
#endif // ifndef PLUGINSTRUCTS_P150_DATA_STRUCT_H
