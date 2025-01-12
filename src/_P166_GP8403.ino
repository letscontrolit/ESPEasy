#include "_Plugin_Helper.h"
#ifdef USES_P166

// #######################################################################################################
// ######################### Plugin 166: Output - GP8403 Dual channel DAC 0-10V ##########################
// #######################################################################################################

/** Changelog:
 * 2024-01-29 tonhuisman: Fix bug that changed Initial output values are not applied until a reset/power cycle.
 *                        Disable development-log at Settings Save
 * 2024-01-28 tonhuisman: Add option to restore output values on warm boot (default enabled, using unused 4th value for state)
 *                        Add command to apply initial value(s) per channel
 *                        Some code refactoring
 * 2024-01-26 tonhuisman: Make 0x5F the default I2C address, as that's how the hardware is configured by default
 * 2024-01-26 tonhuisman: Generate PLUGIN_READ when changing output
 * 2024-01-25 tonhuisman: Add I2C enabled check on PLUGIN_INIT
 * 2024-01-24 tonhuisman: Add PLUGIN_GET_CONFIG_VALUE support
 * 2024-01-23 tonhuisman: Add initial value per channel, add some logging, refactoring
 * 2024-01-22 tonhuisman: Add named presets (not case-sensitive) and command handling
 * 2024-01-21 tonhuisman: Start plugin for GP8403 DAC 0-10V (12 bit, 2 channels) based on DFRobot_GP8403 library modified for ESPEasy
 * (Newest changes on top)
 **/

/** Commands:
 * <ch> = output (channel) 0, 1 or 2 (both)
 * gp8403,volt,<ch>,<value>     : Set the voltage in V (0..5.0/0..10.0) value to channel
 * gp8403,mvolt,<ch>,<value>    : Set the voltage in mV (0..5000/0..10000) value to channel
 * gp8403,range,<5|10>          : Set the range to 5V or 10V (both channels)
 * gp8403,preset,<ch>,<name>    : Set the voltage from preset <name> to channel
 * gp8403,init,<ch>             : Set the initial voltage to channel
 */

/** Get Config values:
 * [<taskname>#preset<X>] : The configured preset <X> value (range checked)
 * [<taskname>#initial0]  : The configured initial output 0 value
 * [<taskname>#initial1]  : The configured initial output 1 value
 * [<taskname>#range]     : The configured range setting 5 or 10
 */

# define PLUGIN_166
# define PLUGIN_ID_166          166
# define PLUGIN_NAME_166        "Output - GP8403 Dual-channel DAC 0-10V"
# define PLUGIN_VALUENAME1_166  "Output0"
# define PLUGIN_VALUENAME2_166  "Output1"

# include "./src/PluginStructs/P166_data_struct.h"

boolean Plugin_166(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_166;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true; // FIXME: Is this useful?

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_166);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_166));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_166));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F };

      if (PLUGIN_WEBFORM_SHOW_I2C_PARAMS == function) {
        addFormSelectorI2C(F("i2c_addr"), 8, i2cAddressValues, P166_I2C_ADDRESS, 0x5F); // Mark 0x5F as default
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P166_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P166_I2C_ADDRESS    = 0x5F; // Hardware comes configured at this address
      P166_MAX_VOLTAGE    = static_cast<int>(DFRobot_GP8403::eOutPutRange_t::eOutputRange10V);
      P166_RESTORE_VALUES = 1;    // Enabled by default

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *configurations[] = {
          F("0-5V"),
          F("0-10V"),
        };
        const int configurationOptions[] = {
          static_cast<int>(DFRobot_GP8403::eOutPutRange_t::eOutputRange5V),
          static_cast<int>(DFRobot_GP8403::eOutPutRange_t::eOutputRange10V),
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Output range"),
                        F("range"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P166_MAX_VOLTAGE);
      }

      addFormCheckBox(F("Restore output on warm boot"), F("prstr"), P166_RESTORE_VALUES == 1);

      addFormFloatNumberBox(F("Initial value output 0"), F("prch0"), P166_PRESET_OUTPUT(0), 0.0f, 10.0f, 3);
      addFormFloatNumberBox(F("Initial value output 1"), F("prch1"), P166_PRESET_OUTPUT(1), 0.0f, 10.0f, 3);

      addFormSubHeader(F("Preset values"));

      String presets[P166_PresetEntries]{};

      LoadCustomTaskSettings(event->TaskIndex, presets, P166_PresetEntries, 0);

      addRowLabel(F("Preset value"));

      html_table(EMPTY_STRING);
      html_table_header(F("#"),           50);
      html_table_header(F("Name"),        200);
      html_table_header(F("Voltage (V)"), 120);
      int i = 0;
      int j = 0;

      while ((!presets[i].isEmpty() || j < 5) && i < P166_PresetEntries) {
        html_TR();
        html_TD(F("text-align:center"));
        addHtmlInt(i + 1);
        html_TD();
        addTextBox(getPluginCustomArgName((i * 10) + 0), parseStringKeepCase(presets[i], 1), 16);
        html_TD();
        float value{};
        validFloatFromString(parseStringKeepCase(presets[i], 2), value);
        addFloatNumberBox(getPluginCustomArgName((i * 10) + 1), value, 0.0f, 10.0f, 3);

        if (presets[i].isEmpty()) {
          ++j;
        }
        ++i;
      }
      html_end_table();
      addFormNote(strformat(F("Max. presets: %d. Submit page to add more entries."), P166_PresetEntries));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P166_I2C_ADDRESS      = getFormItemInt(F("i2c_addr"));
      P166_MAX_VOLTAGE      = getFormItemInt(F("range"));
      P166_RESTORE_VALUES   = isFormItemChecked(F("prstr")) ? 1 : 0;
      P166_PRESET_OUTPUT(0) = getFormItemFloat(F("prch0"));
      P166_PRESET_OUTPUT(1) = getFormItemFloat(F("prch1"));

      UserVar.setFloat(event->TaskIndex, 3, 0.0f); // Reset state flag so Initial values will be applied

      String presets[P166_PresetEntries]{};

      int i = 0;
      int j = 0;

      while (i < P166_PresetEntries) {
        String entry = webArg(getPluginCustomArgName((i * 10) + 0));
        entry.trim();

        if (!entry.isEmpty()) {
          const float value = getFormItemFloat(getPluginCustomArgName((i * 10) + 1));
          presets[j] = strformat(F("%s,%.6g"), wrapWithQuotesIfContainsParameterSeparatorChar(entry).c_str(), value);

          // addLog(LOG_LEVEL_INFO, strformat(F("Saving %d: [%s]"), j + 1, presets[j].c_str()));
          ++j;
        }
        ++i;
      }
      const String error = SaveCustomTaskSettings(event->TaskIndex, presets, P166_PresetEntries, 0);

      if (!error.isEmpty()) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.isI2CEnabled()) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P166_data_struct(P166_I2C_ADDRESS,
                                                               static_cast<DFRobot_GP8403::eOutPutRange_t>(P166_MAX_VOLTAGE)));
        P166_data_struct *P166_data = static_cast<P166_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P166_data) && P166_data->init(event);
      } else {
        addLog(LOG_LEVEL_ERROR, F("GP8403: I2C not enabled, init cancelled."));
      }

      break;
    }

    case PLUGIN_READ:
    {
      P166_data_struct *P166_data = static_cast<P166_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P166_data) {
        success = P166_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P166_data_struct *P166_data = static_cast<P166_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P166_data) {
        success = P166_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P166_data_struct *P166_data = static_cast<P166_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P166_data) {
        success = P166_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P166
