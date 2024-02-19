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

#  define P139_FLAGS              PCONFIG_ULONG(0)

#  define P139_FLAG_GENERATE_EVENTS   0

// #  define P139_FLAG_RAW_DATA_ONLY     1

#  define P139_GET_GENERATE_EVENTS    bitRead(P139_FLAGS, P139_FLAG_GENERATE_EVENTS)
#  define P139_SET_GENERATE_EVENTS(x) bitWrite(P139_FLAGS, P139_FLAG_GENERATE_EVENTS, (x))

// #  define P139_GET_RAW_DATA_ONLY    bitRead(P139_FLAGS, P139_FLAG_RAW_DATA_ONLY)
// #  define P139_SET_RAW_DATA_ONLY(x) bitWrite(P139_FLAGS, P139_FLAG_RAW_DATA_ONLY, (x))

#  define P139_CONST_1_PERCENT    1    // Lowest used percentage, 0 = off
#  define P139_CONST_100_PERCENT  100  // Max percentage
#  define P139_CONST_MIN_LDO      500  // Min. output voltage
#  define P139_CONST_MAX_LDO      3700 // Max. output voltage


struct P139_data_struct : public PluginTaskData_base {
public:

  P139_data_struct(struct EventStruct *event);
  P139_data_struct();
  ~P139_data_struct();

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

  // bool   plugin_ten_per_second(struct EventStruct *event);
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

  AXP2101_chargingState_e _chargingState = AXP2101_chargingState_e::Standby;
};

# endif // ifdef ESP32
#endif // ifdef USES_P139
#endif // ifndef PLUGINSTRUCTS_P139_DATA_STRUCT_H
