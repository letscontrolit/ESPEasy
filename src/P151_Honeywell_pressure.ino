#include "_Plugin_Helper.h"
#ifdef USES_P151

// #######################################################################################################
// ####################### Plugin 151 Honeywell Digital Output Pressure Sensors ##########################
// #######################################################################################################


# define PLUGIN_151
# define PLUGIN_ID_151         151
# define PLUGIN_NAME_151       "Environment - I2C Honeywell Pressure"
# define PLUGIN_VALUENAME1_151 "Pressure"
# define PLUGIN_VALUENAME2_151 "Temperature"

# include "./src/PluginStructs/P151_data_struct.h"

boolean Plugin_151(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_151;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_151);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_151));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_151));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // 8 bit I2C address notation in datasheet,
      // thus have to shift all options 1 bit to the right.
      // default: 40 (28 hex).
      // Other available standard addresses are:
      // 56 (38 hex)
      // 72 (48 hex)
      // 88 (58 hex)
      // 104 (68 hex)
      // 120 (78 hex)
      // 136 (88 hex)
      // 152 (98 hex)
      const uint8_t i2cAddressValues[] = {
        0x28 >> 1,
        0x38 >> 1,
        0x48 >> 1,
        0x58 >> 1,
        0x68 >> 1,
        0x78 >> 1,
        0x88 >> 1,
        0x98 >> 1
      };
      constexpr size_t addrCount = sizeof(i2cAddressValues) / sizeof(uint8_t);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("pi2c"), addrCount, i2cAddressValues, P151_I2C_ADDR);
      } else {
        success = intArrayContains(addrCount, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P151_I2C_ADDR = 0x28 >> 1;

      P151_OUTPUT_MAX   = 14745; // counts (90% of 214 counts or 0x3999)
      P151_OUTPUT_MIN   = 1638;  // counts (10% of 214 counts or 0x0666)
      P151_PRESSURE_MAX = 1;
      P151_PRESSURE_MIN = 0;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Output Min"), F("out_min"), P151_OUTPUT_MIN, 0, (1 << 14) - 1);
      addFormNumericBox(F("Output Max"), F("out_max"), P151_OUTPUT_MAX, 0, (1 << 14) - 1);
      addFormFloatNumberBox(F("Pressure Min"), F("p_min"),
                            P151_PRESSURE_MIN, P151_MIN_PRESSURE_VALUE, 0.0f, 3);
      addFormFloatNumberBox(F("Pressure Max"), F("p_max"),
                            P151_PRESSURE_MAX, 0.0f, P151_MAX_PRESSURE_VALUE, 3);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P151_I2C_ADDR     = getFormItemInt(F("pi2c"));
      P151_OUTPUT_MIN   = getFormItemInt(F("out_min"));
      P151_OUTPUT_MAX   = getFormItemInt(F("out_max"));
      P151_PRESSURE_MIN = getFormItemFloat(F("p_min"));
      P151_PRESSURE_MAX = getFormItemFloat(F("p_max"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P151_data_struct());
      P151_data_struct *P151_data = static_cast<P151_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P151_data);
      break;
    }

    case PLUGIN_READ:
    {
      P151_data_struct *P151_data = static_cast<P151_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P151_data) {
        success = P151_data->plugin_read(event);
      }

      break;
    }
/*
    case PLUGIN_TEN_PER_SECOND:
    {
      P151_data_struct *P151_data = static_cast<P151_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P151_data) {
        success = P151_data->plugin_ten_per_second(event);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P151_data_struct *P151_data = static_cast<P151_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P151_data) {
        success = P151_data->plugin_fifty_per_second(event);
      }

      break;
    }
*/
  }
  return success;
}

#endif // ifdef USES_P151
