#include "../PluginStructs/P139_data_struct.h"

#ifdef USES_P139

# ifdef ESP32

// **************************************************************************/
// Constructor
// **************************************************************************/
P139_data_struct::P139_data_struct(struct EventStruct *event) {
  axp2101 = new (std::nothrow) AXP2101(); // Default address and I2C Wire object

  if (isInitialized()) {                  // Functions based on:
    axp2101->begin(&Wire, AXP2101_ADDR, static_cast<AXP2101_device_model_e>(P139_CONFIG_PREDEFINED));
    loadSettings(event);
    outputSettings(event);
  } else {
    addLog(LOG_LEVEL_ERROR, F("AXP2101: Initialization failed"));
  }
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P139_data_struct::~P139_data_struct() {
  delete axp2101;
}

// **************************************************************************/
// loadSettings: Load the (custom)settings from flash
// **************************************************************************/
String P139_data_struct::loadSettings(struct EventStruct *event) {
  String result;

  if (!_settingsLoaded) {
    result          = LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_settings), sizeof(_settings));
    _settingsLoaded = true;
  }
  return result;
}

// **************************************************************************/
// outputSettings: Write the current settings to AXP2101
// **************************************************************************/
void P139_data_struct::outputSettings(struct EventStruct *event) {
  uint8_t count = 0;

  for (int s = 0; s < AXP2101_settings_count; ++s) {
    const AXP2101_registers_e reg = AXP2101_intToRegister(s);
    const AXP_pin_s pinState      = _settings.getState(reg);

    if (!AXP2101_isPinDefault(pinState)) {
      if (AXP_pin_s::On == pinState) {
        // axp2101->setPortVoltage(_settings.getVoltage(reg), reg);
        // axp2101->setPortState(true, reg);  // Turn on after setting the voltage
      } else {
        // axp2101->setPortState(false, reg); // Turn off
      }
      ++count;
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Set %d values to port(s)"), count));
  }
}

// **************************************************************************/
// saveSettings: Save the settings to the custom settings area
// **************************************************************************/
String P139_data_struct::saveSettings(struct EventStruct *event) {
  String result = SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&_settings), sizeof(_settings));

  _settingsLoaded = true; // When freshly saved == loaded :-)
  return result;
}

// **************************************************************************/
// applySettings: Update settings to defaults for selected device
// **************************************************************************/
bool P139_data_struct::applySettings(AXP2101_device_model_e device) {
  const int idx =  static_cast<int>(AXP2101_device_model_e::UserDefined == device ?
                                    AXP2101_device_model_e::MAX :
                                    device);

  if ((idx > static_cast<int>(AXP2101_device_model_e::Unselected)) &&
      (idx <= static_cast<int>(AXP2101_device_model_e::MAX))) {
    _settings = AXP2101_deviceSettingsArray[idx];
    return true;
  }
  return false;
}

// **************************************************************************/
// plugin_read: Read the values and send to controller(s)
// **************************************************************************/
bool P139_data_struct::plugin_read(struct EventStruct *event) {
  bool success = true;

  const uint8_t valueCount = P139_NR_OUTPUT_VALUES;

  for (uint8_t i = 0; i < valueCount; ++i) {
    UserVar.setFloat(event->TaskIndex, i, read_value(static_cast<AXP2101_registers_e>(PCONFIG(P139_CONFIG_BASE + i))));
  }

  return success;
}

// **************************************************************************/
// read_value: Read the requested value
// **************************************************************************/
float P139_data_struct::read_value(AXP2101_registers_e value) {
  if (isInitialized()) {
    if (AXP2101_registers_e::chargeled == value) {
      return static_cast<float>(axp2101->getChargeLed());
    } else
    if (AXP2101_registers_e::batcharge == value) {
      return static_cast<float>(axp2101->getBatCharge());
    }
    return static_cast<float>(axp2101->getPortVoltage(value));
  }
  return 0.0f;
}

