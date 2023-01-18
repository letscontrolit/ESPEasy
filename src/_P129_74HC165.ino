#include "_Plugin_Helper.h"

#ifdef USES_P129

// #######################################################################################################
// ################################ Plugin 129 74HC165 Shiftregisters      ###############################
// #######################################################################################################

/** Changelog:
 * 2023-01-04 tonhuisman: Use DIRECT_pin GPIO functions for faster GPIO handling (mostly on ESP32), string optimization
 * 2022-08-05 tonhuisman: Fix issue with reading 8th bit of each byte (found during HW testing)
 *                        Reduce number of Values to match the selected number of chips/4. Small UI improvements.
 *                        Enable pin is no longer required, as it is not available or required on some boards.
 * 2022-07-30 tonhuisman: Remove Testing tag from plugin name.
 * 2022-06-12 tonhuisman: Optimizations and small fixes. Implement use of PCONFIG_ULONG()
 * 2022-02-25 tonhuisman: Rename command to ShiftIn,<subcommand>,<arg>...
 * 2022-02-23 tonhuisman: Add command handling.
 * 2022-02-22 tonhuisman: Compare results and generate events.
 * 2022-02-21 tonhuisman: Add output selection dec + hex/bin, dec or hex/bin.
 * 2022-02-20 tonhuisman: Initial plugin development.
 *                        Based on a Forum request: https://www.letscontrolit.com/forum/viewtopic.php?p=57072&hilit=74hc165#p57072
 */

/** Commands:
 * These commands only change configuration settings, but do not save them. They can be saved using the 'save' command.
 *
 * ShiftIn,PinEvent,<pin>,<0|1>    : Set the event enable state for pin (1..128), max. up to configured chips * 8.
 * ShiftIn,ChipEvent,<chip>,<0|1>  : Set the event enable state for an entire chip (1..16), max. up to configured chips.
 * ShiftIn,SetChipCount,<count>    : Set the number of chips, up to 16 (P129_MAX_CHIP_COUNT).
 * ShiftIn,SampleFrequency,<0|1>   : Set the sample frequency, 0 = 10x/sec, 1 = 50x/sec.
 * ShiftIn,EventPerPin,<0|1>       : Set events per pin off or on.
 */

# define PLUGIN_129
# define PLUGIN_ID_129          129
# define PLUGIN_NAME_129        "Input - Shift registers (74HC165)"
# define PLUGIN_VALUENAME1_129  "State_A"
# define PLUGIN_VALUENAME2_129  "State_B"
# define PLUGIN_VALUENAME3_129  "State_C"
# define PLUGIN_VALUENAME4_129  "State_D"

# include "./src/PluginStructs/P129_data_struct.h"

// TODO tonhuisman: ? Move to StringConverter ? though it is a bit specific, can also be used by P126
String P129_ul2stringFixed(uint32_t value, uint8_t base) {
  uint64_t val = static_cast<uint64_t>(value);

  val &= 0x0ffffffff;     // Keep 32 bits
  val |= 0x100000000;     // Set bit just left of 32 bits so we will see the leading zeroes
  String valStr = ull2String(val, base);

  valStr.remove(0, 1);    // Delete leading 1 we added

  if (base == HEX) {
    valStr.toUpperCase(); // uppercase hex for readability
  }
  return valStr;
}

