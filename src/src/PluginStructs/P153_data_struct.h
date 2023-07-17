#ifndef PLUGINSTRUCTS_P153_DATA_STRUCT_H
#define PLUGINSTRUCTS_P153_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P153

# define P153_I2C_ADDRESS             PCONFIG(0)
# define P153_STARTUP_CONFIGURATION   PCONFIG(1)
# define P153_INTERVAL_LOOPS          PCONFIG(2)
# define P153_NORMAL_CONFIGURATION    PCONFIG(3)

# define P153_TEMPERATURE_OFFSET      PCONFIG_FLOAT(0)

# define P153_SHT4X_READ_SERIAL       0x89
# define P153_SHT4X_RESET             0x94

# define P153_MAX_ERRORCOUNT          5

# define P153_DELAY_LOW_RESOLUTION    2
# define P153_DELAY_MEDIUM_RESOLUTION 5
# define P153_DELAY_HIGH_RESOLUTION   10
# define P153_DELAY_100MS_HEATER      110
# define P153_DELAY_1S_HEATER         1100

enum class P153_read_mode_e : uint8_t {
  Idle = 0,
  Reading,
};

enum class P153_configuration_e : uint8_t {
  LowResolution               = 0xE0, // Command code
  MediumResolution            = 0xF6,
  HighResolution              = 0xFD,
  HighResolution200mW1000msec = 0x39,
  HighResolution200mW100msec  = 0x32,
  HighResolution110mW1000msec = 0x2F,
  HighResolution110mW100msec  = 0x24,
  HighResolution20mW1000msec  = 0x1E,
  HighResolution20mW100msec   = 0x15,
};

struct P153_data_struct : public PluginTaskData_base {
public:

  P153_data_struct(uint8_t              address,
                   float                tempOffset,
                   P153_configuration_e startupConfiguration,
                   P153_configuration_e normalConfiguration,
                   uint16_t             intervalLoops);

  P153_data_struct() = delete;
  virtual ~P153_data_struct() {}

  bool init();

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return initialized;
  }

private:

  bool CRC8(uint8_t MSB,
            uint8_t LSB,
            uint8_t CRC);

  uint8_t              _sensorType;
  uint8_t              _address;
  float                _tempOffset;
  P153_configuration_e _startupConfiguration;
  P153_configuration_e _normalConfiguration;
  uint16_t             _intervalLoops;

  uint32_t         serialNumber     = 0;
  float            temperature      = 0.0f;
  float            humidity         = 0.0f;
  bool             initialized      = false;
  P153_read_mode_e readMode         = P153_read_mode_e::Idle;
  uint32_t         measurementStart = 0;
  uint16_t         errorCount       = 0;
};

#endif // ifdef USES_P153
#endif // ifndef PLUGINSTRUCTS_P153_DATA_STRUCT_H
