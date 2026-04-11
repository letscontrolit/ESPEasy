#include "../PluginStructs/P139_data_struct.h"

#ifdef USES_P139

# ifdef ESP32

#  include "../PluginStructs/P139_data_struct_formselectors.h"

// **************************************************************************/
// Constructors
// **************************************************************************/
P139_data_struct::P139_data_struct(struct EventStruct *event) {
  axp2101 = new (std::nothrow) AXP2101(); // Default address and I2C Wire object

  if (isInitialized()) {                  // Functions based on:
    axp2101->begin(&Wire, AXP2101_ADDR, static_cast<AXP2101_device_model_e>(P139_CURRENT_PREDEFINED));
    loadSettings(event);
    applySettings(event);
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
    result = LoadCustomTaskSettings(event->TaskIndex,
                                    reinterpret_cast<uint8_t *>(&_settings),
                                    sizeof(_settings));
    _settingsLoaded = true;
  }
  return result;
}

// **************************************************************************/
// applySettings: Write the current settings to AXP2101
// **************************************************************************/
void P139_data_struct::applySettings(struct EventStruct *event) {
  if (!isInitialized()) {
    return;
  }
  uint8_t count = 0;

  for (int s = 0; s < AXP2101_settings_count; ++s) {
    const AXP2101_registers_e reg = AXP2101_intToRegister(s);
    const AXP_pin_s pinState      = _settings.getState(reg);

    if (!AXP2101_isPinDefault(pinState)) {
      if (AXP_pin_s::On == pinState) {
        axp2101->setPortVoltage(_settings.getVoltage(reg), reg);
        axp2101->setPortState(true, reg);  // Turn on after setting the voltage
      } else {
        axp2101->setPortState(false, reg); // Turn off
      }
      ++count;

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG,
               strformat(F("AXP2101: Port: %s, output: %dmV, pin state: %s"),
                         FsP(toString(reg)), _settings.getVoltage(reg), FsP(toString(pinState))));
      }
      #  endif // ifndef BUILD_NO_DEBUG
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Set %d values to port(s)"), count));
  }
  axp2101->setChargeLed(_settings.getChargeLed());

  // Reg 61: Iprechg Charger Settings
  axp2101->setPreChargeCurrentLimit(_settings.getPreChargeCurrentLimit());

  // Reg 62: ICC Charger Settings
  axp2101->setConstChargeCurrentLimit(_settings.getConstChargeCurrentLimit());

  // Reg 63: Iterm Charger Settings and Control
  axp2101->setTerminationChargeCurrentLimit(_settings.getTerminationChargeCurrentLimit(), _settings.getTerminationChargeCurrentLimitEnable());

  // Reg 64: CV Charger Voltage Settings
  axp2101->setCV_chargeVoltage(_settings.getCV_chargeVoltage());

  // Reg 14: Minimum System Voltage Control
  axp2101->setLinear_Charger_Vsys_dpm(_settings.getLinear_Charger_Vsys_dpm());

  // Reg 15: Input Voltage Limit
  axp2101->setVin_DPM(_settings.getVin_DPM());

  // Reg 16: Input Current Limit
  axp2101->setInputCurrentLimit(_settings.getInputCurrentLimit());
  axp2101->setTS_disabled(_settings.getTS_disabled());


  //  axp2101->set_IRQ_enable_0(0b11110000); // Disable temperature checks
  axp2101->setConstChargeCurrentLimit(_settings.getConstChargeCurrentLimit());
}

// **************************************************************************/
// saveSettings: Save the settings to the custom settings area
// **************************************************************************/
String P139_data_struct::saveSettings(struct EventStruct *event) {
  const String result = SaveCustomTaskSettings(event->TaskIndex,
                                               reinterpret_cast<const uint8_t *>(&_settings),
                                               sizeof(_settings));

  _settingsLoaded = true; // When freshly saved == loaded :-)
  return result;
}

