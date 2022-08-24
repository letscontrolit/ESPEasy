#ifndef PLUGINSTRUCTS_P137_DATA_STRUCT_H
#define PLUGINSTRUCTS_P137_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P137

# include <I2C_AXP192.h>

# define P137_DEBUG_LOG            // Enable for some (extra) logging

# define P137_CONFIG_BASE        0 // Uses PCONFIG(0)..PCONFIG(3) to store the selection for 4 output values
# define P137_SENSOR_TYPE_INDEX  (P137_CONFIG_BASE + VARS_PER_TASK)
# define P137_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P137_SENSOR_TYPE_INDEX)))
# define P137_CONFIG_DECIMALS    PCONFIG(P137_CONFIG_BASE + VARS_PER_TASK + 1)

# define P137_CONFIG_LDO2        PCONFIG_LONG(0) // LDO2
# define P137_CONFIG_LDO3        PCONFIG_LONG(1) // LDO3
# define P137_CONFIG_GPIO0       PCONFIG_LONG(2) // GPIO0

# define P137_CONST_1_PERCENT    1               // Lowest used percentage, 0 = off
# define P137_CONST_100_PERCENT  100             // Max percentage
# define P137_CONST_MIN_LDO      1800            // Min. accepted LDO output voltage
# define P137_CONST_MAX_LDO      3300            // Max. safe LDO output voltage

enum class P137_valueOptions_e : uint8_t {
  None                    = 0x00,
  BatteryVoltage          = 0x01,
  BatteryDischargeCurrent = 0x02,
  BatteryChargeCurrent    = 0x03,
  BatteryPower            = 0x04,
  AcinVoltage             = 0x05,
  AcinCurrent             = 0x06,
  VbusVoltage             = 0x07,
  VbusCurrent             = 0x08,
  InternalTemperature     = 0x09,
  ApsVoltage              = 0x0A,
  LDO2                    = 0x0B,
  LDO3                    = 0x0C,
  GPIO0                   = 0x0D,
  OptionCount             = 0x0E
};

const __FlashStringHelper* toString(const P137_valueOptions_e value,
                                    bool                      displayString = true);

struct P137_data_struct : public PluginTaskData_base {
public:

  P137_data_struct(struct EventStruct *event);
  P137_data_struct() = delete;
  ~P137_data_struct();

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

private:

  I2C_AXP192 *axp192 = nullptr;

  bool isInitialized() {
    return nullptr != axp192;
  }

  float read_value(P137_valueOptions_e value);

  // Map range 0..100%, 0 turns off, TFT backlight will only light up from ~2500 mV
  uint16_t ldo2_range[2]  = { P137_CONST_MIN_LDO, P137_CONST_MAX_LDO };
  uint16_t ldo3_range[2]  = { P137_CONST_MIN_LDO, P137_CONST_MAX_LDO };
  uint16_t gpio0_range[2] = { P137_CONST_MIN_LDO, P137_CONST_MAX_LDO };
  int      ldo2_value; // Keep set values, so we can return them as Get Config values
  int      ldo3_value;
  int      gpio0_value;
};

#endif // ifdef USES_P137
#endif // ifndef PLUGINSTRUCTS_P137_DATA_STRUCT_H
