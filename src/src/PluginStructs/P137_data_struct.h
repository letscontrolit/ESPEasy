#ifndef PLUGINSTRUCTS_P137_DATA_STRUCT_H
#define PLUGINSTRUCTS_P137_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P137

# ifdef ESP32

#  include <I2C_AXP192.h>

#  define P137_DEBUG_LOG            // Enable for some (extra) logging

#  define P137_CONFIG_BASE        0 // Uses PCONFIG(0)..PCONFIG(3) to store the selection for 4 output values
#  define P137_SENSOR_TYPE_INDEX  (P137_CONFIG_BASE + VARS_PER_TASK)
#  define P137_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P137_SENSOR_TYPE_INDEX)))
#  define P137_CONFIG_DECIMALS    PCONFIG(P137_CONFIG_BASE + VARS_PER_TASK + 1)
#  define P137_CONFIG_PREDEFINED  PCONFIG(P137_CONFIG_BASE + VARS_PER_TASK + 2)
#  define P137_CURRENT_PREDEFINED PCONFIG_FLOAT(0)
#  define P137_CONFIG_DISABLEBITS PCONFIG(P137_CONFIG_BASE + VARS_PER_TASK + 3)

#  define P137_CONST_1_PERCENT    1    // Lowest used percentage, 0 = off
#  define P137_CONST_100_PERCENT  100  // Max percentage
#  define P137_CONST_MIN_LDO      1800 // Min. accepted LDO output voltage
#  define P137_CONST_MAX_LDO      3300 // Max. LDO output voltage
#  define P137_CONST_MIN_LDOIO    1800 // Min. accepted GPIO output voltage
#  define P137_CONST_MAX_LDOIO    3300 // Max. GPIO output voltage
#  define P137_CONST_MIN_DCDC     700  // Min. accepted DCDC output voltage
#  define P137_CONST_MAX_DCDC     3500 // Max. DCDC output voltage
#  define P137_CONST_MAX_DCDC2    2750 // Max. DCDC2 output voltage

// to break comment indenting
#  define P137_dummy

#  define P137_REG_DCDC2_LDO2     PCONFIG_ULONG(0)                                                                // DCDC2 & LDO2
#  define P137_REG_DCDC3_LDO3     PCONFIG_ULONG(1)                                                                // DCDC3 & LDO3
#  define P137_REG_LDOIO          PCONFIG_ULONG(2)                                                                // GPIO
#  define P137_CONFIG_FLAGS       PCONFIG_ULONG(3)                                                                // Other flags

#  define P137_GET_CONFIG_LDO2    P137_settingToValue(P137_REG_DCDC2_LDO2 & 0xFFFF, P137_CONST_MAX_LDO)           // LDO2
#  define P137_GET_CONFIG_LDO3    P137_settingToValue(P137_REG_DCDC3_LDO3 & 0xFFFF, P137_CONST_MAX_LDO)           // LDO3
#  define P137_GET_CONFIG_DCDC2   P137_settingToValue((P137_REG_DCDC2_LDO2 >> 16) & 0xFFFF, P137_CONST_MAX_DCDC2) // DCDC2
#  define P137_GET_CONFIG_DCDC3   P137_settingToValue((P137_REG_DCDC3_LDO3 >> 16) & 0xFFFF, P137_CONST_MAX_DCDC)  // DCDC3
#  define P137_GET_CONFIG_LDOIO   P137_settingToValue(P137_REG_LDOIO & 0xFFFF, P137_CONST_MAX_LDOIO)              // GPIO

#  define P137_GET_GPIO_FLAGS(i) (static_cast<int8_t>(get3BitFromUL(P137_CONFIG_FLAGS, i * 3)))
#  define P137_SET_GPIO_FLAGS(i, v) set3BitToUL(P137_CONFIG_FLAGS, i * 3, v)
#  define P137_GET_FLAG_GPIO0     P137_GET_GPIO_FLAGS(0)
#  define P137_GET_FLAG_GPIO1     P137_GET_GPIO_FLAGS(1)
#  define P137_GET_FLAG_GPIO2     P137_GET_GPIO_FLAGS(2)
#  define P137_GET_FLAG_GPIO3     P137_GET_GPIO_FLAGS(3)
#  define P137_GET_FLAG_GPIO4     P137_GET_GPIO_FLAGS(4)

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
  LDOIO                   = 0x0D,
  DCDC2                   = 0x12,
  DCDC3                   = 0x13,
};

enum class P137_GPIOBootState_e: uint8_t { // Will be applied by subtracting 1 !!
  Default     = 0u,
  Output_low  = 1u,
  Output_high = 2u,
  Input       = 3u,
  PWM         = 4u,
};

enum class P137_PredefinedDevices_e : uint8_t {
  Unselected     = 0u,
  M5Stack_StickC = 1u,
  M5Stack_Core2  = 2u,
  LilyGO_TBeam   = 3u,
  UserDefined    = 99u // Keep as last
};

int16_t                    P137_settingToValue(uint16_t data,
                                               uint16_t threshold);
uint16_t                   P137_valueToSetting(int      data,
                                               uint16_t threshold);
void                       P137_CheckPredefinedParameters(struct EventStruct *event);

const __FlashStringHelper* toString(const P137_valueOptions_e value,
                                    bool                      displayString = true);
const __FlashStringHelper* toString(const P137_GPIOBootState_e value);
const __FlashStringHelper* toString(const P137_PredefinedDevices_e device);

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

  // Map range 0..100%
  uint16_t ldo2_range[2]  = { P137_CONST_MIN_LDO, P137_CONST_MAX_LDO };
  uint16_t ldo3_range[2]  = { P137_CONST_MIN_LDO, P137_CONST_MAX_LDO };
  uint16_t ldoio_range[2] = { P137_CONST_MIN_LDOIO, P137_CONST_MAX_LDOIO };
  uint16_t dcdc2_range[2] = { P137_CONST_MIN_DCDC, P137_CONST_MAX_DCDC2 };
  uint16_t dcdc3_range[2] = { P137_CONST_MIN_DCDC, P137_CONST_MAX_DCDC };

  int ldo2_value  = 0; // Keep set values, so we can return them as Get Config values
  int ldo3_value  = 0;
  int ldoio_value = 0;
  int dcdc2_value = 0;
  int dcdc3_value = 0;
};

# endif // ifdef ESP32
#endif // ifdef USES_P137
#endif // ifndef PLUGINSTRUCTS_P137_DATA_STRUCT_H
