#ifndef PLUGINSTRUCTS_P139_DATA_STRUCT_H
#define PLUGINSTRUCTS_P139_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P139

# ifdef ESP32

#  include <AXP2101.h>

#  define P139_DEBUG_LOG            // Enable for some (extra) logging

#  define P139_CONFIG_BASE        0 // Uses PCONFIG(0)..PCONFIG(3) to store the selection for 4 output values
#  define P139_SENSOR_TYPE_INDEX  (P139_CONFIG_BASE + VARS_PER_TASK)
#  define P139_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P139_SENSOR_TYPE_INDEX)))
#  define P139_CONFIG_DECIMALS    PCONFIG(P139_CONFIG_BASE + VARS_PER_TASK + 1)
#  define P139_CONFIG_PREDEFINED  PCONFIG(P139_CONFIG_BASE + VARS_PER_TASK + 2)
#  define P139_CURRENT_PREDEFINED PCONFIG_FLOAT(0)

// #  define P139_CONFIG_DISABLEBITS PCONFIG(P139_CONFIG_BASE + VARS_PER_TASK + 3)

#  define P139_CONST_1_PERCENT    1    // Lowest used percentage, 0 = off
#  define P139_CONST_100_PERCENT  100  // Max percentage
#  define P139_CONST_MIN_LDO      500  // Min. output voltage
#  define P139_CONST_MAX_LDO      3700 // Max. output voltage
// #  define P139_CONST_MIN_LDO      1800 // Min. accepted LDO output voltage
// #  define P139_CONST_MIN_LDOIO    1800 // Min. accepted GPIO output voltage
// #  define P139_CONST_MAX_LDOIO    3300 // Max. GPIO output voltage
// #  define P139_CONST_MIN_DCDC     700  // Min. accepted DCDC output voltage
// #  define P139_CONST_MAX_DCDC     3500 // Max. DCDC output voltage
// #  define P139_CONST_MAX_DCDC2    2750 // Max. DCDC2 output voltage

// to break comment indenting
#  define P139_dummy

// #  define P139_REG_DCDC2_LDO2     PCONFIG_ULONG(0)                                                                // DCDC2 & LDO2
// #  define P139_REG_DCDC3_LDO3     PCONFIG_ULONG(1)                                                                // DCDC3 & LDO3
// #  define P139_REG_LDOIO          PCONFIG_ULONG(2)                                                                // GPIO
// #  define P139_CONFIG_FLAGS       PCONFIG_ULONG(3)                                                                // Other flags

// #  define P139_GET_CONFIG_LDO2    P139_settingToValue(P139_REG_DCDC2_LDO2 & 0xFFFF, P139_CONST_MAX_LDO)           // LDO2
// #  define P139_GET_CONFIG_LDO3    P139_settingToValue(P139_REG_DCDC3_LDO3 & 0xFFFF, P139_CONST_MAX_LDO)           // LDO3
// #  define P139_GET_CONFIG_DCDC2   P139_settingToValue((P139_REG_DCDC2_LDO2 >> 16) & 0xFFFF, P139_CONST_MAX_DCDC2) // DCDC2
// #  define P139_GET_CONFIG_DCDC3   P139_settingToValue((P139_REG_DCDC3_LDO3 >> 16) & 0xFFFF, P139_CONST_MAX_DCDC)  // DCDC3
// #  define P139_GET_CONFIG_LDOIO   P139_settingToValue(P139_REG_LDOIO & 0xFFFF, P139_CONST_MAX_LDOIO)              // GPIO

