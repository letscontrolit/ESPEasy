#ifndef PLUGINSTRUCTS_P159_DATA_STRUCT_H
#define PLUGINSTRUCTS_P159_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P159
# define PLUGIN_159_DEBUG  false // set to true for extra log info

# include <ESPeasySerial.h>
# include <ld2410.h>

// place sensor type selector right after the output value settings
# define P159_QUERY1_CONFIG_POS   0
# define P159_SENSOR_TYPE_INDEX   (P159_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P159_NR_OUTPUT_VALUES    getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P159_SENSOR_TYPE_INDEX)))

# define P159_CONFIG_FLAGS        PCONFIG_ULONG(0)

# define P159_FLAG_ENGINEERING_MODE       0
# define P159_FLAG_UPDATE_DIFF_ONLY       1

# define P159_GET_ENGINEERING_MODE    bitRead(P159_CONFIG_FLAGS, P159_FLAG_ENGINEERING_MODE)
# define P159_SET_ENGINEERING_MODE(x) bitWrite(P159_CONFIG_FLAGS, P159_FLAG_ENGINEERING_MODE, x)
# define P159_GET_UPDATE_DIFF_ONLY    bitRead(P159_CONFIG_FLAGS, P159_FLAG_UPDATE_DIFF_ONLY)
# define P159_SET_UPDATE_DIFF_ONLY(x) bitWrite(P159_CONFIG_FLAGS, P159_FLAG_UPDATE_DIFF_ONLY, x)

# define P159_OUTPUT_PRESENCE             0
# define P159_OUTPUT_STATIONARY_PRESENCE  1
# define P159_OUTPUT_MOVING_PRESENCE      2
# define P159_OUTPUT_DISTANCE             3
# define P159_OUTPUT_STATIONARY_DISTANCE  4
# define P159_OUTPUT_MOVING_DISTANCE      5
# define P159_OUTPUT_STATIONARY_ENERGY    6
# define P159_OUTPUT_MOVING_ENERGY        7

// Engineering mode:
# define P159_OUTPUT_LIGHT_SENSOR                 8
# define P159_OUTPUT_PIN_STATE                    9
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE1 10
# define P159_OUTPUT_STATIC_DISTANCE_GATE_index   10
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE2 11
# define P159_OUTPUT_MOVING_DISTANCE_GATE_index   11
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE3 12
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE4 13
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE5 14
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE6 15
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE7 16
# define P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE8 17

# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE1 18
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE2 19
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE3 20
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE4 21
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE5 22
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE6 23
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE7 24
# define P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE8 25

# define P159_NR_OUTPUT_OPTIONS                   8    // Last P159_OUTPUT_* value + 1 (count)
# define P159_NR_ENGINEERING_OUTPUT_OPTIONS       26   // Last P159_OUTPUT_* value + 1 (count)

# define P159_DELAY_RESTART                       2500 // milliseconds to 'wait' (ignore) after a device-restart

enum class P159_state_e : uint8_t {
  Initializing     = 0u,
  Restarting       = 1u,
  GetVersion       = 2u,
  GetConfiguration = 3u,
  Engineering      = 4u,
  Running          = 5u,
};

const __FlashStringHelper* Plugin_159_valuename(uint8_t value_nr,
                                                bool    displayString);

struct P159_data_struct : public PluginTaskData_base {
  P159_data_struct(ESPEasySerialPort portType,
                   int8_t            rxPin,
                   int8_t            txPin,
                   bool              engineeringMode);
  virtual ~P159_data_struct() = default;
  void disconnectSerial(); // Disconnect the serial port connected to the sensor
  bool processSensor();    // Process sensor, must be called regularly
  bool plugin_read(struct EventStruct *event);
  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);

  bool isValid() const {
    return nullptr != easySerial && nullptr != radar;
  }

  P159_state_e getCurrentState() const {
    return state;
  }

  bool isRunning() const {
    return isValid() && P159_state_e::Running == state;
  }

private:

  int getRadarValue(struct EventStruct *event,
                    uint8_t             varIndex,
                    int16_t             valueIndex,
                    int                 previousValue,
                    bool              & isChanged);
  void addJavascript();

  ESPeasySerial *easySerial         = nullptr; // Serial port object
  ld2410        *radar              = nullptr; // Sensor object
  uint32_t       milestone          = 0;       // When can we do the next phase when not in Running state?
  P159_state_e   state              = P159_state_e::Initializing;
  bool           _engineeringMode   = false;
  bool           _configurationRead = false;
};

#endif  // USES_P159
#endif  // PLUGINSTRUCTS_P159_DATA_STRUCT_H
