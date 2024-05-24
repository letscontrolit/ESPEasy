#include "../PluginStructs/P166_data_struct.h"

#ifdef USES_P166

/**************************************************************************
* Constructor
**************************************************************************/
P166_data_struct::P166_data_struct(uint8_t                        address,
                                   DFRobot_GP8403::eOutPutRange_t range) :
  _address(address), _range(range)
{}

P166_data_struct::~P166_data_struct() {
  delete gp8403;
}

bool P166_data_struct::init(struct EventStruct *event) {
  //
  gp8403 = new (std::nothrow) DFRobot_GP8403(&Wire, _address);

  if (nullptr != gp8403) {
    LoadCustomTaskSettings(event->TaskIndex, presets, P166_PresetEntries, 0);
    maxPreset = 0;

    for (uint8_t i = 0; i < P166_PresetEntries; ++i) {
      if (!presets[i].isEmpty()) {
        ++maxPreset;
      } else {
        break; // done
      }
    }
    initialized = true;
    gp8403->setDACOutRange(_range); // Set the output voltage range

    const bool restoreValues = !essentiallyZero(UserVar.getFloat(event->TaskIndex, 3)) && (1 == P166_RESTORE_VALUES);

    for (uint8_t i = 0; i < P166_MAX_OUTPUTS; ++i) { // Set the initial output values
      float fValue;

      if (restoreValues) {
        fValue = UserVar.getFloat(event->TaskIndex, i);
      } else {
        fValue = P166_PRESET_OUTPUT(i);
      }
      const int iValue = static_cast<int>(roundf(fValue * P166_FACTOR_mV));

      if ((iValue >= 0) && (iValue <= ((_range == DFRobot_GP8403::eOutPutRange_t::eOutputRange5V) ? P166_RANGE_5V : P166_RANGE_10V))) {
        gp8403->setDACOutVoltage(iValue, i);
        setUserVarAndLog(event, i, true, fValue, restoreValues ? F("restore") : F("init"));
      }
    }
  }
  addLog(LOG_LEVEL_ERROR, concat(F("GP8403: Initialization "), isInitialized() ? F("succeeded") : F("failed")));
  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P166_data_struct::plugin_read(struct EventStruct *event)           {
  return isInitialized();
}

const char subcommands[] PROGMEM = "volt|mvolt|range|preset|init";

enum class subcmd_e : int8_t {
  invalid = -1,
  volt    = 0,
  mvolt,
  range,
  preset,
  init,
};

/*****************************************************
* plugin_write
*****************************************************/
bool P166_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (isInitialized() && equals(command, F("gp8403"))) {
    const String subcommand   = parseString(string, 2);
    const String sValue       = parseString(string, 4);
    const int    subcommand_i = GetCommandCode(subcommand.c_str(), subcommands);

    if (subcommand_i < 0) { return false; } // Fail fast

    const subcmd_e subcmd = static_cast<subcmd_e>(subcommand_i);
    uint32_t   nChannel{};
    const bool hasChannel = validUIntFromString(parseString(string, 3), nChannel);

    switch (subcmd) {
      case subcmd_e::invalid:
        break;
      case subcmd_e::volt:
      case subcmd_e::mvolt:
      case subcmd_e::preset:

        if (hasChannel && (nChannel <= 2)) { // Channel range check
          float fValue{};
          bool  isValid = false;

          if (subcmd_e::preset == subcmd) {
            isValid = validPresetValue(sValue, fValue);
          } else {
            isValid = validFloatFromString(sValue, fValue);
          }

          if (isValid) { // Value valid check
            const bool voltValue = (subcmd_e::volt == subcmd || subcmd_e::preset == subcmd);

            // Calculate mV from requested voltage
            const int iValue = static_cast<int>(roundf(voltValue ? fValue * P166_FACTOR_mV : fValue));

            if ((iValue >= 0) && (iValue <= ((_range == DFRobot_GP8403::eOutPutRange_t::eOutputRange5V) ? P166_RANGE_5V : P166_RANGE_10V))) {
              gp8403->setDACOutVoltage(static_cast<uint16_t>(iValue), static_cast<uint8_t>(nChannel));

              if ((0 == nChannel) || (2 == nChannel)) {
                setUserVarAndLog(event, 0, voltValue, fValue, subcommand);
              }

              if ((1 == nChannel) || (2 == nChannel)) {
                setUserVarAndLog(event, 1, voltValue, fValue, subcommand);
              }

              // Send out data for events and to controllers by scheduling a TaskRun/PLUGIN_READ
              Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);

              success = true;
            }
          }
        }
        break;

      case subcmd_e::init:

        if (hasChannel && (nChannel <= 2)) { // Channel range check
          for (uint8_t i = 0; i < P166_MAX_OUTPUTS; ++i) {
            const int iValue = static_cast<int>(roundf(P166_PRESET_OUTPUT(i) * P166_FACTOR_mV));

            if ((iValue >= 0) && (iValue <= ((_range == DFRobot_GP8403::eOutPutRange_t::eOutputRange5V) ? P166_RANGE_5V : P166_RANGE_10V))) {
              if ((0 == i) && ((0 == nChannel) || (2 == nChannel))) {
                gp8403->setDACOutVoltage(static_cast<uint16_t>(iValue), static_cast<uint8_t>(i));
                setUserVarAndLog(event, 0, true, P166_PRESET_OUTPUT(i), subcommand);
              }

              if ((1 == i) && ((1 == nChannel) || (2 == nChannel))) {
                gp8403->setDACOutVoltage(static_cast<uint16_t>(iValue), static_cast<uint8_t>(i));
                setUserVarAndLog(event, 1, true, P166_PRESET_OUTPUT(i), subcommand);
              }
              success = true;
            }
          }

          if (success) {
            // Send out data for events and to controllers by scheduling a TaskRun/PLUGIN_READ
            Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          }
        }
        break;

      case subcmd_e::range:

        if ((event->Par2 == 5) || (event->Par2 == 10)) { // Value options check
          _range = (event->Par2 == 5
                    ? DFRobot_GP8403::eOutPutRange_t::eOutputRange5V
                    : DFRobot_GP8403::eOutPutRange_t::eOutputRange10V);
          P166_MAX_VOLTAGE = static_cast<int>(_range); // Save manually to store new setting...
          gp8403->setDACOutRange(_range);              // Device output will be updated immediately!

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, strformat(F("%s: 'range', value: 0-%dV"), F("GP8403"), event->Par2));
          }
          success = true;
        }
        break;
    }
  }
  return success;
}

