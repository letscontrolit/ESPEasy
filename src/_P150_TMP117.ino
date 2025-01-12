#include "_Plugin_Helper.h"
#ifdef USES_P150

// #######################################################################################################
// ######################## Plugin-150: TMP117 High precision temperature sensor I2C  ####################
// #######################################################################################################

/** Changelog:
 * 2023-04-15 tonhuisman: Correctly apply configuration bits (clear first), add optional low-level logging, improve configuration page
 * 2023-04-13 tonhuisman: Switch to check the sensor once a second, and read when data is available, return last value every interval
 *                        Make one-shot mode work as intended, one-shot is started from last read, so based on the interval
 * 2023-04-09 tonhuisman: Rename configuration options (compile-time), add optional output logging (default on),
 *                        use more I2C_access functions, make Raw value optional (default on),
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
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_150;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

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

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P150_GET_OPT_ENABLE_RAW ? 2 : 1;
      success     = true;

      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = P150_GET_OPT_ENABLE_RAW ? Sensor_VType::SENSOR_TYPE_DUAL : Sensor_VType::SENSOR_TYPE_SINGLE;
      event->idx        = P150_GET_OPT_ENABLE_RAW ? 2 : 1;
      success           = true;

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };

      if (PLUGIN_WEBFORM_SHOW_I2C_PARAMS == function) {
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
      P150_SET_CONF_AVERAGING(P150_AVERAGING_8_SAMPLES);
      P150_SET_CONF_CONVERSION_MODE(P150_CONVERSION_CONTINUOUS);
      P150_SET_CONF_CYCLE_BITS(P150_CYCLE_1_SEC);
      P150_SET_OPT_ENABLE_RAW(1);                       // Enable Raw by default
      P150_SET_OPT_ENABLE_LOG(1);                       // Enable logging by default
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // No decimals for the Raw value

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Temperature offset"), F("offset"), P150_TEMPERATURE_OFFSET);
      addUnit(F("x 0.1C"));
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Offset in units of 0.1 degree Celsius!"));
      # endif // ifndef BUILD_NO_DEBUG

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
        constexpr size_t optionCount = NR_ELEMENTS(averagingOptions);
        addFormSelector(F("Averaging"), F("avg"), optionCount, averagingCaptions, averagingOptions, P150_GET_CONF_AVERAGING);
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
        constexpr size_t optionCount = NR_ELEMENTS(conversionOptions);
        addFormSelector(F("Conversion mode"),
                        F("conv"),
                        optionCount,
                        conversionCaptions,
                        conversionOptions,
                        P150_GET_CONF_CONVERSION_MODE,
                        true);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("Changing this setting will save and reload this page."));
        # endif // ifndef BUILD_NO_DEBUG
      }

      if (P150_GET_CONF_CONVERSION_MODE == P150_CONVERSION_CONTINUOUS) {
        const __FlashStringHelper *cycleCaptions[] = {
          F("15.5 msec"),
          F("125 msec"),
          F("250 msec"),
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
        constexpr size_t optionCount = NR_ELEMENTS(cycleOptions);
        addFormSelector(F("Continuous conversion cycle time"), F("cycle"), optionCount, cycleCaptions, cycleOptions,
                        P150_GET_CONF_CYCLE_BITS);
      }

      addFormSubHeader(F("Output"));

      addFormSelector_YesNo(F("Enable 'Raw' value"), F("raw"), P150_GET_OPT_ENABLE_RAW ? 1 : 0, true);
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Changing this setting will save and reload this page."));
      # endif // ifndef BUILD_NO_DEBUG

      addFormCheckBox(F("Log measured values (INFO)"),  F("log"),  P150_GET_OPT_ENABLE_LOG);

      # if P150_USE_EXTRA_LOG
      addFormCheckBox(F("Log low-level values (INFO)"), F("xlog"), P150_GET_OPT_EXTRA_LOG);
      # endif // if P150_USE_EXTRA_LOG

      success = true;

      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P150_I2C_ADDRESS        = getFormItemInt(F("i2c_addr"));
      P150_TEMPERATURE_OFFSET = getFormItemInt(F("offset"));
      P150_SET_CONF_AVERAGING(getFormItemInt(F("avg")));
      uint8_t prvConv = P150_GET_CONF_CONVERSION_MODE;
      P150_SET_CONF_CONVERSION_MODE(getFormItemInt(F("conv")));

      if ((P150_GET_CONF_CONVERSION_MODE == P150_CONVERSION_CONTINUOUS) && (prvConv == P150_CONVERSION_CONTINUOUS)) {
        P150_SET_CONF_CYCLE_BITS(getFormItemInt(F("cycle")));
      } else {
        P150_SET_CONF_CYCLE_BITS(P150_CYCLE_1_SEC); // Default
      }

      uint8_t raw = getFormItemInt(F("raw"));

      if (P150_GET_OPT_ENABLE_RAW != raw) {
        if (raw) {
          strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_150));
          ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // No decimals for the Raw value
        }
        P150_SET_OPT_ENABLE_RAW(raw);
      }
      P150_SET_OPT_ENABLE_LOG(isFormItemChecked(F("log")));
      # if P150_USE_EXTRA_LOG
      P150_SET_OPT_EXTRA_LOG(isFormItemChecked(F("xlog")));
      # endif // if P150_USE_EXTRA_LOG

      success = true;

      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P150_data_struct(event));
      P150_data_struct *P150_data =
        static_cast<P150_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P150_data && P150_data->init());

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

    case PLUGIN_ONCE_A_SECOND:
    {
      P150_data_struct *P150_data =
        static_cast<P150_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P150_data) {
        success = P150_data->plugin_once_a_second(event);
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P150
