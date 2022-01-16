#include "_Plugin_Helper.h"

#ifdef USES_P126

// #######################################################################################################
// ################################ Plugin 126 74HC595 Shiftregisters      ###############################
// #######################################################################################################

/** Changelog:
 * 2022-01-16 tonhuisman: Refactor ShiftRegister74HC595 to ShiftRegister74HC595_NonTemplate to enable runtime sizing
 *                        Add commands, implement PLUGIN_WEBFORM_SHOW_VALUES, testing and improving
 * 2022-01-15 tonhuisman: Implement command handling
 * 2021-11-17 tonhuisman: Initial plugin development. Based on a Forum request: https://www.letscontrolit.com/forum/viewtopic.php?f=5&t=8751
 */

/** Commands:
 * 74hcSet,<pin>,<0|1>
 * 74hcSetNoUpdate,<pin>,<0|1>
 * 74hcUpdate
 * 74hcSetAll,<value>
 * 74hcGetAll
 * 74hcSetAllLow
 * 74hcSetAllHigh
 */

# define PLUGIN_126
# define PLUGIN_ID_126          126
# define PLUGIN_NAME_126        "Output - 74HC595 Shiftregisters [TESTING]"
# define PLUGIN_VALUENAME1_126  "State_1_4"
# define PLUGIN_VALUENAME2_126  "State_5_8"
# define PLUGIN_VALUENAME3_126  "State_9_12"
# define PLUGIN_VALUENAME4_126  "State_13_16"

# include "./src/PluginStructs/P126_data_struct.h"

boolean Plugin_126(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_126;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
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
      String chipCount[P126_MAX_CHIP_COUNT];
      int    chipCountValue[P126_MAX_CHIP_COUNT];

      for (uint8_t chip = 0; chip < P126_MAX_CHIP_COUNT; chip++) {
        chipCount[chip]      =  String(chip + 1);
        chipCountValue[chip] = chip + 1;
      }
      addFormSelector(F("Number of chips (Q7' -> DS)"),
                      F("p126_chips"),
                      P126_MAX_CHIP_COUNT,
                      chipCount,
                      chipCountValue,
                      P126_CONFIG_CHIP_COUNT);
      addUnit(F("Daisychained"));

      addFormCheckBox(F("Values display (Off=Hex/On=Bin)"), F("p126_valuesdisplay"), P126_CONFIG_FLAGS_GET_VALUES_DISPLAY == 1);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P126_CONFIG_CHIP_COUNT = getFormItemInt(F("p126_chips"));
      uint32_t lSettings = 0u;

      bitWrite(lSettings, P126_FLAGS_VALUES_DISPLAY, isFormItemChecked(F("p126_valuesdisplay")));

      P126_CONFIG_FLAGS = lSettings;

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

      if (nullptr == P126_data) {
        return success;
      }

      if (P126_data->isInitialized()) {
        addLog(LOG_LEVEL_INFO, F("74HC595: Initialized."));

        success = true;
      } else {
        addLog(LOG_LEVEL_ERROR, F("74HC595: Initialization error!"));
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

      if (nullptr == P126_data) {
        return success;
      }

      success = P126_data->plugin_read(event); // Get state

      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P126_data_struct *P126_data = static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P126_data) && P126_data->isInitialized()) { // Only show if plugin is active
        String   state, label;
        uint64_t val;
        uint8_t  maxVar = static_cast<uint8_t>(ceil(P126_CONFIG_CHIP_COUNT / 4.0));

        for (uint8_t varNr = 0; varNr < maxVar; varNr++) {
          if (P126_CONFIG_FLAGS_GET_VALUES_DISPLAY) {
            label = F("Bin");
            state = F("0b");
          } else {
            label = F("Hex");
            state = F("0x");
          }
          label += F(" State ");

          switch (varNr) {
            case 0:
              label += F("1_4");
              break;
            case 1:
              label += F("5_8");
              break;
            case 2:
              label += F("9_12");
              break;
            case 3:
              label += F("13_16");
              break;
          }

          val    = static_cast<uint64_t>(UserVar.getUint32(event->TaskIndex, varNr));
          val   &= 0x0ffffffff; // Keep 32 bits
          val   |= 0x100000000; // Set bit just left of 32 bits so we will see the leading zeroes
          state += ull2String(val, (P126_CONFIG_FLAGS_GET_VALUES_DISPLAY ? BIN : HEX));
          state.remove(2, 1);   // Delete leading 1 we added
          pluginWebformShowValue(event->TaskIndex, VARS_PER_TASK + varNr, label, state, true);
        }
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P126_data_struct *P126_data = static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P126_data) {
        return success;
      }

      success = P126_data->plugin_write(event, string);

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P126
