#include "_Plugin_Helper.h"

#ifdef USES_P015

// #######################################################################################################
// ######################## Plugin 015 TSL2561 I2C Lux Sensor ############################################
// #######################################################################################################
// complete rewrite, to support lower lux values better, add ability to change gain and sleep mode
// by: https://github.com/krikk
// this plugin is based on the sparkfun library
// written based on version 1.1.0 from https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library


# include "src/PluginStructs/P015_data_struct.h"

# define PLUGIN_015
# define PLUGIN_ID_015        15
# define PLUGIN_NAME_015       "Light/Lux - TSL2561"
# define PLUGIN_VALUENAME1_015 "Lux"
# define PLUGIN_VALUENAME2_015 "Infrared"
# define PLUGIN_VALUENAME3_015 "Broadband"
# define PLUGIN_VALUENAME4_015 "Ratio"


# define P015_I2C_ADDR    PCONFIG(0)
# define P015_INTEGRATION PCONFIG(1)
# define P015_SLEEP       PCONFIG(2)
# define P015_GAIN        PCONFIG(3)


boolean Plugin_015(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_015;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_015);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_015));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { TSL2561_ADDR_0, TSL2561_ADDR, TSL2561_ADDR_1 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 3, i2cAddressValues, P015_I2C_ADDR, TSL2561_ADDR);
      } else {
        success = intArrayContains(3, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P015_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P015_I2C_ADDR = TSL2561_ADDR; // Default address

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *options[] = {
          F("13.7 ms"),
          F("101 ms"),
          F("402 ms"),
        };
        constexpr size_t optionCount = NR_ELEMENTS(options);
        addFormSelector(F("Integration time"), F("pintegration"), optionCount, options, nullptr, P015_INTEGRATION);
      }

      addFormCheckBox(F("Send sensor to sleep:"), F("psleep"),
                      P015_SLEEP);

      {
        const __FlashStringHelper *options[] = {
          F("No Gain"),
          F("16x Gain"),
          F("Auto Gain"),
          F("Extended Auto Gain"),
        };
        const int optionValues[] = {
          P015_NO_GAIN,
          P015_16X_GAIN,
          P015_AUTO_GAIN,
          P015_EXT_AUTO_GAIN,
        };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("Gain"), F("pgain"), optionCount, options, optionValues, P015_GAIN);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P015_I2C_ADDR    = getFormItemInt(F("i2c_addr"));
      P015_INTEGRATION = getFormItemInt(F("pintegration"));
      P015_SLEEP       = isFormItemChecked(F("psleep"));
      P015_GAIN        = getFormItemInt(F("pgain"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P015_data_struct(P015_I2C_ADDR, P015_GAIN, P015_INTEGRATION));
      break;
    }

    case PLUGIN_READ:
    {
      P015_data_struct *P015_data =
        static_cast<P015_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P015_data) {
        P015_data->begin();

        float luxVal, infraredVal, broadbandVal, ir_broadband_ratio{};

        success = P015_data->performRead(
          luxVal, infraredVal, broadbandVal, ir_broadband_ratio);
        UserVar.setFloat(event->TaskIndex, 0, luxVal);
        UserVar.setFloat(event->TaskIndex, 1, infraredVal);
        UserVar.setFloat(event->TaskIndex, 2, broadbandVal);
        UserVar.setFloat(event->TaskIndex, 3, ir_broadband_ratio);

        if (P015_SLEEP) {
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("TSL2561: sleeping..."));
          # endif // ifndef BUILD_NO_DEBUG
          P015_data->setPowerDown();
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P015