// **************************************************************************/
// applySettings: Update settings to defaults for selected device
// **************************************************************************/
bool P139_data_struct::applyDeviceModelTemplate(AXP2101_device_model_e device) {
  const int idx = static_cast<int>(AXP2101_device_model_e::UserDefined == device ?
                                   AXP2101_device_model_e::MAX :
                                   device);

  if ((idx > static_cast<int>(AXP2101_device_model_e::Unselected)) &&
      (idx <= static_cast<int>(AXP2101_device_model_e::MAX))) {
    _settings = AXP2101_deviceSettingsArray[idx];
    return true;
  }
  return false;
}

void P139_data_struct::webform_load(struct EventStruct *event) {
  const bool isPowerManagerTask = Settings.isPowerManagerTask(event->TaskIndex);

  {
    AXP2101_ChargeLED_FormSelector selector(_settings.getChargeLed());
  }
  {
    // Reg 61: Iprechg Charger Settings
    // 0 .. 200 mA in 25 mA steps
    AXP2101_PreChargeCurrentLimit_FormSelector selector(_settings.getPreChargeCurrentLimit());
  }
  {
    // Reg 62: ICC Charger Settings
    AXP2101_ConstChargeCurrentLimit_FormSelector selector(_settings.getConstChargeCurrentLimit());
  }
  {
    // Reg 63: Iterm Charger Settings and Control
    // 0 .. 200 mA in 25 mA steps  + enable checkbox
    AXP2101_TerminationChargeCurrentLimit_FormSelector selector(_settings.getTerminationChargeCurrentLimit());
    addFormCheckBox(F("Enable CV Charging"), F("iterm_en"), _settings.getTerminationChargeCurrentLimitEnable());
    addFormNote(F("When enabled, the last part of the charge cycle is done using constant voltage (CV)"));
  }

  {
    // Reg 64: CV Charger Voltage Settings
    AXP2101_CV_charger_voltage_FormSelector selector(_settings.getCV_chargeVoltage());
  }
  {
    // Reg 14: Minimum System Voltage Control
    AXP2101_Linear_Charger_Vsys_dpm_FormSelector selector(_settings.getLinear_Charger_Vsys_dpm());
  }

  {
    // Reg 15: Input Voltage Limit
    AXP2101_Vin_DPM_FormSelector selector(_settings.getVin_DPM());
  }

  {
    // Reg 16: Input Current Limit
    AXP2101_InputCurrentLimit_FormSelector selector(_settings.getInputCurrentLimit());
  }

  addFormCheckBox(F("Disable TS pin"), F("dis_TS"), _settings.getTS_disabled());
  addFormNote(F("Make sure to disable TS pin when no battery temperature sensor is used, or else the battery will not charge"));

  addFormCheckBox(F("Generate events"), F("events"), P139_GET_GENERATE_EVENTS);

  addFormSubHeader(F("Hardware outputs AXP2101"));

  {
    const __FlashStringHelper *predefinedNames[] = {
      toString(AXP2101_device_model_e::Unselected),
      toString(AXP2101_device_model_e::M5Stack_Core2_v1_1),
      toString(AXP2101_device_model_e::M5Stack_CoreS3),
      toString(AXP2101_device_model_e::LilyGO_TBeam_v1_2),
      toString(AXP2101_device_model_e::LilyGO_TBeamS3_v3),
      toString(AXP2101_device_model_e::LilyGO_TPCie_v1_2),
      toString(AXP2101_device_model_e::UserDefined) // keep last !!
    };
    const int predefinedValues[] = {
      static_cast<int>(AXP2101_device_model_e::Unselected),
      static_cast<int>(AXP2101_device_model_e::M5Stack_Core2_v1_1),
      static_cast<int>(AXP2101_device_model_e::M5Stack_CoreS3),
      static_cast<int>(AXP2101_device_model_e::LilyGO_TBeam_v1_2),
      static_cast<int>(AXP2101_device_model_e::LilyGO_TBeamS3_v3),
      static_cast<int>(AXP2101_device_model_e::LilyGO_TPCie_v1_2),
      static_cast<int>(AXP2101_device_model_e::UserDefined) }; // keep last !!
    constexpr uint8_t   valueCount = NR_ELEMENTS(predefinedValues);
    FormSelectorOptions selector(
      valueCount,
      predefinedNames, predefinedValues);
    selector.reloadonchange = !isPowerManagerTask;
    selector.addFormSelector(
      F("Predefined device configuration"),
      F("predef"),
      0);

    const AXP2101_device_model_e device = static_cast<AXP2101_device_model_e>(P139_CURRENT_PREDEFINED);

    if (AXP2101_device_model_e::Unselected != device) {
      addFormNote(concat(F("Last selected: "), toString(device)));

      if (AXP2101_device_model_e::UserDefined == device) {
        addHtml(F("<div class='note'><span style=\"color:red\">Warning: "
                  "Configuring invalid values can damage your device or render it useless!</span></div>"));
      }
    }
  }

  {
    const __FlashStringHelper *bootStates[] = {
      toString(AXP_pin_s::Off),
      toString(AXP_pin_s::On),
      toString(AXP_pin_s::Default),
    };
    const int bootStateValues[] = {
      static_cast<int>(AXP_pin_s::Off),
      static_cast<int>(AXP_pin_s::On),
      static_cast<int>(AXP_pin_s::Default),
    };

    // Don't include Disabled or Protected here, not user-selectable
    constexpr int bootStatesCount = NR_ELEMENTS(bootStateValues);

    const FormSelectorOptions selector(
          bootStatesCount,
          bootStates,
          bootStateValues);


    addRowLabel(F("Output ports"));

    html_table(EMPTY_STRING);
    html_table_header(F("Name"),         100);
    html_table_header(F("Voltage (mV)"), 270);
    html_table_header(F("Pin state"),    150);

    for (int s = 0; s < AXP2101_settings_count; ++s) {
      const AXP2101_registers_e reg = AXP2101_intToRegister(s);
      const AXP_pin_s pin           = _settings.getState(reg);
      html_TR_TD();
      addHtml(toString(reg));
      html_TD();
      addNumericBox(toString(reg, false),
                    _settings.getVoltage(reg, false),
                    -1,
                    AXP2101_maxVoltage(reg),
                    AXP2101_isPinDefault(pin));
      addUnit(strformat(F("range %d - %d"), AXP2101_minVoltage(reg), AXP2101_maxVoltage(reg)));
      html_TD();

      if (AXP2101_isPinProtected(pin)) {
        addUnit(toString(pin));
      } else {
        selector.addSelector(
          concat(F("ps"), toString(reg, false)),
          static_cast<int>(pin));
      }
    }

    html_end_table();

    addFormNote(F("Check your device documentation for what is connected to each output."));

    if (isInitialized()) {
      addFormSubHeader(F("Current State"));

      const AXP2101_registers_e registers[] = {
        AXP2101_registers_e::vbus,
        AXP2101_registers_e::vsys,
        AXP2101_registers_e::vbat,
        AXP2101_registers_e::batcharge,
        AXP2101_registers_e::battemp,
        AXP2101_registers_e::chiptemp
      };

      for (size_t i = 0; i < NR_ELEMENTS(registers); ++i) {
        addRowLabel(toString(registers[i], true));

        if ((registers[i] == AXP2101_registers_e::battemp) ||
            (registers[i] == AXP2101_registers_e::chiptemp))
        {
          addHtmlFloat(read_value(registers[i]));
          addUnit(F("°C"));
        } else {
          addHtmlInt(static_cast<int>(read_value(registers[i])));
          addUnit((registers[i] == AXP2101_registers_e::batcharge)
            ? F("%") : F("mV"));
        }
      }

      addRowLabel(F("Charging State"));

      const AXP2101_chargingState_e chargingState = axp2101->getChargingState();

      if (chargingState != AXP2101_chargingState_e::Charging) {
        addHtml(toString(chargingState));
      }
      else {
        addHtml(toString(axp2101->getChargingDetail()));
      }
    }
  }
}

