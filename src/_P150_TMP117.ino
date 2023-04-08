#include "_Plugin_Helper.h"
#ifdef USES_P150

// #######################################################################################################
// ######################## Plugin-150: TMP117 High precision temperature sensor I2C  ####################
// #######################################################################################################

/** Changelog:
 * 2023-04-08 tonhuisman: Basic workings for setting configuration and temperature offset, then reading the temperature
 *                        only when data is available.
 * 2023-04-08 tonhuisman: Initial work on new plugin for TMP117, available in Collection F, Climate and MAX builds
 */
# include "src/PluginStructs/P150_data_struct.h"

# define PLUGIN_150
# define PLUGIN_ID_150         150
# define PLUGIN_NAME_150       "Environment - TMP117 Temperature"
# define PLUGIN_VALUENAME1_150 "Temperature"
# define PLUGIN_VALUENAME2_150 "Raw"

boolean Plugin_150(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_150;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE; // Only send first value to controllers
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_150);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_150));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_150));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 4, i2cAddressValues, P150_I2C_ADDRESS);
      } else {
        success = intArrayContains(4, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P150_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P150_SET_FLAG_AVERAGING(P150_AVERAGING_8_SAMPLES);
      P150_SET_FLAG_CONVERSION_MODE(P150_CONVERSION_CONTINUOUS);
      P150_SET_FLAG_CYCLE_BITS(P150_CYCLE_1_SEC);
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // No decimals for the Raw value
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Temperature offset"), F("offset"), P150_TEMPERATURE_OFFSET);
      addUnit(F("x 0.1C"));
      addFormNote(F("Offset in units of 0.1 degree Celsius"));

      {
        const __FlashStringHelper *averagingCaptions[] = {
          F("No averaging"),
          F("8 sample average"),
          F("32 sample average"),
          F("64 sample average"),
        };
        const int averagingOptions[] = {
          P150_AVERAGING_NONE,
          P150_AVERAGING_8_SAMPLES,
          P150_AVERAGING_32_SAMPLES,
          P150_AVERAGING_64_SAMPLES,
        };
        addFormSelector(F("Averaging"), F("avg"), 4, averagingCaptions, averagingOptions, P150_GET_FLAG_AVERAGING);
      }

      {
        const __FlashStringHelper *conversionCaptions[] = {
          F("Continuous"),
          F("One-shot"),
        };
        const int conversionOptions[] = {
          P150_CONVERSION_CONTINUOUS,
          P150_CONVERSION_ONE_SHOT,
        };
        addFormSelector(F("Conversion mode"), F("conv"), 2, conversionCaptions, conversionOptions, P150_GET_FLAG_CONVERSION_MODE);
      }
      {
        const __FlashStringHelper *cycleCaptions[] = {
          F("15.5 msec (continuous)"),
          F("125 msec (continuous)"),
          F("250 msec (continuous)"),
          F("500 msec"),
          F("1 sec"),
          F("4 sec"),
          F("8 sec"),
          F("16 sec"),
        };
        const int cycleOptions[] = {
          P150_CYCLE_15_5_MSEC,
          P150_CYCLE_125_MSEC,
          P150_CYCLE_250_MSEC,
          P150_CYCLE_500_MSEC,
          P150_CYCLE_1_SEC,
          P150_CYCLE_4_SEC,
          P150_CYCLE_8_SEC,
          P150_CYCLE_16_SEC,
        };
        addFormSelector(F("Conversion cycle time"), F("cycle"), 8, cycleCaptions, cycleOptions, P150_GET_FLAG_CYCLE_BITS);
        addFormNote(F("Not all Cycle time options are available for One-shot mode!"));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P150_I2C_ADDRESS        = getFormItemInt(F("i2c_addr"));
      P150_TEMPERATURE_OFFSET = getFormItemInt(F("offset"));
      P150_SET_FLAG_AVERAGING(getFormItemInt(F("avg")));
      P150_SET_FLAG_CONVERSION_MODE(getFormItemInt(F("conv")));
      P150_SET_FLAG_CYCLE_BITS(getFormItemInt(F("cycle")));

      // P150_ERROR_STATE_OUTPUT = getFormItemInt(F("err"));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P150_data_struct(event));
      P150_data_struct *P150_data =
        static_cast<P150_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P150_data && P150_data->init(event));
      break;
    }

    case PLUGIN_READ:
    {
      P150_data_struct *P150_data =
        static_cast<P150_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P150_data) {
        success = P150_data->plugin_read(event);
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P150
