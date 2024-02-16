#include "../PluginStructs/P139_data_struct.h"

#ifdef USES_P139

# ifdef ESP32

// int16_t P139_settingToValue(uint16_t data,
//                             uint16_t threshold) {
//   if (data > threshold) {
//     return threshold - data;
//   }
//   return data;
// }

// uint16_t P139_valueToSetting(int      data,
//                              uint16_t threshold) {
//   if (data <= -1) {
//     return threshold - data;
//   }
//   return static_cast<uint16_t>(data);
// }

/**
 * Apply default parameters based on P139_CONFIG_PREDEFINED setting.
 * This will be 0 by default, so nothing is changed.
 * P139_CONFIG_DISABLEBITS: (set to 1 to disable that setting)
 * Bit = Feature
 * 0   = LDO2
 * 1   = LDO3
 * 2   = LDOIO
 * 3   = GPIO0
 * 4   = GPIO1
 * 5   = GPIO2
 * 6   = GPIO3
 * 7   = GPIO4
 * 8   = DCDC2
 * 9   = DCDC3
 */
void P139_CheckPredefinedParameters(struct EventStruct *event) {
  if (P139_CONFIG_PREDEFINED > 0) {
    P139_CURRENT_PREDEFINED = P139_CONFIG_PREDEFINED;

    //   // Set defaults
    //   for (int i = 0; i < 5; i++) { // GPI0..4
    //     P139_SET_GPIO_FLAGS(i, static_cast<uint8_t>(P139_GPIOBootState_e::Default));
    //   }

    switch (static_cast<AXP2101_device_model_e>(P139_CONFIG_PREDEFINED)) {
      case AXP2101_device_model_e::M5Stack_Core2_v1_1: // M5Stack Core2 V1.1 and newer
      {
        //       P139_REG_DCDC2_LDO2     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC2) << 16) | P139_valueToSetting(3000,
        // P139_CONST_MAX_LDO);
        //       P139_REG_DCDC3_LDO3     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC) << 16) | P139_valueToSetting(3000,
        // P139_CONST_MAX_LDO);
        //       P139_REG_LDOIO          =  P139_valueToSetting(2800, P139_CONST_MAX_LDOIO);
        //       P139_CONFIG_DISABLEBITS = 0b1111110000;     // NC pins disabled
        break;
      }
      case AXP2101_device_model_e::M5Stack_CoreS3: // M5Stack CoreS3
      {
        //       P139_REG_DCDC2_LDO2     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC2) << 16) | P139_valueToSetting(3300,
        // P139_CONST_MAX_LDO);
        //       P139_REG_DCDC3_LDO3     = (P139_valueToSetting(0, P139_CONST_MAX_DCDC) << 16) | P139_valueToSetting(0, P139_CONST_MAX_LDO);
        //       P139_REG_LDOIO          =  P139_valueToSetting(3300, P139_CONST_MAX_LDOIO);
        //       P139_CONFIG_DISABLEBITS = 0b0101000000; // NC pins disabled
        //       // Specials: GPIO1 High = LED off, GPIO4 High = Enable TFT
        //       P139_SET_GPIO_FLAGS(1, static_cast<uint8_t>(P139_GPIOBootState_e::Output_high));
        //       P139_SET_GPIO_FLAGS(4, static_cast<uint8_t>(P139_GPIOBootState_e::Output_high));
        break;
      }
      case AXP2101_device_model_e::LilyGO_TBeam_v1_2: // LilyGO T-Beam V1.2 and newer
      case AXP2101_device_model_e::LilyGO_TBeamS3_v3: // LilyGO T-BeamS3 V3 and newer
      case AXP2101_device_model_e::LilyGO_TPCie_v1_2: // LilyGO T-PCie V1.2 and newer
      {
        //       P139_REG_DCDC2_LDO2     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC2) << 16) | P139_valueToSetting(3300,
        // P139_CONST_MAX_LDO);
        //       P139_REG_DCDC3_LDO3     = (P139_valueToSetting(3300, P139_CONST_MAX_DCDC) << 16) | P139_valueToSetting(3300,
        // P139_CONST_MAX_LDO);
        //       P139_REG_LDOIO          =  P139_valueToSetting(3300, P139_CONST_MAX_LDOIO);
        //       P139_CONFIG_DISABLEBITS = 0b1111111000;   // NC pins disabled
        break;
      }
      case AXP2101_device_model_e::UserDefined: // User defined
      {
        //       P139_REG_DCDC2_LDO2     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC2) << 16) | P139_valueToSetting(3300,
        // P139_CONST_MAX_LDO);
        //       P139_REG_DCDC3_LDO3     = (P139_valueToSetting(-1, P139_CONST_MAX_DCDC) << 16) | P139_valueToSetting(-1,
        // P139_CONST_MAX_LDO);
        //       P139_REG_LDOIO          = P139_valueToSetting(-1, P139_CONST_MAX_LDOIO);
        //       P139_CONFIG_DISABLEBITS = 0b0000000000; // All pins enabled
        break;
      }
      case AXP2101_device_model_e::Unselected:
      case AXP2101_device_model_e::MAX:
        // Fall through
        break;
    }
  }
}