// #  define P139_GET_GPIO_FLAGS(i) (static_cast<int8_t>(get3BitFromUL(P139_CONFIG_FLAGS, (i) * 3)))
// #  define P139_SET_GPIO_FLAGS(i, v) set3BitToUL(P139_CONFIG_FLAGS, (i) * 3, (v))
// #  define P139_GET_FLAG_GPIO0     P139_GET_GPIO_FLAGS(0)
// #  define P139_GET_FLAG_GPIO1     P139_GET_GPIO_FLAGS(1)
// #  define P139_GET_FLAG_GPIO2     P139_GET_GPIO_FLAGS(2)
// #  define P139_GET_FLAG_GPIO3     P139_GET_GPIO_FLAGS(3)
// #  define P139_GET_FLAG_GPIO4     P139_GET_GPIO_FLAGS(4)

enum class P139_valueOptions_e : uint8_t {
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
  LDOIO                   = 0x0D,
  DCDC2                   = 0x12,
  DCDC3                   = 0x13,
};

enum class P139_GPIOBootState_e: uint8_t { // Will be applied by subtracting 1 !!
  Default     = 0u,
  Output_low  = 1u,
  Output_high = 2u,
  Input       = 3u,
  PWM         = 4u,
};

// int16_t                    P139_settingToValue(uint16_t data,
//                                                uint16_t threshold);
// uint16_t                   P139_valueToSetting(int      data,
//                                                uint16_t threshold);
void                       P139_CheckPredefinedParameters(struct EventStruct *event);

const __FlashStringHelper* toString(const P139_valueOptions_e value,
                                    bool                      displayString = true);
const __FlashStringHelper* toString(const P139_GPIOBootState_e value);


struct P139_data_struct : public PluginTaskData_base {
public:

  P139_data_struct(struct EventStruct *event);
  P139_data_struct() = delete;
  ~P139_data_struct();

  bool   plugin_read(struct EventStruct *event);
  bool   plugin_write(struct EventStruct *event,
                      const String      & string);
  bool   plugin_get_config_value(struct EventStruct *event,
                                 String            & string);
  bool   plugin_ten_per_second(struct EventStruct *event);
  bool   plugin_fifty_per_second(struct EventStruct *event);
  String loadSettings(struct EventStruct *event);
  String saveSettings(struct EventStruct *event);
  void   outputSettings(struct EventStruct *event);
  bool   applySettings(AXP2101_device_model_e device);

  AXP2101_settings _settings;

private:

  AXP2101 *axp2101 = nullptr;

  bool isInitialized() {
    return nullptr != axp2101;
  }

  float read_value(AXP2101_registers_e value);

  // *INDENT-OFF*
  // Map range 0..100%
  uint16_t _ranges[AXP2101_settings_count][2] = {
    { AXP2101_DCDC1_MIN,   AXP2101_DCDC1_MAX   },
    { AXP2101_DCDC2_MIN,   AXP2101_DCDC2_MAX   },
    { AXP2101_DCDC3_MIN,   AXP2101_DCDC3_MAX   },
    { AXP2101_DCDC4_MIN,   AXP2101_DCDC4_MAX   },
    { AXP2101_DCDC5_MIN,   AXP2101_DCDC5_MAX   },
    { AXP2101_ALDO1_MIN,   AXP2101_ALDO1_MAX   },
    { AXP2101_ALDO2_MIN,   AXP2101_ALDO2_MAX   },
    { AXP2101_ALDO3_MIN,   AXP2101_ALDO3_MAX   },
    { AXP2101_ALDO4_MIN,   AXP2101_ALDO4_MAX   },
    { AXP2101_BLDO1_MIN,   AXP2101_BLDO1_MAX   },
    { AXP2101_BLDO2_MIN,   AXP2101_BLDO2_MAX   },
    { AXP2101_DLDO1_MIN,   AXP2101_DLDO1_MAX   },
    { AXP2101_DLDO2_MIN,   AXP2101_DLDO2_MAX   },
    { AXP2101_CPUSLDO_MIN, AXP2101_CPUSLDO_MAX },
  };

 // *INDENT-ON*

  bool _settingsLoaded = false;
};

# endif // ifdef ESP32
#endif // ifdef USES_P139
#endif // ifndef PLUGINSTRUCTS_P139_DATA_STRUCT_H