void P139_data_struct::webform_save(struct EventStruct *event) {
  for (uint8_t i = 0; i < P139_NR_OUTPUT_VALUES; ++i) {
    sensorTypeHelper_saveOutputSelector(event, P139_CONFIG_BASE + i, i,
                                        toString(static_cast<AXP2101_registers_e>(PCONFIG(P139_CONFIG_BASE + i)), false));
  }

  const int predefined = getFormItemInt(F("predef"));
  P139_SET_GENERATE_EVENTS(isFormItemChecked(F("events")));

  if (predefined > 0) { // Apply new template to save it, even when task is disabled
    P139_CURRENT_PREDEFINED = predefined;
    applyDeviceModelTemplate(static_cast<AXP2101_device_model_e>(P139_CURRENT_PREDEFINED));
  } else {
    for (int s = 0; s < AXP2101_settings_count; ++s) {
      const AXP2101_registers_e reg = AXP2101_intToRegister(s);

      if (!AXP2101_isPinProtected(_settings.getState(reg))) {
        _settings.setVoltage(reg, getFormItemInt(toString(reg, false)));
        _settings.setState(reg, static_cast<AXP_pin_s>(getFormItemInt(concat(F("ps"), toString(reg, false)))));
      }
    }
  }

  _settings.setChargeLed(AXP2101_ChargeLED_FormSelector::get());

  // Reg 61: Iprechg Charger Settings
  _settings.setPreChargeCurrentLimit(AXP2101_PreChargeCurrentLimit_FormSelector::get());

  // Reg 62: ICC Charger Settings
  _settings.setConstChargeCurrentLimit(AXP2101_ConstChargeCurrentLimit_FormSelector::get());

  // Reg 63: Iterm Charger Settings and Control
  _settings.setTerminationChargeCurrentLimit(
    AXP2101_TerminationChargeCurrentLimit_FormSelector::get(),
    isFormItemChecked(F("iterm_en")));

  // Reg 64: CV Charger Voltage Settings
  _settings.setCV_chargeVoltage(AXP2101_CV_charger_voltage_FormSelector::get());

  // Reg 14: Minimum System Voltage Control
  _settings.setLinear_Charger_Vsys_dpm(AXP2101_Linear_Charger_Vsys_dpm_FormSelector::get());

  // Reg 15: Input Voltage Limit
  _settings.setVin_DPM(AXP2101_Vin_DPM_FormSelector::get());

  // Reg 16: Input Current Limit
  _settings.setInputCurrentLimit(AXP2101_InputCurrentLimit_FormSelector::get());

  _settings.setTS_disabled(isFormItemChecked(F("dis_TS")));

  saveSettings(event);
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
    switch (value)
    {
      case AXP2101_registers_e::chargeled:
        return static_cast<float>(axp2101->getChargeLed());
      case AXP2101_registers_e::batcharge:
        return static_cast<float>(axp2101->getBatCharge());
      case AXP2101_registers_e::charging:
        return static_cast<float>(axp2101->getChargingState());
      case AXP2101_registers_e::batpresent:
        return static_cast<float>(axp2101->isBatteryDetected());
      case AXP2101_registers_e::chipid:
        return static_cast<float>(axp2101->getChipIDRaw());
      case AXP2101_registers_e::chargedet:
        return static_cast<float>(axp2101->getChargingDetail());

      case AXP2101_registers_e::vbat:
      case AXP2101_registers_e::vbus:
      case AXP2101_registers_e::vsys:
        return static_cast<float>(axp2101->getADCVoltage(value));

      case AXP2101_registers_e::battemp:
        return axp2101->TS_registerToTemp(axp2101->getADCVoltage(value));

      case AXP2101_registers_e::chiptemp:
        return 22.0f + (7274 - axp2101->getADCVoltage(value)) / 20.0f;

      default:
        return static_cast<float>(axp2101->getPortVoltage(value));
    }
  }
  return 0.0f;
}

