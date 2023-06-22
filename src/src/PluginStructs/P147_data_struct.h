#ifndef PLUGINSTRUCTS_P147_DATA_STRUCT_H
#define PLUGINSTRUCTS_P147_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P147

# ifndef P147_FEATURE_GASINDEXALGORITHM
#  define P147_FEATURE_GASINDEXALGORITHM    1 // Enabled by default
# endif // ifndef P147_FEATURE_GASINDEXALGORITHM

# if P147_FEATURE_GASINDEXALGORITHM
#  include <SensirionGasIndexAlgorithm.h>
#  include <VOCGasIndexAlgorithm.h>
#  include <NOxGasIndexAlgorithm.h>
# endif // if P147_FEATURE_GASINDEXALGORITHM

# include "../Helpers/CRC_functions.h"

# define P147_SENSOR_TYPE         PCONFIG(0)
# define P147_LOW_POWER_MEASURE   PCONFIG(1)

# define P147_TEMPERATURE_TASK    PCONFIG(4)
# define P147_TEMPERATURE_VALUE   PCONFIG(5)
# define P147_HUMIDITY_TASK       PCONFIG(6)
# define P147_HUMIDITY_VALUE      PCONFIG(7)

# define P147_FLAGS               PCONFIG_ULONG(0)

# define P147_FLAG_USE_COMPENSATION 0
# define P147_FLAG_RAW_DATA_ONLY    1

# define P147_GET_USE_COMPENSATION    bitRead(P147_FLAGS, P147_FLAG_USE_COMPENSATION)
# define P147_SET_USE_COMPENSATION(x) bitWrite(P147_FLAGS, P147_FLAG_USE_COMPENSATION, (x))
# define P147_GET_RAW_DATA_ONLY    bitRead(P147_FLAGS, P147_FLAG_RAW_DATA_ONLY)
# define P147_SET_RAW_DATA_ONLY(x) bitWrite(P147_FLAGS, P147_FLAG_RAW_DATA_ONLY, (x))

# define P147_SHORT_COUNTER       1   // Regular measurement 1x second
# define P147_LONG_COUNTER        10  // Low power measurement, 1x 10 seconds

# define P147_DELAY_REGULAR       30  // Milliseconds, regular measurement
# define P147_DELAY_REGULAR_SGP41 50  // Milliseconds, regular measurement for SGP41
# define P147_DELAY_LOW_POWER     140 // Allow the heater to heat up, 170 - 30 msec
# define P147_DELAY_MINIMAL       10  // Next step delay
# define P147_DELAY_SELFTEST      320 // Selftest delay

// Fixed address
# define P147_I2C_ADDRESS         0x59

// I2C command bytes for SGP4x, most are 16 bits, when split into 2 bytes they can be written using I2C_write_8_reg()

# define P147_CMD_SELF_TEST_A     0x28 // Will return 3 bytes of data, SGP40: fixed value 0xD4xxcc = OK, 0x4Bxxcc = Error
# define P147_CMD_SELF_TEST_B     0x0E // cc = crc8 checksum. SGP41: 0xxx00cc = OK 0xxx01cc..0xxx03cc = Error
# define P147_CMD_SGP40_READ_A    0x26 // SGP40: Will take 6 bytes of data, and return 3 bytes of data 0xnnnncc = 16 bits raw
# define P147_CMD_SGP40_READ_B    0x0F
# define P147_CMD_SGP41_READ_A    0x26 // SGP41: Will take 6 bytes of data, and return 6 bytes of data 0xnnnncc = 16 bits rawVOC
# define P147_CMD_SGP41_READ_B    0x19 // + 0xnnnncc = 16 bits rawNOx
# define P147_CMD_SGP41_COND_A    0x26 // SGP41: Conditioning: Takes 6 bytes of data, return 3 bytes of data 0xnnnncc = 16 bits rawVOC
# define P147_CMD_SGP41_COND_B    0x12
# define P147_CMD_HEATER_OFF_A    0x36 // No returned data
# define P147_CMD_HEATER_OFF_B    0x15
# define P147_CMD_READ_SERIALNR_A 0x36
# define P147_CMD_READ_SERIALNR_B 0x82 // Will return 9 bytes of data 0xnnnnccnnnnccnnnncc = 48 bits serialnumber


enum class P147_sensor_e : uint8_t {
  SGP40 = 0,
  SGP41 = 1,
};

enum class P147_state_e : uint8_t {
  Uninitialized  = 0,
  MeasureTest    = 1,
  MeasureStart   = 2,
  MeasureTrigger = 3,
  MeasureReading = 4,
  Ready          = 5,
};

struct P147_data_struct : public PluginTaskData_base {
public:

  P147_data_struct(struct EventStruct *event);

  P147_data_struct() = delete;
  virtual ~P147_data_struct();

  bool init(struct EventStruct *event);

  bool plugin_tasktimer_in(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return _initialized;
  }

private:

  # if P147_FEATURE_GASINDEXALGORITHM
  VOCGasIndexAlgorithm *vocGasIndexAlgorithm = nullptr;
  NOxGasIndexAlgorithm *noxGasIndexAlgorithm = nullptr;
  uint32_t              _vocIndex            = 0;
  uint32_t              _noxIndex            = 0;
  # endif // if P147_FEATURE_GASINDEXALGORITHM

  uint16_t readCheckedWord(bool& is_ok,
                           long  extraDelay = 5);
  bool     startSensorRead(uint16_t compensationRh,
                           uint16_t compensationT);

  P147_state_e _state = P147_state_e::Uninitialized;

  P147_sensor_e _sensorType            = P147_sensor_e::SGP40;
  int16_t       _temperatureValueIndex = -1;
  int16_t       _humidityValueIndex    = -1;
  uint8_t       _initialCounter        = P147_SHORT_COUNTER;
  uint8_t       _secondsCounter        = _initialCounter;
  uint8_t       _startupNOxCounter     = 10; // Only after 10 seconds NOx value is usable
  bool          _useCompensation       = false;
  bool          _initialized           = false;
  # if P147_FEATURE_GASINDEXALGORITHM
  uint16_t _skipCount = 0;
  bool     _rawOnly   = false;
  # endif // if P147_FEATURE_GASINDEXALGORITHM

  uint64_t _serial          = 0;
  uint16_t _rawVOC          = 0;
  uint16_t _rawNOx          = 0;
  uint8_t  _readLoop        = 0;
  uint8_t  _lastCommand     = 0;
  bool     _dataAvailable   = false;
  bool     _ignoreFirstRead = false;
};

#endif // ifdef USES_P147
#endif // ifndef PLUGINSTRUCTS_P147_DATA_STRUCT_H
