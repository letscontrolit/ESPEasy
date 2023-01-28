#ifndef PLUGINSTRUCTS_P138_DATA_STRUCT_H
#define PLUGINSTRUCTS_P138_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P138

# include <ip5306.hpp>

# define P138_DEBUG_LOG             // Enable for some (extra) logging

# define P138_CONFIG_BASE         0 // Uses PCONFIG(0)..PCONFIG(3) to store the selection for 4 output values
# define P138_SENSOR_TYPE_INDEX   (P138_CONFIG_BASE + VARS_PER_TASK)
# define P138_NR_OUTPUT_VALUES    getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P138_SENSOR_TYPE_INDEX)))
# define P138_CONFIG_DECIMALS     PCONFIG(P138_CONFIG_BASE + VARS_PER_TASK + 1)

# define P138_CONFIG_FLAGS        PCONFIG_ULONG(0)
# define P138_FLAG_POWERCHANGE    0 // Flag 0: Send event on PowerChange event

enum class P138_valueOptions_e : uint8_t {
  None               = 0x00,
  BatteryCurrent     = 0x01,
  ChargeUnderVoltage = 0x02,
  StopVoltage        = 0x03,
  InCurrent          = 0x04,
  ChargeLevel        = 0x05,
  PowerSource        = 0x06,
};

const __FlashStringHelper* toString(const P138_valueOptions_e value,
                                    bool                      displayString = true);

struct P138_data_struct : public PluginTaskData_base {
public:

  P138_data_struct(struct EventStruct *event);
  P138_data_struct() = delete;
  ~P138_data_struct();

  bool plugin_read(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

private:

  arduino::ip5306 *_ip5306 = nullptr;

  bool isInitialized() {
    return nullptr != _ip5306;
  }

  float read_value(P138_valueOptions_e value);
  int8_t _lastPowerSource = -1;
};

#endif // ifdef USES_P138
#endif // ifndef PLUGINSTRUCTS_P138_DATA_STRUCT_H