// **************************************************************************/
// toString: convert P139_valueOptions_e enum to value-name or display-name
// **************************************************************************/
const __FlashStringHelper* toString(const P139_valueOptions_e value,
                                    bool                      displayString) {
  switch (value) {
    case P139_valueOptions_e::None: return displayString ? F("None") : F("none");
    case P139_valueOptions_e::BatteryVoltage: return displayString ? F("Battery voltage") : F("batteryvoltage");
    case P139_valueOptions_e::BatteryDischargeCurrent: return displayString ? F("Battery discharge current") : F("batterydischargecurrent");
    case P139_valueOptions_e::BatteryChargeCurrent: return displayString ? F("Battery charge current") : F("batterychargecurrent");
    case P139_valueOptions_e::BatteryPower: return displayString ? F("Battery power") : F("batterypower");
    case P139_valueOptions_e::AcinVoltage: return displayString ? F("Input voltage") : F("inputvoltage");
    case P139_valueOptions_e::AcinCurrent: return displayString ? F("Input current") : F("inputcurrent");
    case P139_valueOptions_e::VbusVoltage: return displayString ? F("VBus voltage") : F("vbusvoltage");
    case P139_valueOptions_e::VbusCurrent: return displayString ? F("VBus current") : F("vbuscurrent");
    case P139_valueOptions_e::InternalTemperature: return displayString ? F("Internal temperature") : F("internaltemperature");
    case P139_valueOptions_e::ApsVoltage: return displayString ? F("APS Voltage") : F("apsvoltage");
    case P139_valueOptions_e::LDO2: return displayString ? F("LDO2 Voltage") : F("ldo2voltage");
    case P139_valueOptions_e::LDO3: return displayString ? F("LDO3 Voltage") : F("ldo3voltage");
    case P139_valueOptions_e::LDOIO: return displayString ? F("LDOIO Voltage") : F("gpiovoltage");
    case P139_valueOptions_e::DCDC2: return displayString ? F("DCDC2 Voltage") : F("dcdc2voltage");
    case P139_valueOptions_e::DCDC3: return displayString ? F("DCDC3 Voltage") : F("dcdc3voltage");
  }
  return F("*Undefined*");
}

// **************************************************************************/
// toString: convert P139_GPIOBootState_e enum to string
// **************************************************************************/
const __FlashStringHelper* toString(const P139_GPIOBootState_e value) {
  switch (value) {
    case P139_GPIOBootState_e::Default: return F("Default");
    case P139_GPIOBootState_e::Output_low: return F("Output, low");
    case P139_GPIOBootState_e::Output_high: return F("Output, high");
    case P139_GPIOBootState_e::Input: return F("Input");
    case P139_GPIOBootState_e::PWM: return F("PWM");
  }
  return F("*Undefined*");
}

