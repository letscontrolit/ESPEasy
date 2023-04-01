#include "_Plugin_Helper.h"

#ifdef USES_P126

// #######################################################################################################
// ################################ Plugin 126 Shift registers 74HC595     ###############################
// #######################################################################################################

/** Changelog:
 * 2022-02-27 tonhuisman: Rename plugin title to Output - Shift registers (74HC595)
 * 2022-02-25 tonhuisman: Again rename commands, now using separate prefix shiftout and the rest of the previous command as subcommand.
 * 2022-02-24 tonhuisman: Further update changing 74hc commands to 74hc595.
 *                        Allow selecting value output decimal + hex/bin, decimal only or hex/bin only.
 *                        Adjust Values display label order to show State_4_1 instead of State_1_4, same order as byte values.
 * 2022-02-23 tonhuisman: Rename commands using prefix 74hc595 to distinguish from plugin P129 74hc165 using similar commands.
 * 2022-01-22 tonhuisman: ShiftRegister74HC595_NonTemplate library: Add setSize method, cleanup constructor
 *                        Setting: Restore register-buffer state from RTC values after warm boot (or crash...)
 *                        NB:!!! Only restores up to 4 * VARS_PER_TASK (16) chip values, starting at the configured Offset for display !!!
 *                        When enabled, changing the offset will reset the values content to 0.
 *                        Code improvements and optimizations
 *                        Add command 74hc595SetChipCount for changing the number of chips at runtime. Does not restart the plugin.
 *                        Hide regular Values display if plugin is active, only custom Hex/Bin states. Show periods in Hex state (too).
 *                        Hide Formula and Decimals for Values. Correct Sensor_VType setting.
 *                        Output both Decimal and Hex or Bin (depending on setting) in generated event. Don't use Single event option, as
 *                        that won't allow handling all 8 values (yet).
 * 2022-01-20 tonhuisman: Fix some bugs, optimize code, now actually supports 255 chips = 2048 pins
 *                        Hex Values display now in uppercase for readability
 * 2022-01-19 tonhuisman: Add 74hc595SetOffset and 74hxSetHexBin commands
 * 2022-01-18 tonhuisman: Improve parsing for 74hc595setall with chipnumber (1..chipCount) and data width (1..4) options
 * 2022-01-17 tonhuisman: Extend to max. 255 chips, add offset for display values, add 74hc595SetAllNoUpdate command
 *                        Rename Value names
 * 2022-01-16 tonhuisman: Refactor ShiftRegister74HC595 to ShiftRegister74HC595_NonTemplate to enable runtime sizing
 *                        Add commands, implement PLUGIN_WEBFORM_SHOW_VALUES, testing and improving
 * 2022-01-15 tonhuisman: Implement command handling
 * 2021-11-17 tonhuisman: Initial plugin development. Based on a Forum request: https://www.letscontrolit.com/forum/viewtopic.php?f=5&t=8751
 */

/** Commands:
 * ShiftOut,Set,<pin>,<0|1>                       : Set a single pin on or off, and update.
 * ShiftOut,SetNoUpdate,<pin>,<0|1>               : Set a single pin on or off. Use ShiftOut,Update to set outputs.
 * ShiftOut,Update                                : Update all pin states to the registers.
 * ShiftOut,SetAll,[chip:][width:]<value>...      : Set a range of chips with values, default 32 bit values (width 4).
 * ShiftOut,SetAllNoUpdate,[chip:][width:]<value> : Ditto, without immediate update. Use ShiftOut,Update to set outputs.
 * ShiftOut,SetAllLow                             : Set all register outputs to 0/low.
 * ShiftOut,SetAllHigh                            : Set all register outputs to 1/high.
 * ShiftOut,SetChipCount,<chip count>             : Set the number of chips, without restarting the plugin. Range 1..P126_MAX_CHIP_COUNT.
 * ShiftOut,SetOffset,<chip offset>               : Set the chip offset for display. Will reflect in device configuration, but not saved.
 * ShiftOut,SetHexBin,<0|1>                       : Turn off/on the Hex or Bin Values display, reflected in device configuration, not saved.
 */

# define PLUGIN_126
# define PLUGIN_ID_126          126
# define PLUGIN_NAME_126        "Output - Shift registers (74HC595)"
# define PLUGIN_VALUENAME1_126  "State_A"
# define PLUGIN_VALUENAME2_126  "State_B"
# define PLUGIN_VALUENAME3_126  "State_C"
# define PLUGIN_VALUENAME4_126  "State_D"

# include "./src/PluginStructs/P126_data_struct.h"