// **************************************************************************/
// plugin_ten_per_second: Check state and generate events
// **************************************************************************/
// bool P139_data_struct::plugin_ten_per_second(struct EventStruct *event) {
//   // TODO ?
//   return false;
// }

// **************************************************************************/
// plugin_fifty_per_second: Check state and generate events
// **************************************************************************/
bool P139_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (isInitialized() && P139_GET_GENERATE_EVENTS) {
    const AXP2101_chargingState_e charging = axp2101->getChargingState();

    if (_chargingState != charging) {
      eventQueue.add(event->TaskIndex, F("ChargingState"), // Event: <taskname>#ChargingState=<new>,<old> (numeric)
                     strformat(F("%d,%d"), static_cast<int8_t>(charging), static_cast<int8_t>(_chargingState)));
      _chargingState = charging;
    }
    return true;
  }
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

            int32_t value = -1;
            uint8_t state = 0u;
            String  data;

            if (AXP2101_registers_e::chargeled == reg) {
              data = strformat(F(", Led: %s"), FsP(toString(axp2101->getChargeLed())));
            } else
            if (AXP2101_registers_e::batcharge == reg) {
              data = strformat(F(", Battery: %d%%"), axp2101->getBatCharge());
            } else
            if (AXP2101_registers_e::charging == reg) {
              data = strformat(F(", Battery: %s"), FsP(toString(axp2101->getChargingState())));
            } else
            if (AXP2101_registers_e::batpresent == reg) {
              data = strformat(F(", Battery: %s"), FsP(boolToString(axp2101->isBatteryDetected())));
            } else
            if (AXP2101_registers_e::chipid == reg) {
              data = strformat(F(", ChipID: %s (0x%02x)"), FsP(toString(axp2101->getChipID())), axp2101->getChipIDRaw());
            } else
            if (AXP2101_registers_e::chargedet == reg) {
              data = strformat(F(", ChargingDetail: %s"), FsP(toString(axp2101->getChargingDetail())));
            } else {
              value = axp2101->getPortVoltage(reg);
              state = axp2101->getPortState(reg);
            }
            addLog(LOG_LEVEL_INFO, strformat(F("Port: %7s: %4dmV, state: %d, range: %d - %dmV%s"),
                                             FsP(toString(reg)),
                                             value,
                                             state,
                                             AXP2101_minVoltage(reg),
                                             AXP2101_maxVoltage(reg),
                                             data.c_str()));
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
                // Q: Turn off when A) 0 or B) below minimum voltage? Selected answer: A)
                axp2101->setPortState(false, reg);

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Turn off port %s"), FsP(toString(reg))));
                }
                success = true;
              } else
              if ((event->Par3 >= min_) && (event->Par3 <= AXP2101_maxVoltage(reg))) {
                axp2101->setPortVoltage(event->Par3, reg);
                axp2101->setPortState(true, reg); // Turn on after setting the voltage

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Set port %s to %dmV"), FsP(toString(reg)), event->Par3));
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
                  addLog(LOG_LEVEL_ERROR, strformat(F("AXP2101: Port %s is %s"), FsP(toString(reg)), FsP(toString(pinState))));
                }
              } else {
                axp2101->setPortState(stateOn, reg);

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("AXP2101: Switch port %s: %s"), FsP(toString(reg)), FsP(stateOn ? F("On") : F("Off"))));
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

                axp2101->setPortVoltage(_value, reg);
                axp2101->setPortState(true, reg);  // Turn on after setting the voltage
              } else {
                axp2101->setPortState(false, reg); // Turn off
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
                                               FsP(toString(reg)),
                                               _ranges[s][0],
                                               _ranges[s][1],
                                               FsP(toString(_settings.getState(reg)))));
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
  if (!isInitialized()) { return false; }
  bool success         = false;
  const String command = parseString(string, 1);

  for (int r = 0; r < AXP2101_register_count && !success; ++r) {
    const AXP2101_registers_e reg = AXP2101_intToRegister(r);

    if (equals(command, toString(reg, false))) { // Voltage (mV) / numeric state
      if ((reg == AXP2101_registers_e::battemp) ||
          (reg == AXP2101_registers_e::chiptemp))
      {
        string = floatToString(read_value(reg), 2);
      } else {
        string = read_value(reg);
      }
      success = true;
    } else
    if (command.equals(concat(toString(reg, false), F(".status")))) { // Status (name)
      if (r >= AXP2101_settings_count) {
        if (AXP2101_registers_e::chargeled == reg) {
          string  = toString(axp2101->getChargeLed());
          success = true;
        } else
        if (AXP2101_registers_e::charging == reg) {
          string  = toString(axp2101->getChargingState());
          success = true;
        } else
        if (AXP2101_registers_e::chipid == reg) {
          string  = toString(axp2101->getChipID());
          success = true;
        } else
        if (AXP2101_registers_e::chargedet == reg) {
          string  = toString(axp2101->getChargingDetail());
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
