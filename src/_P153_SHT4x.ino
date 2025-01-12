#include "_Plugin_Helper.h"
#ifdef USES_P153

// #######################################################################################################
// ######################### Plugin 153: Environment - SHT4x Temperature, Humidity #######################
// #######################################################################################################

/**
 * 2023-08-26 tonhuisman: BUGFIX: Fixed wrong VType to correctly use SENSOR_TYPE_TEMP_HUM, so it will send data correctly to Domoticz
 * 2023-06-10 tonhuisman: Return NaN values if there is an error connecting to the sensor, or a checksum error is reported
 * 2023-06-10 tonhuisman: BUGFIX: The switch to Normal configuration wasn't working, resulting in checksum errors
 * 2023-04-24 tonhuisman: Rename Boot configuration to Startup configuration, add PLUGIN_WRITE support for commands
 *                        Minor improvements
 * 2023-04-23 tonhuisman: Add Boot Configuration and Normal Configuration options, to allow evaporating condensation
 *                        after start of the plugin
 * 2023-04-22 tonhuisman: Start plugin for SHT4x (SHT40/SHT41/SHT43/SHT45) I2C Temperature and Humidity sensor
 *                        Using direct I2C communication
 **/

/** Commands:
 * sht4x,startup    : Re-start with the Startup Configuration, like a plugin re-start, using the same Interval runs,
 *                    only accepted if Startup and Normal configuration are different, and Interval runs > 0.
 *                    This allows to remove condensation from the sensor from Rules, f.e. once a day.
 */

# define PLUGIN_153
# define PLUGIN_ID_153          153
# define PLUGIN_NAME_153        "Environment - SHT4x"
# define PLUGIN_VALUENAME1_153  "Temperature"
# define PLUGIN_VALUENAME2_153  "Humidity"

# include "./src/PluginStructs/P153_data_struct.h"

bool P153_CheckIntervalError(struct EventStruct *event, int interval) {
  bool result               = false;
  P153_configuration_e conf = static_cast<P153_configuration_e>(P153_STARTUP_CONFIGURATION);

  if (((P153_configuration_e::HighResolution200mW1000msec == conf) ||
       (P153_configuration_e::HighResolution110mW1000msec == conf) ||
       (P153_configuration_e::HighResolution20mW1000msec == conf))
      && (interval <= 10)) {
    result = true;
  }
  return result;
}

boolean Plugin_153(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  const __FlashStringHelper *_intervalError = F("Interval must be increased or Heater duration decreased!"); // used 2x

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_153;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_153);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_153));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_153));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x44, 0x45, 0x46 };

      if (PLUGIN_WEBFORM_SHOW_I2C_PARAMS == function) {
        addFormSelectorI2C(F("i2c_addr"), 3, i2cAddressValues, P153_I2C_ADDRESS);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("Chip type determines address: SHT-4x-<b>A</b>xxx = 0x44, SHT-4x-<b>B</b>xxx = 0x45, SHT-4x-<b>C</b>xxx = 0x46"));
        # endif // ifndef BUILD_NO_DEBUG
      } else {
        success = intArrayContains(3, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P153_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P153_STARTUP_CONFIGURATION = static_cast<int>(P153_configuration_e::HighResolution);
      P153_NORMAL_CONFIGURATION  = static_cast<int>(P153_configuration_e::HighResolution);
      P153_INTERVAL_LOOPS        = 0;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormTextBox(F("Temperature offset"), F("tempoffset"), toString(P153_TEMPERATURE_OFFSET, 2), 5);
      addUnit(F("&deg;C"));

      {
        const __FlashStringHelper *configurations[] = {
          F("Low resolution"),
          F("Medium resolution"),
          F("High resolution"),
          F("High res., Heater 200 mWatt for 1 sec."),
          F("High res., Heater 200 mWatt for 0.1 sec."),
          F("High res., Heater 110 mWatt for 1 sec."),
          F("High res., Heater 110 mWatt for 0.1 sec."),
          F("High res., Heater 20 mWatt for 1 sec."),
          F("High res., Heater 20 mWatt for 0.1 sec."),
        };
        const int configurationOptions[] = {
          static_cast<int>(P153_configuration_e::LowResolution),
          static_cast<int>(P153_configuration_e::MediumResolution),
          static_cast<int>(P153_configuration_e::HighResolution),
          static_cast<int>(P153_configuration_e::HighResolution200mW1000msec),
          static_cast<int>(P153_configuration_e::HighResolution200mW100msec),
          static_cast<int>(P153_configuration_e::HighResolution110mW1000msec),
          static_cast<int>(P153_configuration_e::HighResolution110mW100msec),
          static_cast<int>(P153_configuration_e::HighResolution20mW1000msec),
          static_cast<int>(P153_configuration_e::HighResolution20mW100msec),
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Startup Configuration"),
                        F("startup"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P153_STARTUP_CONFIGURATION);
        addFormNote(F("Heater should not exceed 10% dutycycle, so 1 sec. heater must have Interval > 10 sec.!"));

        addFormNumericBox(F("Use Normal Configuration after"), F("loops"), P153_INTERVAL_LOOPS, 0, 10);
        addUnit(F("Interval runs (0..10)"));

        addFormSelector(F("Normal Configuration"),
                        F("normal"),
                        3, // Only non-heater options
                        configurations,
                        configurationOptions,
                        P153_NORMAL_CONFIGURATION);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P153_I2C_ADDRESS           = getFormItemInt(F("i2c_addr"));
      P153_TEMPERATURE_OFFSET    = getFormItemFloat(F("tempoffset"));
      P153_STARTUP_CONFIGURATION = getFormItemInt(F("startup"));
      P153_INTERVAL_LOOPS        = getFormItemInt(F("loops"));
      P153_NORMAL_CONFIGURATION  = getFormItemInt(F("normal"));

      int interval = getFormItemInt(F("TDT"));

      if (P153_CheckIntervalError(event, interval)) {
        addHtmlError(_intervalError);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      int interval = Settings.TaskDeviceTimer[event->TaskIndex];

      if (!P153_CheckIntervalError(event, interval)) {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P153_data_struct(P153_I2C_ADDRESS,
                                                                                 P153_TEMPERATURE_OFFSET,
                                                                                 static_cast<P153_configuration_e>(P153_STARTUP_CONFIGURATION),
                                                                                 static_cast<P153_configuration_e>(P153_NORMAL_CONFIGURATION),
                                                                                 P153_INTERVAL_LOOPS));
        P153_data_struct *P153_data = static_cast<P153_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P153_data) && P153_data->init();
      } else {
        addLog(LOG_LEVEL_ERROR, concat(F("SHT4x: "), _intervalError));
      }

      break;
    }

    case PLUGIN_READ:
    {
      P153_data_struct *P153_data = static_cast<P153_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P153_data) {
        success = P153_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P153_data_struct *P153_data = static_cast<P153_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P153_data) {
        success = P153_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P153_data_struct *P153_data = static_cast<P153_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P153_data) {
        success = P153_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P153
