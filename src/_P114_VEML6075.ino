#include "_Plugin_Helper.h"

#ifdef USES_P114

// #######################################################################################################
// ########################### Plugin 114 VEML6075 I2C UVA/UVB Sensor      ###############################
// #######################################################################################################
// ###################################### stefan@clumsy.ch      ##########################################
// #######################################################################################################


# define PLUGIN_114
# define PLUGIN_ID_114          114
# define PLUGIN_NAME_114        "UV - VEML6075 UVA/UVB Sensor"
# define PLUGIN_VALUENAME1_114  "UVA"
# define PLUGIN_VALUENAME2_114  "UVB"
# define PLUGIN_VALUENAME3_114  "UVIndex"

# include "./src/PluginStructs/P114_data_struct.h"


boolean Plugin_114(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_114;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_114);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_114));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_114));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_114));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[2] = { 0x10, 0x11 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c"), 2, i2cAddressValues, PCONFIG(0));
        addFormNote(F("SDO Low=0x10, High=0x11"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsMode2[] = {
          F("50 ms"),
          F("100 ms"),
          F("200 ms"),
          F("400 ms"),
          F("800 ms"),
        };
        const int optionValuesMode2[] = {
          P114_IT_50,
          P114_IT_100,
          P114_IT_200,
          P114_IT_400,
          P114_IT_800,
        };
        addFormSelector(F("Integration Time"), F("it"), 5, optionsMode2, optionValuesMode2, PCONFIG(1));
      }

      {
        const __FlashStringHelper *optionsMode3[] = {
          F("Normal Dynamic"),
          F("High Dynamic") }
        ;
        addFormSelector(F("Dynamic Setting"), F("hd"), 2, optionsMode3, nullptr, PCONFIG(2));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c"));
      PCONFIG(1) = getFormItemInt(F("it"));
      PCONFIG(2) = getFormItemInt(F("hd"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P114_data_struct(PCONFIG(0), PCONFIG(1), PCONFIG(2) == 1));
      break;
    }

    case PLUGIN_READ:
    {
      P114_data_struct *P114_data =
        static_cast<P114_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P114_data) {
        return success;
      }

      float UVA     = 0.0f;
      float UVB     = 0.0f;
      float UVIndex = 0.0f;

      if (P114_data->read_sensor(UVA, UVB, UVIndex)) {
        UserVar.setFloat(event->TaskIndex, 0, UVA);
        UserVar.setFloat(event->TaskIndex, 1, UVB);
        UserVar.setFloat(event->TaskIndex, 2, UVIndex);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, strformat(F("VEML6075: Address: 0x%02x / Integration Time: %d / "
                                                 "Dynamic Mode: %d / divisor: %d / UVA: %.2f / UVB: %.2f / UVIndex: %.2f"),
                                               PCONFIG(0), PCONFIG(1), PCONFIG(2), 1 << (PCONFIG(1) - 1),
                                               UserVar[event->BaseVarIndex],
                                               UserVar[event->BaseVarIndex + 1],
                                               UserVar[event->BaseVarIndex + 2]));
        }

        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P114