/*****************************************************
* setUserVarAndLog
*****************************************************/
void P166_data_struct::setUserVarAndLog(struct EventStruct *event,
                                        taskVarIndex_t      varNr,
                                        const bool          voltValue,
                                        const float         fValue,
                                        const String      & subcommand) {
  UserVar.setFloat(event->TaskIndex, varNr, voltValue ? fValue : fValue / P166_FACTOR_mV); // V
  UserVar.setFloat(event->TaskIndex, 3,     1.0f);                                         // Mark as set, will be reset on cold boot

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("%s: '%s' Output %d, value: %.3fV"), F("GP8403"),
                                     subcommand.c_str(),
                                     varNr,
                                     voltValue ? fValue : fValue / P166_FACTOR_mV));
  }
}

/*****************************************************
* validPresetValue
*****************************************************/
bool P166_data_struct::validPresetValue(const String& name,
                                        float       & value) {
  bool isValid = false;

  if (!name.isEmpty()) {
    for (uint8_t i = 0; i < maxPreset; ++i) {
      if (parseString(presets[i], 1).equals(name)) {
        isValid = validFloatFromString(parseString(presets[i], 2), value);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("%s: Preset '%s' found, value: %.3fV"), F("GP8403"),
                                           name.c_str(),
                                           value));
        }
        break; // Found it
      }
    }
  }
  return isValid;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P166_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  const String var = parseString(string, 1);

  if (var.startsWith(F("preset")) && (var.length() > 6)) { // [<taskname>#presetX] = the configured preset X value
    uint32_t preIdx = 0;

    if (validUIntFromString(var.substring(6), preIdx) && (preIdx > 0) && (preIdx <= maxPreset)) {
      string  = parseString(presets[preIdx - 1], 2);
      success = true;
    }
  } else if (equals(var, F("initial0"))) { // [<taskname>#initial0] = the configured initial output 0 value
    string  = toString(P166_PRESET_OUTPUT(0), 3);
    success = true;
  } else if (equals(var, F("initial1"))) { // [<taskname>#initial1] = the configured initial output 1 value
    string  = toString(P166_PRESET_OUTPUT(1), 3);
    success = true;
  } else if (equals(var, F("range"))) {    // [<taskname>#range] = the configured range setting 5 or 10
    string  = static_cast<DFRobot_GP8403::eOutPutRange_t>(P166_MAX_VOLTAGE) == DFRobot_GP8403::eOutPutRange_t::eOutputRange5V ? 5 : 10;
    success = true;
  }
  return success;
}

#endif // ifdef USES_P166