// TODO tonhuisman: ? Move to StringConverter ? though it is a bit specific, can also be used by P129
String P126_ul2stringFixed(uint32_t value, uint8_t base) {
  uint64_t val = static_cast<uint64_t>(value);

  val &= 0x0ffffffff;   // Keep 32 bits
  val |= 0x100000000;   // Set bit just left of 32 bits so we will see the leading zeroes
  String valStr = ull2String(val, base);

  valStr.remove(0, 1);  // Delete leading 1 we added
  valStr.toUpperCase(); // uppercase hex for readability
  return valStr;
}

boolean Plugin_126(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_126;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = false;
      Device[deviceCount].ValueCount         =
      # if P126_MAX_CHIP_COUNT <= 4
        1
      # elif P126_MAX_CHIP_COUNT <= 8
        2
      # elif P126_MAX_CHIP_COUNT <= 12
        3
      # else // if P126_MAX_CHIP_COUNT <= 4
        4
      # endif // if P126_MAX_CHIP_COUNT <= 4
      ;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_126);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_126));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_126));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_126));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_126));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P126_CONFIG_DATA_PIN                         = -1;
      P126_CONFIG_CLOCK_PIN                        = -1;
      P126_CONFIG_LATCH_PIN                        = -1;
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[3] = 0; // No decimals needed
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Data pin (DS)"));
      event->String2 = formatGpioName_output(F("Clock pin (SH_CP)"));
      event->String3 = formatGpioName_output(F("Latch pin (ST_CP)"));
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Device configuration"));

      addFormNumericBox(F("Number of chips (Q7' &rarr; DS)"),
                        F("chips"),
                        P126_CONFIG_CHIP_COUNT,
                        1,                    // Minimum is 1 chip
                        P126_MAX_CHIP_COUNT); // Max chip count
      String unit = F("Daisychained 1..");
      unit += P126_MAX_CHIP_COUNT;
      addUnit(unit);

      addFormNumericBox(F("Offset for display"),
                        F("offset"),
                        P126_CONFIG_SHOW_OFFSET,
                        0,
                        P126_MAX_SHOW_OFFSET);
      addUnit(F("Multiple of 4"));

      # ifdef P126_SHOW_VALUES
      addFormCheckBox(F("Values display (Off=Hex/On=Bin)"), F("valdisplay"), P126_CONFIG_FLAGS_GET_VALUES_DISPLAY == 1);
      # endif // ifdef P126_SHOW_VALUES

      const __FlashStringHelper *outputOptions[] = {
        F("Decimal &amp; hex/bin"),
        F("Decimal only"),
        F("Hex/bin only") };
      const int outputValues[] = { P126_OUTPUT_BOTH, P126_OUTPUT_DEC_ONLY, P126_OUTPUT_HEXBIN };
      addFormSelector(F("Output selection"), F("output"), 3, outputOptions, outputValues, P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION);

      addFormCheckBox(F("Restore Values on warm boot"), F("valrestore"), P126_CONFIG_FLAGS_GET_VALUES_RESTORE);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t previousOffset = P126_CONFIG_SHOW_OFFSET;
      P126_CONFIG_CHIP_COUNT  = getFormItemInt(F("chips"));
      P126_CONFIG_SHOW_OFFSET = getFormItemInt(F("offset"));

      if (P126_CONFIG_SHOW_OFFSET >= P126_CONFIG_CHIP_COUNT) {
        P126_CONFIG_SHOW_OFFSET = 0;
      }
      P126_CONFIG_SHOW_OFFSET -= (P126_CONFIG_SHOW_OFFSET % 4);

      if ((P126_CONFIG_CHIP_COUNT > 4) &&
          (P126_CONFIG_SHOW_OFFSET > P126_CONFIG_CHIP_COUNT - 4) &&
          (P126_CONFIG_CHIP_COUNT < P126_MAX_SHOW_OFFSET)) {
        P126_CONFIG_SHOW_OFFSET -= 4;
      }

      uint32_t lSettings = 0u;

      # ifdef P126_SHOW_VALUES

      if (isFormItemChecked(F("valdisplay"))) { bitSet(lSettings, P126_FLAGS_VALUES_DISPLAY); }
      # endif // ifdef P126_SHOW_VALUES

      if (!isFormItemChecked(F("valrestore"))) { bitSet(lSettings, P126_FLAGS_VALUES_RESTORE); } // Inverted setting!
      set4BitToUL(lSettings, P126_FLAGS_OUTPUT_SELECTION, getFormItemInt(F("output")));

      P126_CONFIG_FLAGS = lSettings;

      // Reset State_A..D values when changing the offset
      if ((previousOffset != P126_CONFIG_SHOW_OFFSET) && P126_CONFIG_FLAGS_GET_VALUES_RESTORE) {
        for (uint8_t varNr = 0; varNr < VARS_PER_TASK; varNr++) {
          UserVar.setUint32(event->TaskIndex, varNr, 0u);
        }
        # ifdef P126_DEBUG_LOG
        addLog(LOG_LEVEL_INFO, F("74HC595: 'Offset for display' changed: state values reset."));
        # endif // ifdef P126_DEBUG_LOG
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P126_data_struct(P126_CONFIG_DATA_PIN,
                                                                               P126_CONFIG_CLOCK_PIN,
                                                                               P126_CONFIG_LATCH_PIN,
                                                                               P126_CONFIG_CHIP_COUNT));
      P126_data_struct *P126_data = static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P126_data) && P126_data->isInitialized()) {
        success = P126_data->plugin_init(event); // Optionally restore State_A..State_D values from RTC (on warm-boot only!)
      }

      if (!success) {
        addLog(LOG_LEVEL_ERROR, F("74HC595: Initialization error!"));
      # ifdef P126_DEBUG_LOG
      } else {
        addLog(LOG_LEVEL_INFO, F("74HC595: Initialized."));
      # endif // ifdef P126_DEBUG_LOG
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P126_data_struct *P126_data = static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P126_data) {
        success = P126_data->plugin_read(event); // Get state
      }


      break;
    }

    case PLUGIN_FORMAT_USERVAR:
    {
      string.clear();

      if ((P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P126_OUTPUT_BOTH) ||
          (P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P126_OUTPUT_DEC_ONLY)) {
        string += ull2String(UserVar.getUint32(event->TaskIndex, event->idx));
      }

      if (P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P126_OUTPUT_BOTH) {
        string += ',';
      }

      if ((P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P126_OUTPUT_BOTH) ||
          (P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P126_OUTPUT_HEXBIN)) {
        string += '0';
        string += (P126_CONFIG_FLAGS_GET_VALUES_DISPLAY ? 'b' : 'x');
        string += P126_ul2stringFixed(UserVar.getUint32(event->TaskIndex, event->idx),
                                      # ifdef P126_SHOW_VALUES
                                      (P126_CONFIG_FLAGS_GET_VALUES_DISPLAY ? BIN :
                                      # endif // ifdef P126_SHOW_VALUES
                                      HEX
                                      # ifdef P126_SHOW_VALUES
                                      )
                                      # endif // ifdef P126_SHOW_VALUES
                                      );
      }
      success = true;
      break;
    }

    # ifdef P126_SHOW_VALUES
    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        String state, label;
        state.reserve(40);
        String abcd = F("ABCDEFGH");                                                                // In case anyone dares to extend
                                                                                                    // VARS_PER_TASK to 8...
        const uint16_t endCheck = P126_CONFIG_CHIP_COUNT + (P126_CONFIG_CHIP_COUNT == 255 ? 3 : 4); // 4(.0) = nr of bytes in an uint32_t.
        const uint16_t maxVar   = min(static_cast<uint8_t>(VARS_PER_TASK), static_cast<uint8_t>(ceil(P126_CONFIG_CHIP_COUNT / 4.0)));
        uint8_t dotInsert;
        uint8_t dotOffset;

        for (uint16_t varNr = 0; varNr < maxVar; varNr++) {
          if (P126_CONFIG_FLAGS_GET_VALUES_DISPLAY) {
            label     = F("Bin");
            state     = F("0b");
            dotInsert = 10;
            dotOffset = 9;
          } else {
            label     = F("Hex");
            state     = F("0x");
            dotInsert = 4;
            dotOffset = 3;
          }
          label += F(" State_");
          label += abcd.substring(varNr, varNr + 1);
          label += ' ';

          label += min(255, P126_CONFIG_SHOW_OFFSET + (4 * varNr) + 4);  // Limited to max 255 chips
          label += '_';
          label += (P126_CONFIG_SHOW_OFFSET + (4 * varNr) + 1);          // 4 = nr of bytes in an uint32_t.

          if ((P126_CONFIG_SHOW_OFFSET + (4 * varNr) + 4) <= endCheck) { // Only show if still in range
            state += P126_ul2stringFixed(UserVar.getUint32(event->TaskIndex, varNr), P126_CONFIG_FLAGS_GET_VALUES_DISPLAY ? BIN : HEX);

            for (uint8_t i = 0; i < 3; i++, dotInsert += dotOffset) {    // Insert readability separators
              state = state.substring(0, dotInsert) + '.' + state.substring(dotInsert);
            }
            pluginWebformShowValue(event->TaskIndex, VARS_PER_TASK + varNr, label, state, true);
          }
        }
        success = true; // Don't show the default value data
        break;
      }
    # endif // ifdef P126_SHOW_VALUES
    case PLUGIN_WRITE:
      {
        P126_data_struct *P126_data = static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P126_data) {
          success = P126_data->plugin_write(event, string);
        }

        break;
      }
  }
  return success;
}

#endif // ifdef USES_P126