// **************************************************************************/
// plugin_ten_per_second: Check state and generate events
// **************************************************************************/
bool P139_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  // TODO
  return false;
}

// **************************************************************************/
// plugin_fifty_per_second: Check state and generate events
// **************************************************************************/
bool P139_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  // TODO
  return false;
}

// **************************************************************************/
// plugin_write: Process commands
// **************************************************************************/
const char P139_subcommands[] PROGMEM = "readchip|voltage|off|on|percentage|range|chargeled";

enum class P139_subcommands_e : int8_t {
  invalid  = -1,
  readchip = 0,
  voltage,
  off,
  on,
  percentage,
  range,
  chargeled,
};

bool P139_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if (isInitialized() && equals(cmd, F("axp"))) { // Command trigger
    cmd = parseString(string, 2);                 // sub command
    const int subcommand_i = GetCommandCode(cmd.c_str(), P139_subcommands);

    if (-1 == subcommand_i) { return success; } // Shortcut

    const P139_subcommands_e subcmd = static_cast<P139_subcommands_e>(subcommand_i);

    const String var3   = parseString(string, 3);
    const bool   empty3 = var3.isEmpty();
    const bool   empty4 = parseString(string, 4).isEmpty();

    switch (subcmd) {
      case P139_subcommands_e::invalid: break;
      case P139_subcommands_e::readchip:

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, F("AXP2101: Port voltages (mV), state and range:"));

          for (int s = 0; s < AXP2101_register_count; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);
            uint16_t value;
            uint8_t  state = 0u;

            if (AXP2101_registers_e::chargeled == reg) {
              value = static_cast<uint16_t>(axp2101->getChargeLed());
            } else
            if (AXP2101_registers_e::batcharge == reg) {
              value = axp2101->getBatCharge();
            } else {
              value = axp2101->getPortVoltage(reg);
              state = axp2101->getPortState(reg);
            }
            addLog(LOG_LEVEL_INFO, strformat(F("Port: %7s: %4dmV, state: %d, range: %d - %dmV"),
                                             toString(reg),
                                             value,
                                             state,
                                             AXP2101_minVoltage(reg),
                                             AXP2101_maxVoltage(reg)));
          }
        } else {
          addLog(LOG_LEVEL_ERROR, F("AXP2101: 'readchip' needs logging level INFO"));
        }

        success = true;
        break;

      case P139_subcommands_e::voltage:

        if (!empty3 && !empty4) {
          for (int s = 0; s < AXP2101_settings_count; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);

            if (equals(var3, toString(reg, false))) {
              const int min_ = AXP2101_minVoltage(reg);

              if (0 == event->Par3 /* < min_ */) {
                // TODO Q: Turn off when A) 0 or B) below minimum voltage?
                // axp2101->setPortState(false, reg);
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Turn off port %s"), toString(reg)));
                }
                success = true;
              } else
              if ((event->Par3 >= min_) && (event->Par3 <= AXP2101_maxVoltage(reg))) {
                // axp2101->setPortVoltage(event->Par3, reg);
                // axp2101->setPortState(true, reg); // Turn on after setting the voltage
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Set port %s to %dmV"), toString(reg), event->Par3));
                }
                success = true;
              }
            }
          }
        }
        break;

      case P139_subcommands_e::off:
      case P139_subcommands_e::on:

        if (!empty3) {
          for (int s = 0; s < AXP2101_settings_count; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);

            if (equals(var3, toString(reg, false))) {
              const bool stateOn       = P139_subcommands_e::on == subcmd;
              const AXP_pin_s pinState = _settings.getState(reg);

              if (AXP2101_isPinProtected(pinState)) {
                if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                  addLog(LOG_LEVEL_ERROR, strformat(F("AXP2101: Port %s is %s"), toString(reg), toString(pinState)));
                }
              } else {
                // axp2101->setPortState(stateOn, reg);
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Turn %s port %s"), (stateOn ? F("On") : F("Off")), toString(reg)));
                }
                success = true;
              }
            }
          }
        }
        break;

      case P139_subcommands_e::percentage:

        if ((event->Par3 >= 0) && (event->Par3 <= P139_CONST_100_PERCENT) && !empty3 && empty4) {
          for (int s = 0; s < AXP2101_settings_count && !success; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);

            if (equals(var3, toString(reg, false)) && !AXP2101_isPinProtected(_settings.getState(reg))) {
              if (event->Par3 > 0) {
                const uint16_t _value = map(event->Par3,
                                            P139_CONST_1_PERCENT, P139_CONST_100_PERCENT,
                                            _ranges[s][0], _ranges[s][1]);

                // axp2101->setPortVoltage(_value, reg);
                // axp2101->setPortState(true, reg);  // Turn on after setting the voltage
              } else {
                // axp2101->setPortState(false, reg); // Turn off
              }
              success = true;
            }
          }
        }
        break;

      case P139_subcommands_e::range:

        if ((event->Par4 >= 0) && (event->Par4 <= P139_CONST_MAX_LDO) &&
            (event->Par3 < event->Par4) && !empty3 && !empty4) {
          for (int s = 0; s < AXP2101_settings_count && !success; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);

            if (equals(var3, toString(reg, false)) &&
                (event->Par3 >= AXP2101_minVoltage(reg)) &&
                (event->Par4 <= AXP2101_maxVoltage(reg))) {
              _ranges[s][0] = event->Par3;
              _ranges[s][1] = event->Par4;
              success       = true;
            }
          }
        } else {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            for (int s = 0; s < AXP2101_settings_count; ++s) {
              const AXP2101_registers_e reg = AXP2101_intToRegister(s);
              addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: %7s Percentage range: %d - %dmV (State: %s)"),
                                               toString(reg),
                                               _ranges[s][0],
                                               _ranges[s][1],
                                               toString(_settings.getState(reg))));
            }
          }
        }
        success = true;
        break;

      case P139_subcommands_e::chargeled:

        if (event->Par2 >= 0 and event->Par2 <= 3) { // Only allowed options
          AXP2101_chargeled_d led = static_cast<AXP2101_chargeled_d>(event->Par2);

          // axp2101->setChargeLed(led);
          _settings.setChargeLed(led); // Store in settings, but don't save yet
          success = true;
        }
        break;
    } // switch
  }

  return success;
}

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P139_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success         = false;
  const String command = parseString(string, 1);

  for (int r = 0; r < AXP2101_register_count && !success; ++r) {
    const AXP2101_registers_e reg = AXP2101_intToRegister(r);

    if (equals(command, toString(reg, false))) { // Voltage (mV) / numeric state
      if (r >= AXP2101_settings_count) {
        if (AXP2101_registers_e::chargeled == reg) {
          string = static_cast<uint8_t>(axp2101->getChargeLed());
        } else
        if (AXP2101_registers_e::batcharge == reg) {
          string = axp2101->getBatCharge();
        }
      } else {
        string = axp2101->getPortVoltage(reg);
      }
      success = true;
    } else
    if (command.equals(concat(toString(reg, false), F(".status")))) { // Status (name)
      if (r >= AXP2101_settings_count) {
        if (AXP2101_registers_e::chargeled == reg) {
          string  = toString(axp2101->getChargeLed());
          success = true;
        }
      } else {
        string  = toString(axp2101->getPortState(reg));
        success = true;
      }
    } else
    if (command.equals(concat(toString(reg, false), F(".state")))) { // State (int)
      if (r >= AXP2101_settings_count) {
        if (AXP2101_registers_e::chargeled == reg) {
          string  = static_cast<uint8_t>(axp2101->getChargeLed());
          success = true;
        }
      } else {
        string  = static_cast<uint8_t>(axp2101->getPortState(reg));
        success = true;
      }
    }
  }

  return success;
}

# endif // ifdef ESP32
#endif  // ifdef USES_P139