boolean Plugin_129(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_129;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = false;
      Device[deviceCount].ValueCount         =
      # if P129_MAX_CHIP_COUNT <= 4
        1
      # elif P129_MAX_CHIP_COUNT <= 8
        2
      # elif P129_MAX_CHIP_COUNT <= 12
        3
      # else // if P129_MAX_CHIP_COUNT > 12
        4
      # endif // if P129_MAX_CHIP_COUNT <= 4
      ;
      Device[deviceCount].SendDataOption = true; // No use in sending the Values to a controller
      Device[deviceCount].TimerOption    = true; // Used to update the Devices page
      Device[deviceCount].TimerOptional  = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_129);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_129));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_129));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_129));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_129));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P129_CONFIG_CHIP_COUNT                       = 1; // Minimum is 1 chip
      P129_CONFIG_DATA_PIN                         = -1;
      P129_CONFIG_CLOCK_PIN                        = -1;
      P129_CONFIG_ENABLE_PIN                       = -1;
      P129_CONFIG_LOAD_PIN                         = -1;
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0; // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[3] = 0; // No decimals needed
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("Data (Q7)"));
      event->String2 = formatGpioName_output(F("Clock (CP)"));
      event->String3 = formatGpioName_output(F("Enable (<SPAN STYLE=\"text-decoration:overline\">EN</SPAN>) (opt.)"));
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = min(static_cast<uint8_t>(VARS_PER_TASK),
                        static_cast<uint8_t>(ceil(P129_CONFIG_CHIP_COUNT / 4.0f)));
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(
        min(static_cast<uint8_t>(VARS_PER_TASK),
            static_cast<uint8_t>(ceil(P129_CONFIG_CHIP_COUNT / 4.0f))));
      event->idx = 0;
      success    = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output(F("Load (<SPAN STYLE=\"text-decoration:overline\">PL</SPAN>)")),
                       F("load_pin"),
                       P129_CONFIG_LOAD_PIN);
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("GPIO pins for Data, Clock and Load <B>must</B> be configured to correctly initialize the plugin."));
      # endif // ifndef LIMIT_BUILD_SIZE

      addFormSubHeader(F("Device configuration"));

      {
        String chipCount[P129_MAX_CHIP_COUNT];
        int    chipOption[P129_MAX_CHIP_COUNT];

        for (uint8_t i = 0; i < P129_MAX_CHIP_COUNT; i++) {
          chipCount[i]  = String(i + 1);
          chipOption[i] = i + 1;
        }
        addFormSelector(F("Number of chips (Q7 &rarr; DS)"),
                        F("chipcnt"),
                        P129_MAX_CHIP_COUNT,
                        chipCount,
                        chipOption,
                        P129_CONFIG_CHIP_COUNT,
                        true);
        addUnit(concat(F("Daisychained 1.."), P129_MAX_CHIP_COUNT));
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Changing the number of chips will reload the page and update the Event configuration."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      const __FlashStringHelper *frequencyOptions[] = {
        F("10/sec (100 msec)"),
        F("50/sec (20 msec)") };
      const int frequencyValues[] = { P129_FREQUENCY_10, P129_FREQUENCY_50 };
      addFormSelector(F("Sample frequency"), F("frequency"), 2, frequencyOptions, frequencyValues, P129_CONFIG_FLAGS_GET_READ_FREQUENCY);

      addFormSubHeader(F("Display and output"));

      # ifdef P129_SHOW_VALUES
      addFormCheckBox(F("Values display (Off=Hex/On=Bin)"), F("valuesdisplay"), P129_CONFIG_FLAGS_GET_VALUES_DISPLAY == 1);
      # endif // ifdef P129_SHOW_VALUES

      const __FlashStringHelper *outputOptions[] = {
        F("Decimal &amp; hex/bin"),
        F("Decimal only"),
        F("Hex/bin only") };
      const int outputValues[] = { P129_OUTPUT_BOTH, P129_OUTPUT_DEC_ONLY, P129_OUTPUT_HEXBIN };
      addFormSelector(F("Output selection"), F("outputsel"), 3, outputOptions, outputValues, P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION);

      addFormCheckBox(F("Separate events per pin"), F("separate_events"), P129_CONFIG_FLAGS_GET_SEPARATE_EVENTS == 1);

      addFormSubHeader(F("Event configuration"));

      {
        addRowLabel(F("Enable change-event for"));
        html_table(EMPTY_STRING); // Sub-table
        html_table_header(F("Chip #&nbsp;"), 70);
        html_table_header(F("Port:"),        70);
        html_table_header(F("D7"),           30);
        html_table_header(F("D6"),           30);
        html_table_header(F("D5"),           30);
        html_table_header(F("D4"),           30);
        html_table_header(F("D3"),           30);
        html_table_header(F("D2"),           30);
        html_table_header(F("D1"),           30);
        html_table_header(F("D0"),           30);

        uint64_t bits = 0;
        uint8_t  off  = 0;

        for (uint8_t i = 0; i < P129_CONFIG_CHIP_COUNT; i++) {
          if (i % 4 == 0) {
            bits = PCONFIG_ULONG(i / 4) & 0x0ffffffff;
            off  = 0;
            # ifndef P129_DEBUG_LOG

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("74HC165 Reading from: ");
              log += (i / 4);
              log += F(", bits: ");
              log += P129_ul2stringFixed(bits, BIN);
              addLog(LOG_LEVEL_INFO, log);
            }
            # endif // ifndef P129_DEBUG_LOG
          }
          html_TR();
          addHtml(F("<td align =\"center\">"));
          addHtmlInt(i + 1);
          html_TD();

          for (uint8_t j = 0; j < 8; j++) {
            html_TD();
            # if FEATURE_TOOLTIPS
            String toolTip = F("Chip ");
            toolTip += (i + 1);
            toolTip += F(" port D");
            toolTip += (7 - j);
            toolTip += F(", pin ");
            toolTip += i * 8 + (8 - j);
            # endif // if FEATURE_TOOLTIPS
            addCheckBox(getPluginCustomArgName((i * 8 + (7 - j)) + 1), bitRead(bits, off * 8 + (7 - j)) == 1
                        # if FEATURE_TOOLTIPS
                        , false // = not Disabled
                        , toolTip
                        # endif // if FEATURE_TOOLTIPS
                        );
          }
          off++;
        }
        html_end_table();
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P129_CONFIG_LOAD_PIN   = getFormItemInt(F("load_pin"));
      P129_CONFIG_CHIP_COUNT = getFormItemInt(F("chipcnt"));

      uint32_t lSettings = 0u;

      # ifdef P129_SHOW_VALUES

      if (isFormItemChecked(F("valuesdisplay"))) { bitSet(lSettings, P129_FLAGS_VALUES_DISPLAY); }

      if (isFormItemChecked(F("separate_events"))) { bitSet(lSettings, P129_FLAGS_SEPARATE_EVENTS); }
      # endif // ifdef P129_SHOW_VALUES

      if (getFormItemInt(F("frequency"))) { bitSet(lSettings, P129_FLAGS_READ_FREQUENCY); }
      set4BitToUL(lSettings, P129_FLAGS_OUTPUT_SELECTION, getFormItemInt(F("outputsel")));

      P129_CONFIG_FLAGS = lSettings & 0xFFFF;

      uint64_t bits = 0;
      uint8_t  off  = 0;

      for (uint8_t i = 0; i < P129_CONFIG_CHIP_COUNT; i++) {
        if (i % 4 == 0) {
          bits = 0;
          off  = 0;
        }

        for (uint8_t j = 0; j < 8; j++) {
          bitWrite(bits, static_cast<uint64_t>(off * 8 + (7 - j)), isFormItemChecked(getPluginCustomArgName((i * 8 + (7 - j)) + 1))); // -V629
        }
        PCONFIG_ULONG(i / 4) = bits;

        # ifndef P129_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_INFO) && ((i % 4 == 3) || (i == P129_CONFIG_CHIP_COUNT))) {
          String log = F("74HC165 Writing to: ");
          log += (i / 4);
          log += F(", offset: ");
          log += (off * 8);
          log += F(", bits: ");
          log += P129_ul2stringFixed(bits, BIN);
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifndef P129_DEBUG_LOG
        off++;
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P129_data_struct(P129_CONFIG_DATA_PIN,
                                                                               P129_CONFIG_CLOCK_PIN,
                                                                               P129_CONFIG_ENABLE_PIN,
                                                                               P129_CONFIG_LOAD_PIN,
                                                                               P129_CONFIG_CHIP_COUNT));
      P129_data_struct *P129_data = static_cast<P129_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P129_data) && P129_data->isInitialized()) {
        success = P129_data->plugin_init(event);
      }

      if (!success) {
        addLog(LOG_LEVEL_ERROR, F("74HC165: Initialization error!"));
      # ifdef P129_DEBUG_LOG
      } else {
        addLog(LOG_LEVEL_INFO, F("74HC165: Initialized."));
      # endif // ifdef P129_DEBUG_LOG
      }

      break;
    }

    case PLUGIN_READ:
    {
      P129_data_struct *P129_data = static_cast<P129_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P129_data) {
        success = P129_data->plugin_read(event); // Get state
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (((function == PLUGIN_TEN_PER_SECOND) && (P129_CONFIG_FLAGS_GET_READ_FREQUENCY == P129_FREQUENCY_10)) ||
          ((function == PLUGIN_FIFTY_PER_SECOND) && (P129_CONFIG_FLAGS_GET_READ_FREQUENCY == P129_FREQUENCY_50))) {
        P129_data_struct *P129_data = static_cast<P129_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P129_data) {
          success = P129_data->plugin_readData(event);
        }
      }

      break;
    }
    case PLUGIN_FORMAT_USERVAR:
    {
      string.clear();

      if ((P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P129_OUTPUT_BOTH) ||
          (P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P129_OUTPUT_DEC_ONLY)) {
        string += String(UserVar.getUint32(event->TaskIndex, event->idx));
      }

      if (P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P129_OUTPUT_BOTH) {
        string += ',';
      }

      if ((P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P129_OUTPUT_BOTH) ||
          (P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION == P129_OUTPUT_HEXBIN)) {
        string += '0';
        string += (P129_CONFIG_FLAGS_GET_VALUES_DISPLAY ? 'b' : 'x');
        string += P129_ul2stringFixed(UserVar.getUint32(event->TaskIndex, event->idx),
                                      # ifdef P129_SHOW_VALUES
                                      (P129_CONFIG_FLAGS_GET_VALUES_DISPLAY ? BIN :
                                      # endif // ifdef P129_SHOW_VALUES
                                      HEX
                                      # ifdef P129_SHOW_VALUES
                                      )
                                      # endif // ifdef P129_SHOW_VALUES
                                      );
      }
      success = true;
      break;
    }

    # ifdef P129_SHOW_VALUES
    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        String state, label;
        state.reserve(40);
        String abcd             = F("ABCDEFGH");              // In case anyone dares to extend VARS_PER_TASK to 8...
        const uint16_t endCheck = P129_CONFIG_CHIP_COUNT + 4; // 4(.0) = nr of bytes in an uint32_t.
        const uint16_t maxVar   = min(static_cast<uint8_t>(VARS_PER_TASK), static_cast<uint8_t>(ceil(P129_CONFIG_CHIP_COUNT / 4.0f)));
        uint8_t dotInsert;
        uint8_t dotOffset;

        for (uint16_t varNr = 0; varNr < maxVar; varNr++) {
          if (P129_CONFIG_FLAGS_GET_VALUES_DISPLAY) {
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

          label += min(255, P129_CONFIG_SHOW_OFFSET + (4 * varNr) + 4);  // Limited to max 255 chips
          label += '_';
          label += (P129_CONFIG_SHOW_OFFSET + (4 * varNr) + 1);          // 4 = nr of bytes in an uint32_t.

          if ((P129_CONFIG_SHOW_OFFSET + (4 * varNr) + 4) <= endCheck) { // Only show if still in range
            state += P129_ul2stringFixed(UserVar.getUint32(event->TaskIndex, varNr), P129_CONFIG_FLAGS_GET_VALUES_DISPLAY ? BIN : HEX);

            for (uint8_t i = 0; i < 3; i++, dotInsert += dotOffset) {    // Insert readability separators
              state = state.substring(0, dotInsert) + '.' + state.substring(dotInsert);
            }
            pluginWebformShowValue(event->TaskIndex, VARS_PER_TASK + varNr, label, state, true);
          }
        }
        success = true; // Don't show the default value data
        break;
      }
    # endif // ifdef P129_SHOW_VALUES
    case PLUGIN_WRITE:
      {
        P129_data_struct *P129_data = static_cast<P129_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P129_data) {
          success = P129_data->plugin_write(event, string);
        }

        break;
      }
  }
  return success;
}

#endif // ifdef USES_P129