// **************************************************************************/
// Constructor
// **************************************************************************/
P139_data_struct::P139_data_struct(struct EventStruct *event) {
  axp2101 = new (std::nothrow) AXP2101(); // Default address and I2C Wire object

  if (isInitialized()) {                  // Functions based on:
    // I2C_AXP192_InitDef initDef = {                         // M5Stack StickC / M5Stack Core2 / LilyGO T-Beam
    //   .EXTEN  = true,                                      // Enable ESP Power
    //   .BACKUP = true,                                      // Enable RTC power
    //   .DCDC1  = 3300,                                      // ESP Power      / ESP Power (Fixed)  / OLed
    //   .DCDC2  = P139_GET_CONFIG_DCDC2,                     // Unused         / Unused             / Unused
    //   .DCDC3  = P139_GET_CONFIG_DCDC3,                     // Unused         / LCD Backlight      / ESP Power
    //   .LDO2   = P139_GET_CONFIG_LDO2,                      // Backlight power (3000 mV) / Periferal VDD   / LoRa
    //   .LDO3   = P139_GET_CONFIG_LDO3,                      // TFT Power (3000 mV)       / Vibration motor / GPS
    //   .LDOIO  = P139_GET_CONFIG_LDOIO,                     // LDOIO voltage (2800 mV)
    //   .GPIO0  = static_cast<int>(P139_GET_FLAG_GPIO0 - 1), // Microphone power / Bus pwr enable / Unused
    //   .GPIO1  = static_cast<int>(P139_GET_FLAG_GPIO1 - 1), // Unused / Sys Led (green)          / Unused
    //   .GPIO2  = static_cast<int>(P139_GET_FLAG_GPIO2 - 1), // Unused / Speaker enable           / Unused
    //   .GPIO3  = static_cast<int>(P139_GET_FLAG_GPIO3 - 1), // Unused / Unused                   / Unused
    //   .GPIO4  = static_cast<int>(P139_GET_FLAG_GPIO4 - 1), // Unused / TFT Reset                / Unused
    // };
    // ldo2_value  = P139_GET_CONFIG_LDO2;
    // ldo3_value  = P139_GET_CONFIG_LDO3;
    // ldoio_value = P139_GET_CONFIG_LDOIO;
    // dcdc2_value = P139_GET_CONFIG_DCDC2;
    // dcdc3_value = P139_GET_CONFIG_DCDC3;

    // axp2101->begin(initDef);
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
const char P139_subcommands[] PROGMEM = "readchip|voltage|off|on|percentage|range";

enum class P139_subcommands_e : int8_t {
  invalid  = -1,
  readchip = 0,
  voltage,
  off,
  on,
  percentage,
  range,
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

          for (int s = 0; s < AXP2101_settings_count; ++s) {
            const AXP2101_registers_e reg = AXP2101_intToRegister(s);
            addLog(LOG_LEVEL_INFO, strformat(F("Port: %7s: %4dmV, state: %d, range: %d - %dmV"),
                                             toString(reg),
                                             axp2101->getPortVoltage(reg),
                                             axp2101->getPortState(reg),
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
            }
          }
        } else {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            for (int s = 0; s < AXP2101_settings_count && !success; ++s) {
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

    if (equals(command, toString(reg, false))) { // Voltage (mV)
      string  = axp2101->getPortVoltage(reg);
      success = true;
    } else
    if (command.equals(concat(toString(reg, false), F(".status")))) { // Status (name)
      string  = toString(axp2101->getPortState(reg));
      success = true;
    } else
    if (command.equals(concat(toString(reg, false), F(".state")))) { // State (int)
      string  = static_cast<uint8_t>(axp2101->getPortState(reg));
      success = true;
    }
  }

  return success;
}

# endif // ifdef ESP32
#endif  // ifdef USES_P139
